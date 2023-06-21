
#include <World/World.hpp>
#include <Mesh/Simple/Quad.hpp>
#include <VulkanWrapper/Shader.hpp>

//TODO: set them in CMAKE
#define RAYTRACING_CHUNK_RANGE 2
#define RAYTRACING_CHUNK_PER_AXIS (RAYTRACING_CHUNK_RANGE * 2 + 1)
#define RAYTRACING_SVO_SIZE (RAYTRACING_CHUNK_PER_AXIS * RAYTRACING_CHUNK_PER_AXIS * RAYTRACING_CHUNK_PER_AXIS)

namespace mav {

    class RayCastingRenderer : public Quad {

        public:
            RayCastingRenderer(World* world, Environment* environment, size_t svoDepth)
                : svoDepth_(svoDepth), world_(world), environment_(environment), Quad(environment_) {


            }

            void updateShader(vuw::Shader* shader, uint32_t currentFrame) const override {
                
                //Uniforms
                RayCastInformations rci;
                WorldOctreeInformations woi;

                //Screen ratio
                VkExtent2D const& extent = Global::vulkanWrapper->getExtent();
                rci.xRatio = (float)extent.width/(float)extent.height;

                //Camera
                rci.camera.position = environment_->camera->Position;
                rci.camera.front = environment_->camera->Front;
                rci.camera.up = environment_->camera->Up;
                rci.camera.right = environment_->camera->Right;

                //Sun informations
                rci.sun.position = environment_->sun->getPosition();

                //Time
                rci.time = environment_->totalElapsedTime;

                if (environment_->collisionInformations) {
                    
                    //TODO: better
                    rci.voxelCursorPosition = ( glm::vec3(environment_->collisionInformations->voxelLocalPosition) + (glm::vec3(environment_->collisionInformations->chunkPosition) * std::pow(2, svoDepth_)) - (float)(std::pow(2, svoDepth_) / 2.0) ) * world_->getVoxelSize();
                    rci.faceCursorNormal = environment_->collisionInformations->normal;
                }   
                else {
                    rci.voxelCursorPosition = glm::vec3(0.0f);
                    rci.faceCursorNormal = glm::vec3(0.0f);
                }
                

                //Current chunk position
                woi.centerChunkPosition = world_->getChunkIndex(environment_->camera->Position);
                woi.depth = svoDepth_;
                woi.len = std::pow(2, svoDepth_);
                woi.voxelSize = world_->getVoxelSize();

                //Uniform on binding 0
                shader->updateUniform(0, currentFrame, &rci, sizeof(rci));

                //Uniform on binding 1
                shader->updateUniform(1, currentFrame, &woi, sizeof(woi));


                //Buffers
                static const int range = RAYTRACING_CHUNK_RANGE;
                static const int len = RAYTRACING_CHUNK_PER_AXIS;

                std::vector<const vuw::SSBO*> svoSsboInRange;
                std::vector<mav::SparseVoxelOctree*> ssboDebug;
                std::vector<int> svoInformations;
                svoSsboInRange.reserve(RAYTRACING_SVO_SIZE);
                
                unsigned int ssboIndex = 0;
                for (int x = -range; x <= range; ++x) {
                for (int y = -range; y <= range; ++y) {
                for (int z = -range; z <= range; ++z) {
                    
                    Chunk* chunkPtr = world_->getChunk(woi.centerChunkPosition.x + x, woi.centerChunkPosition.y + y, woi.centerChunkPosition.z + z);
                    if (chunkPtr && chunkPtr->state == 2) {
                        svoSsboInRange.push_back(&chunkPtr->svo_.getSSBO());
                        svoInformations.push_back(0);
                        ssboDebug.push_back(&chunkPtr->svo_);
                    }
                    else {
                        svoSsboInRange.push_back(nullptr);
                        svoInformations.push_back(1);
                        ssboDebug.push_back(nullptr);
                    }
                    
                }
                }
                }

                shader->updateUniformArray(2, currentFrame, svoInformations, sizeof(int));

                shader->updateSSBOs(0, svoSsboInRange);

                //TODO: Vérifier if modification entre frame d'avant et mtn sur les ptr de ssbo utilisé, et si oui call "updateDescriptorSets"
                
                shader->updateDescriptorSets(currentFrame);

            }


        private:

            size_t svoDepth_;

            World* world_;
            Environment* environment_;

    };

}
