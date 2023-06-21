
#include <Octree/SparseVoxelOctree.hpp>

#include <iostream>
#include <World/WorldPositionHelper.hpp>
#include <Octree/SparseVoxelOctreeHelper.hpp>
#include <Core/Global.hpp>

//DEBUG
#include <glm/common.hpp>
namespace mav {

    SparseVoxelOctree::SparseVoxelOctree(size_t depth)
        : depth_(depth), size_(std::pow(8, depth)), len_(std::pow(2, depth)), data_(8, 0), gpuBuffer_(Global::vulkanWrapper->generateSSBO()) {

        #ifndef NDEBUG
            assert(depth_ > 0);
        #endif

        depthToLen_.reserve(depth);
        for (size_t i = 0; i < depth; ++i) {
            depthToLen_.emplace_back( (float)std::pow(2, (depth - 1) - i) );
        }

    }

    uint8_t SparseVoxelOctree::depthStep(glm::uvec3& position, size_t& currentLen) const {

        currentLen /= 2;

        glm::uvec3 mask = glm::uvec3(glm::greaterThanEqual(position, glm::uvec3(currentLen)));
        position -= mask * (unsigned int)currentLen;
        return glm::dot(glm::vec3(mask), glm::vec3(1, 2, 4));;

    }

    void SparseVoxelOctree::set(glm::uvec3 position, int32_t value) {

        //Store the data chunk position and the parent cell offset index
        std::vector<std::pair<size_t, uint8_t>> parentStack;

        size_t currentIndex = 0; //Store the current data chunk
        uint8_t currentDepth = 0;
        size_t currentLen = len_;

        while ( true ) {

            ++currentDepth;

            //Early break
            if (currentDepth == depth_) break;

            uint8_t indexOffset = depthStep(position, currentLen);

            //indicate index of the next depth
            int32_t currentValue = data_[currentIndex + indexOffset];

            // If currentValue <= 0, propagate it's value to its 8 nodes
            if (currentValue <= 0) {
                
                //If value is already set, there is nothing to do
                if (value == -currentValue) return;
                
                //Get an available chunk of data
                size_t childPosition = getDataChunk(currentValue);                    

                data_[currentIndex + indexOffset] = childPosition;

                //Set the current value to the index of the new child node
                currentValue = childPosition;
            }

            // CurrentValue actually represent the index of the start of the next node
            parentStack.emplace_back(currentIndex, indexOffset); //Note: we save current index as the parent cell of the parent node
            currentIndex = currentValue; //We move the index to the child data chunk

        }

        uint8_t indexOffset = depthStep(position, currentLen);
        data_[currentIndex + indexOffset] = -value;

        while ( !parentStack.empty() ) {

            for (uint8_t i = currentIndex; i < (currentIndex+8); ++i) {
                if ( data_[i] != -value ) return;
            }

            //TODO: Il se passe quoi si on est à depth 0 ?
            freeDataChunk_.push_back(currentIndex);

            currentIndex = parentStack.back().first;

            //If we pass the precedent loop without returning, it mean the whole data chunk is the same value
            data_[currentIndex + parentStack.back().second] = -value;

            parentStack.pop_back();

        }

    }

    /**
     * Return a pair containing :
     *      - The position voxel value
     *      - The depth
    */
    std::pair<int32_t, uint8_t> SparseVoxelOctree::get(glm::uvec3 position) const {

        size_t currentIndex = 0;
        uint8_t currentDepth = 0;
        size_t currentLen = len_;

        while ( true ) {

            //This should never happen, the precedent test should always be true when we're at the last depth.
            #ifndef NDEBUG
            assert (currentDepth != depth_);
            #endif

            uint8_t indexOffset = depthStep(position, currentLen);
            currentIndex += indexOffset;

            //indicate index of the next depth
            int32_t currentValue = data_[currentIndex];

            //If true, we found the value of the given position
            if (currentValue <= 0) return {-currentValue, currentDepth};

            //If value not reached, we go to the next memory chunk
            currentIndex = currentValue;

            ++currentDepth;

        }

    }

    void SparseVoxelOctree::clearDataChunk(size_t index, int32_t clearValue) {
        for (uint8_t i = index; i < (index+8); ++i)
            data_[i] = clearValue;
    }

