
#pragma once

#include <Core/Global.hpp>
#include <VulkanWrapper/SSBO.hpp>

#include <glm/vec3.hpp>

#include <vector>

namespace mav {
    
    //TODO: peut être créer une classe mère en commun avec les SVO pour la partie GPU (getSSBO, updateBuffer...)
    class VoxelManager {

        public:
            
            struct SimpleVoxel {
                glm::vec3 position;
            };


            VoxelManager() : gpuBuffer_(Global::vulkanWrapper->generateSSBO()) {}

            void addVoxel(SimpleVoxel const& voxelToAdd) {
                voxels_.push_back(voxelToAdd);
                updateBuffer();
            }


            //GPU part
            inline vuw::SSBO const& getSSBO() { return gpuBuffer_; }

            void updateBuffer() {
                gpuBuffer_.update<SimpleVoxel>(voxels_);
            }

        private:

            std::vector<SimpleVoxel> voxels_;

            vuw::SSBO gpuBuffer_;

    };

}
