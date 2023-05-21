
#include <World/World.hpp>
#include <Mesh/Simple/Quad.hpp>
#include <VulkanWrapper/Shader.hpp>

namespace mav {

    class RayCastingRenderer : public Quad {

        public:
            RayCastingRenderer(World* world, Environment* environment)
                : world_(world), environment_(environment), Quad(environment_),
                emptyTexture_(world->getChunkSize()), emptyDataMatrix_(std::pow(world->getChunkSize(), 3), 0) {

                emptyTexture_.setData(emptyDataMatrix_);

            }

            void updateUniforms(vuw::Shader* shader, uint32_t currentFrame) const override {
                
                RayCastInformations rci;

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

                shader->updateUniform(0, currentFrame, &rci, sizeof(rci));

            }


            void draw() {
                std::cout << "Deprecated RC draw" << std::endl;

                // vao_.bind();

                // shader_->use();



                // Bind the chunk texture to texture unit 0
                //Chunk* centerChunk = world_->getChunkFromWorldPos(environment_->camera->Position);
                glm::ivec3 centerPosition = world_->getChunkIndex(environment_->camera->Position);

                static const int range = 2;
                int len = range * 2 + 1;

                unsigned int textureUnit = 0;
                std::vector<int> textureUnits;
                textureUnits.reserve(len * len * len);
                for (int x = -range; x <= range; ++x) {
                for (int y = -range; y <= range; ++y) {
                for (int z = -range; z <= range; ++z) {
                    
                    Chunk* chunkPtr = world_->getChunk(centerPosition.x + x, centerPosition.y + y, centerPosition.z + z);
                    if (chunkPtr) {
                        
                        chunkPtr->getTexture()->bind(textureUnit);

                        
                        
                    }
                    else {
                        emptyTexture_.bind(textureUnit);
                    }
                    
                    textureUnits.push_back(textureUnit++);

                }
                }
                }

                // Set the value of the sampler3D uniforms variables
                // shader_->setIntV("chunkTextures", textureUnits);
                // shader_->setIVec3("centerChunkPosition", centerPosition);

                // shader_->setFloat("xRatio", (float)Global::width/(float)Global::height);
                // shader_->setVec3("camera.position", environment_->camera->Position);
                // shader_->setVec3("camera.front", environment_->camera->Front);
                // shader_->setVec3("camera.up", environment_->camera->Up);
                // shader_->setVec3("camera.right", environment_->camera->Right);

                // shader_->setVec3("sun.position", environment_->sun->getPosition());

                // glDrawElements(GL_TRIANGLES, indicesSize_, GL_UNSIGNED_INT, 0);

            }

        private:
            World* world_;
            Environment* environment_;

            //Empty texture
            Texture3D emptyTexture_;
            std::vector<uint8_t> emptyDataMatrix_;

    };

}