    // Return a disponible data chunk initialized with the given value
    size_t SparseVoxelOctree::getDataChunk(int32_t initializeValue) {
        
        size_t dataChunkPosition;

        if (freeDataChunk_.empty()) {
            dataChunkPosition = data_.size();
            data_.resize(dataChunkPosition + 8, initializeValue);
        }
        else {
            dataChunkPosition = freeDataChunk_.back();
            freeDataChunk_.pop_back();

            //Clear it because not new data chunk can still have data
            clearDataChunk(dataChunkPosition, initializeValue);
        }

        return dataChunkPosition;

    }

    /**
     * 
     * Position must be in the SVO space (with an assuming size of voxel of 1).
     * Given position and max distance can be adapted to respect this criteria.
     * 
     * Note: position will be edited to be equal to the current position of the ray.
     * 
     * direction input must be normalized
     * 
     * return:
     *      a tuple containing:
     *          - int: The return code :
     *              - 0 -> Collided with a voxel.
     *              - 1 -> No collision, max distance reached.
     *              - [1; 6] -> No collision, the value indicate on which side of the chunk the ray left.
     *          - float: Traveled distance
    */
    std::tuple<int, float, std::optional<SVOCollisionInformation>> SparseVoxelOctree::castRay(glm::vec3& position, glm::vec3 const& direction, float maxDistance) const {
        
        //This vector is used to find the correct side of the collided face
        static const std::vector<int> indexToSide {2, 4, 5, 0, 1, 3};

        //Positive or negative direction
        glm::vec3 directionSign;

        directionSign.x = std::signbit(direction.x) ? -1 : 1;
        directionSign.y = std::signbit(direction.y) ? -1 : 1;
        directionSign.z = std::signbit(direction.z) ? -1 : 1;

        //Index of the first voxel
        glm::ivec3 voxelPosition = getIntPosition(position, directionSign);

        //TODO: Maybe we could remove this or maybe not

        int movingSide = -1; //bottom = 0, front = 1, right = 2, back = 3, left = 4, top = 5
        uint32_t foundVoxel = 0;
        // Chunk* foundChunk = nullptr;

        float traveledDistance = 0.0f;

        uint8_t foundDepth = 0;
        float traveled;
        uint8_t minimumValueIndex = 0; //DEBUG: remove default value


        std::tie(foundVoxel, foundDepth) = get(voxelPosition);

        //We prevent a return movingSide == -1
        if (foundVoxel != 0) {

            for (uint8_t i = 0; i < 3; ++i) {
                if (std::trunc(position[i]) == position[i]) {
                    return {0, traveledDistance, SVOCollisionInformation{foundVoxel, voxelPosition, indexToSide[i*2 + (directionSign[i]>0)]}};
                }
            }

            //TODO: This will be corrected by implementing the chunk not found segment in world cast ray
            std::cout << "Big error" << std::endl;

            //TODO: add a big security that return side with the highest direction value.

        }

        while(foundVoxel == 0) {
            
            float leafSize = depthToLen_[foundDepth];

            //Required time to exit the full voxel along each axis.
            std::tie(traveled, minimumValueIndex) = getTraveledDistanceAndIndex(position, direction, directionSign, leafSize);             
            
            //Compute position offset and add it to the current position to find the new current position
            glm::vec3 positionOffset = direction * traveled;
            position += positionOffset;

            //Add the traveled distance to the total traveled distance
            traveledDistance += traveled;

            //Compute the new voxel position
            voxelPosition = getIntPosition(position, directionSign);

            if (traveledDistance > maxDistance) return {1, traveledDistance, {}};

            //Test if we went out of bound
            //Note: 2: x >= len, 3: x < 0, 4: y >= len, 5: y < 0, 6: z >= len, 7: z < 0
            for (uint8_t i = 0; i < 3; ++i) {
                if (voxelPosition[i] >= (int)len_) return {2 + i*2, traveledDistance, {}};
                if (voxelPosition[i] < 0) return {2 + i*2 + 1, traveledDistance, {}};
            }

            std::tie(foundVoxel, foundDepth) = get(voxelPosition);

        }

        //We only arrive here if found voxel is true
        return {0, traveledDistance, SVOCollisionInformation{foundVoxel, voxelPosition, indexToSide[minimumValueIndex*2 + (directionSign[minimumValueIndex]>0)]}};

    }

}
