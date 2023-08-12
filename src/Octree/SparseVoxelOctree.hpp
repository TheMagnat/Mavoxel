
#pragma once


#include <Octree/SparseVoxelOctreeHelper.hpp>
#include <VulkanWrapper/SSBO.hpp>


#include <glm/vec3.hpp>

#include <vector>
#include <optional>
#include <fstream>

namespace mav {

    class SparseVoxelOctree {

        public:
            SparseVoxelOctree(size_t depth);

            void set(glm::uvec3 position, int32_t value);

            /**
             * Return a pair containing :
             *      - The position voxel value
             *      - The depth
            */
            std::pair<int32_t, uint8_t> get(glm::uvec3 position) const;

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
            std::tuple<int, float, std::optional<SVOCollisionInformation>> castRay(glm::vec3& position, glm::vec3 const& direction, float maxDistance, std::vector<glm::vec3>& normals) const;

            inline size_t getLen() { return len_; }
            inline size_t getSize() { return size_; }
            inline size_t getDepth() { return depth_; }

            //Vulkan part

            /**
             * Update the SSBO GPU buffer. If it's not created, it will also create it. 
             */
            void updateBuffer() {
                gpuBuffer_.update<int32_t>(data_);
            }

            inline vuw::SSBO const& getSSBO() { return gpuBuffer_; }

            //File handler
            void writeToFile(std::ofstream& stream) const;
            void readFromFile(std::ifstream& stream);

        private:
            uint8_t depthStep(glm::uvec3& position, size_t& currentLen) const;

            void clearDataChunk(size_t index, int32_t clearValue);

            // Return a disponible data chunk initialized with the given value
            size_t getDataChunk(int32_t initializeValue);

        private:

            std::vector<int32_t> data_;
            size_t depth_;
            size_t size_;
            size_t len_;

            //This vector allow a fast leaf depth to len access
            std::vector<float> depthToLen_;

            //This vector store all the position of data chunk that were released 
            std::vector<size_t> freeDataChunk_;

            //Vulkan
            vuw::SSBO gpuBuffer_;
    };

}
