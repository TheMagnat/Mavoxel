
#include <vector>
#include <glm/vec3.hpp>


class SparseVoxelOctree {

    public:
        SparseVoxelOctree(size_t depth)
            : depth_(depth), size_(std::pow(8, depth)), len_(std::pow(2, depth)), data_(8, 0)  {

            #ifndef NDEBUG
                assert(depth_ > 0);
            #endif

        }

        // void initFirst() {
            
        //     //Initialize the first 8 chunks
        //     data_.resize(8, 0);

        // }

        uint8_t depthStep(glm::uvec3& position, size_t& currentLen) {

            currentLen /= 2;
            uint8_t xOffset = 0, yOffset = 0, zOffset = 0;

            if(position.x >= currentLen) {
                xOffset = 1;
                position.x -= currentLen;
            }

            if(position.y >= currentLen){
                yOffset = 1;
                position.y -= currentLen;
            }

            if(position.z >= currentLen){
                zOffset = 1;
                position.z -= currentLen;
            }

            return xOffset + yOffset * 2 + zOffset * 4;

        }

        void set(glm::uvec3 position, int32_t value) {

            // if (data_.empty()) {
            //     initFirst();
            // }

            insert(position, value);


        }

        int32_t get(glm::uvec3 position) {
            
            //If empty, it mean the whole Octree is empty so we return 0
            // if (data_.empty()) {
            //     return 0;
            // }

            size_t currentIndex = 0;
            uint8_t currentDepth = 0;
            size_t currentLen = len_;

            while ( true ) {

                ++currentDepth;

                uint8_t indexOffset = depthStep(position, currentLen);
                currentIndex += indexOffset;

                //indicate index of the next depth
                int32_t currentValue = data_[currentIndex];

                //If true, we found the value of the given position
                if (currentValue <= 0) return -currentValue;

                //If value not reached, we go to the next memory chunk
                currentIndex = currentValue;

                //This should never happen, the precedent test should always be true when we're at the last depth.
                #ifndef NDEBUG
                assert (currentDepth != depth_);
                #endif

            }



        }

        void clearDataChunk(size_t index, int32_t clearValue) {
            for (uint8_t i = index; i < (index+8); ++i)
                data_[i] = clearValue;
        }

        // Return a disponible data chunk initialized with the given value
        size_t getDataChunk(int32_t initializeValue) {
            
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

        void insert(glm::uvec3& position, int32_t value) {
            
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



    private:
        std::vector<int32_t> data_;
        size_t depth_;
        size_t size_;
        size_t len_;

        //This vector store all the position of data chunk that were released 
        std::vector<size_t> freeDataChunk_;
};
