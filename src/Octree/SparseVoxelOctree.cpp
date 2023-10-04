
#include <Octree/SparseVoxelOctree.hpp>

#include <iostream>
#include <World/WorldPositionHelper.hpp>
#include <Octree/SparseVoxelOctreeHelper.hpp>
#include <Core/Global.hpp>

//File saving / compression
#include <zlib/zlib.h>

#ifdef TIME
    //Help/debug
    #include <Helper/Benchmark/Profiler.hpp>
#endif

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
    std::tuple<int, float, std::optional<SVOCollisionInformation>> SparseVoxelOctree::castRay(glm::vec3& position, glm::vec3 const& direction, float maxDistance, std::vector<glm::vec3>& normals) const {
        
        //This vector is used to find the correct side of the collided face
        static const std::vector<int> indexToSide {2, 4, 5, 0, 1, 3};

        //Positive or negative direction
		glm::vec3 directionSign = glm::step(0.0f, direction) * 2.0f - 1.0f;
		glm::vec3 directionSignWithZero = glm::sign(direction);

        //Index of the first voxel
        glm::ivec3 voxelPosition = getIntPosition(position, directionSign);

        uint32_t foundVoxel;
        uint8_t foundDepth;
        float traveledDistance = 0.0f;

        //Found a voxel before starting to move, the ray started in the SVO or perfectly at the limit between a voxel and an empty space.
        std::tie(foundVoxel, foundDepth) = get(voxelPosition);
        if (foundVoxel != 0) {
            
            //This allow finding the axis causing the instant collision.
            normals.clear();
            for (uint8_t i = 0; i < 3; ++i) {
                if (std::trunc(position[i]) == position[i] && directionSignWithZero[i] != 0) {
                    normals.emplace_back(0.0f);
                    normals.back()[i] = -directionSignWithZero[i];
                }
            }

            //If no axis found, the Ray started inside the SVO.
            if (normals.empty()) {
                std::cout << "Ray-Casting starting inside the SVO." << std::endl;
                return {1, maxDistance, {}};
            }
            
            return {0, traveledDistance, SVOCollisionInformation{foundVoxel, voxelPosition}};

        }

        float traveled;
        uint8_t minimumValueIndex;
        glm::vec3 crossedSides;

        int count = 0;
        while(foundVoxel == 0) {

            if (count++ > 100) {
                std::cout << "Infinite SVO loop" << std::endl;
            }

            float leafSize = depthToLen_[foundDepth];

            //Required time to exit the full voxel along each axis.
            std::tie(traveled, minimumValueIndex) = getShortestTraveledDistanceAndIndex(position, direction, directionSign, leafSize);             
            
            //Compute position offset and add it to the current position to find the new current position
            glm::vec3 positionOffset = direction * traveled;

            position += positionOffset;
            roundPosition(position);

            glm::vec3 fractionPart = glm::fract(glm::mod(position, leafSize));
            //Note: This line may be useless since we put good values for the round (it may even be equivalent to the COMPARISON DELTA). We could just use the equal now.
            // crossedSides = glm::lessThan(fractionPart, glm::vec3(COMPARISON_DELTA)) + glm::greaterThan(fractionPart, glm::vec3(1.0f - COMPARISON_DELTA));
            crossedSides = glm::equal(fractionPart, glm::vec3(0.0f));
            crossedSides[minimumValueIndex] = 1.0f; //This line add a security if the precision is too low in the precedent operation

            //Add the traveled distance to the total traveled distance
            traveledDistance += traveled;

            //Compute the new voxel position
            voxelPosition = getIntPosition(position, directionSign);

            if (traveledDistance >= maxDistance) return {1, traveledDistance, {}};

            //Verify if we went out of the SVO
            for (uint8_t i = 0; i < 3; ++i) {
                if (voxelPosition[i] >= (int)len_ && directionSignWithZero[i] != 0){
                    normals.emplace_back(0.0f);
                    normals.back()[minimumValueIndex] = -directionSignWithZero[minimumValueIndex];
                    return { 2 + i*2, traveledDistance, {} };
                }
                if (voxelPosition[i] < 0 && directionSignWithZero[i] != 0) {
                    normals.clear();
                    normals.emplace_back(0.0f);
                    normals.back()[minimumValueIndex] = -directionSignWithZero[minimumValueIndex];
                    return { 2 + i*2 + 1, traveledDistance, {} };
                }
            }

            std::tie(foundVoxel, foundDepth) = get(voxelPosition);

        }

        //Calculate normals
        normals.clear();
        for (uint8_t i = 0; i < 3; ++i) {
            if (crossedSides[i] != 0 && directionSignWithZero[i] != 0) {
                normals.emplace_back(0.0f);
                normals.back()[i] = -directionSignWithZero[i];
            }
        }

        //We only arrive here if found voxel is true
        return {0, traveledDistance, SVOCollisionInformation{foundVoxel, voxelPosition}};

    }

    //File handler
    void SparseVoxelOctree::writeToFile(std::ofstream& stream) const {
        
        //Save original size
        size_t dataOriginalSize = data_.size();
        size_t dataOriginalByteSize = dataOriginalSize * sizeof(int32_t);

        uLong dataCompressedSize_uLong;
        std::vector<unsigned char> compressedData;

        {
                
            #ifdef TIME
                Profiler profiler("Octree compression");
            #endif

            dataCompressedSize_uLong = compressBound(dataOriginalByteSize); //Get maximum compressed buffer size
            compressedData.resize(dataCompressedSize_uLong);

            int result = compress2(compressedData.data(), &dataCompressedSize_uLong, reinterpret_cast<const Bytef*>(data_.data()), dataOriginalByteSize, 1); //Compress buffer
            compressedData.resize(dataCompressedSize_uLong); //Resize the compressed data according to the true compressed size

        }

        size_t dataCompressedSize = dataCompressedSize_uLong; //Convert uLong to size_t to stay consistent

        stream.write(reinterpret_cast<const char*>(&dataOriginalSize), sizeof(size_t));
        stream.write(reinterpret_cast<const char*>(&dataCompressedSize), sizeof(size_t));
        stream.write(reinterpret_cast<const char*>(compressedData.data()), sizeof(unsigned char) * dataCompressedSize);

        //Save uncompressed freeDataChunk vector
        size_t freeDataChunkSize = freeDataChunk_.size();
        stream.write(reinterpret_cast<const char*>(&freeDataChunkSize), sizeof(size_t));
        stream.write(reinterpret_cast<const char*>(freeDataChunk_.data()), sizeof(size_t) * freeDataChunk_.size());

    }

    void SparseVoxelOctree::readFromFile(std::ifstream& stream) {
        
        //1 - Read original size
        size_t dataOriginalSize;
        stream.read(reinterpret_cast<char*>(&dataOriginalSize), sizeof(size_t));

        //2 - Read compressed size
        size_t dataCompressedSize;
        stream.read(reinterpret_cast<char*>(&dataCompressedSize), sizeof(size_t));

        //3 - Read compressed octree
        std::vector<unsigned char> compressedData;
        compressedData.resize(dataCompressedSize); //Prepare buffer to receive data

        stream.read(reinterpret_cast<char*>(compressedData.data()), sizeof(unsigned char) * dataCompressedSize);

        // Uncompress octree into data member
        data_.resize(dataOriginalSize);
        uLong originalULongSize = dataOriginalSize * sizeof(size_t);
        uncompress(reinterpret_cast<Bytef*>(&data_[0]), &originalULongSize, compressedData.data(), dataCompressedSize);

        //4 - Read free chunk size
        size_t freeDataChunkSize;
        stream.read(reinterpret_cast<char*>(&freeDataChunkSize), sizeof(size_t));

        //5 - Read free chunk vector
        freeDataChunk_.resize(freeDataChunkSize);
        stream.read(reinterpret_cast<char*>(freeDataChunk_.data()), sizeof(int32_t) * freeDataChunkSize);

    }

}
