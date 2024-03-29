
#pragma once

#include <World/WorldLoader.hpp>
#include <World/EntityManager.hpp>
#include <Mesh/Simple/Quad.hpp>
#include <VulkanWrapper/Shader.hpp>
#include <GraphicObjects/BufferTemplates.hpp>

#include <glm/gtx/vector_angle.hpp>

//RAYTRACING_CHUNK_RANGE is set in the CMAKE
#define RAYTRACING_CHUNK_PER_AXIS (RAYTRACING_CHUNK_RANGE * 2 + 1)
#define RAYTRACING_SVO_SIZE (RAYTRACING_CHUNK_PER_AXIS * RAYTRACING_CHUNK_PER_AXIS * RAYTRACING_CHUNK_PER_AXIS)


//TODO: mettre dans un fichier helper ou un truc du genre
namespace {

    float halton(uint32_t i, uint32_t b) {

        float f = 1.0f;
        float r = 0.0f;
    
        while (i > 0)
        {
            f /= static_cast<float>(b);
            r = r + f * static_cast<float>(i % b);
            i = static_cast<uint32_t>(floorf(static_cast<float>(i) / static_cast<float>(b)));
        }
    
        return r;
    }

}


namespace mav {

    class RayCastingRenderer : public Quad {

        public:
            RayCastingRenderer(const WorldLoader* worldLoader, EntityManager* entityManager, Environment* environment, size_t svoDepth, float voxelSize)
                : svoDepth_(svoDepth), voxelSize_(voxelSize), worldLoader_(worldLoader), entityManager_(entityManager), environment_(environment) {


            }

            void updateShader(vuw::Shader* shader, uint32_t currentFrame) override {
                
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

                rci.projection = environment_->camera->Projection;
                rci.view = environment_->camera->GetViewMatrix();

                //Computation for camera velocity...
                glm::vec3 displacement_vector = rci.camera.position - lastFrameCameraPosition_;
                float linearVelocity = glm::length(displacement_vector);

                float angularVelocity = glm::angle(lastFrameCameraDirection_, rci.camera.front);

                // Scalar velocity (you can adjust the weights as needed)
                float velocityScalar = linearVelocity + angularVelocity * 20.0f;
                velocityScalar = glm::clamp(velocityScalar * 10.0f, 0.0f, 1.0f);
                // velocityScalar = 1.0f;

                environment_->velocityScalar = velocityScalar;

                rci.velocityScalar = velocityScalar;

                // std::cout << "Scalar velocity: " << rci.velocityScalar << std::endl;

                //...
                lastFrameCameraPosition_ = rci.camera.position;
                lastFrameCameraDirection_ = rci.camera.front;

                //Sun informations
                rci.sun.position = environment_->sun->getPosition();

                //Time
                rci.time = environment_->totalElapsedTime;

                if (environment_->collisionInformations) {
                    
                    //TODO: better
                    rci.voxelCursorPosition = ( glm::vec3(environment_->collisionInformations->voxelLocalPosition) + (glm::vec3(environment_->collisionInformations->chunkPosition) * std::pow(2, svoDepth_)) ) * voxelSize_;
                    rci.faceCursorNormal = environment_->collisionInformations->normal;
                }
                else {
                    rci.voxelCursorPosition = glm::vec3(0.0f);
                    rci.faceCursorNormal = glm::vec3(0.0f);
                }

                ++iteration_;

                float haltonX = 2.0f * halton(iteration_ + 1, 2) - 1.0f;
                float haltonY = 2.0f * halton(iteration_ + 1, 3) - 1.0f;

                //TODO: ne plus mettre en dur la résolution
                float weight = 2.5f;
                rci.jitter.x = (haltonX / (1080 * weight));
                rci.jitter.y = (haltonY / (1080 * weight));
                // rci.jitter = glm::vec2(0.0);

                //Current chunk position
                woi.centerChunkPosition = worldLoader_->getCenterPosition();
                woi.depth = svoDepth_;
                woi.len = std::pow(2, svoDepth_);
                woi.voxelSize = voxelSize_;

                //Uniform on binding 0
                shader->updateUniform(0, currentFrame, &rci, sizeof(rci));

                //Uniform on binding 1
                shader->updateUniform(1, currentFrame, &woi, sizeof(woi));


                //Buffers
                shader->updateSSBOs(0, worldLoader_->getAroundChunks().octreeSSBOs); //Binding 2
                shader->updateUniformArray(2, currentFrame, worldLoader_->getAroundChunks().octreeInformations, sizeof(int)); //Binding 3
                
                shader->updateSSBOs(1, std::vector<const vuw::SSBO*>{ &entityManager_->getSSBO() }); //Binding 4

                //TODO: Vérifier if modification entre frame d'avant et mtn sur les ptr de ssbo utilisé, et si oui call "updateDescriptorSets"
                
                shader->updateDescriptorSets(currentFrame);

            }


            static void initializeShaderLayout(vuw::Shader* shader) {

                //Ray Casting
                shader->addUniformBufferObjects({
                    {0, sizeof(mav::RayCastInformations), VK_SHADER_STAGE_FRAGMENT_BIT},
                    {1, sizeof(mav::WorldOctreeInformations), VK_SHADER_STAGE_FRAGMENT_BIT}
                });
                shader->addSSBO({
                    //Binding, Stage flag, count, ssbo ptr
                    2, VK_SHADER_STAGE_FRAGMENT_BIT, RAYTRACING_SVO_SIZE, std::vector<const vuw::SSBO*>(RAYTRACING_SVO_SIZE, nullptr)
                });
                shader->addUniformBufferObjects({
                    {3, sizeof(int), VK_SHADER_STAGE_FRAGMENT_BIT, RAYTRACING_SVO_SIZE},
                });
                shader->addSSBO({
                    //Binding, Stage flag, count, ssbo ptr
                    4, VK_SHADER_STAGE_FRAGMENT_BIT, 1, std::vector<const vuw::SSBO*>(1, nullptr)
                });

            }

        private:

            size_t svoDepth_;
            float voxelSize_;

            const WorldLoader* worldLoader_;
            EntityManager* entityManager_;
            Environment* environment_;

            int iteration_ = 0;

            //Save
            glm::vec3 lastFrameCameraPosition_;
            glm::vec3 lastFrameCameraDirection_;

    };

}
