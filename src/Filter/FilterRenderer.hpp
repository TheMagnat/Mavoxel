
#pragma once

#include <Mesh/Simple/Quad.hpp>
#include <VulkanWrapper/Shader.hpp>
#include <VulkanWrapper/Texture.hpp>


namespace mav {

    class FilterRenderer : public Quad {

        public:

            //TODO: rendre le nb de texture paramétrable
            FilterRenderer(Environment* environment) : environment_(environment) {}

            /**
             * Textures is a vector of pointer of vector of textures.
             * It contain the texture (for each frame in flight) the shader use in input.
             * 
             * nbOwnedTextures correspond to the number of textures the renderer own itself.
             */
            FilterRenderer(Environment* environment, std::vector<const std::vector<vuw::Texture>*> const& textures, std::vector<vuw::Texture::TextureInformations> const& ownedTexturesInformations = std::vector<vuw::Texture::TextureInformations>())
                : environment_(environment), textures_(textures), ownedTexturesInformations_(ownedTexturesInformations) {}

            void setTextures(std::vector<const std::vector<vuw::Texture>*> const& textures, std::vector<vuw::Texture::TextureInformations> const& ownedTexturesInformations = std::vector<vuw::Texture::TextureInformations>()) {
                textures_ = textures;

                //Note: if we change the format of the already generated textures, it will be ignored. Maybe we should change this
                for (size_t i = ownedTextures_.size(); i < ownedTexturesInformations.size(); ++i) {
                    generateOwnedTexture(ownedTexturesInformations[i]);
                }

            }

            void generateOwnedTexture(vuw::Texture::TextureInformations const& textureInformations) {
                
                //Generate empty vector...
                ownedTextures_.emplace_back();

                //...And fill the empty vector
                while (ownedTextures_.back().size() < textures_.front()->size()) {
                    
                    ownedTextures_.back().push_back(
                        mav::Global::vulkanWrapper->generateEmptyTexture(textureInformations, textureInformations.imageFormat, VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                    );
                    mav::Global::vulkanWrapper->transitionImageLayout(ownedTextures_.back().back().getImage(), textureInformations.imageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                }

            }

            void copyIntoOwnedTexture(uint32_t lastFrameIndex, vuw::Texture const& srcTexture, size_t ownedTextureIndex) {

                lastFrameIndex_ = lastFrameIndex;

                //Copy last frame texture
                vuw::Texture::TextureInformations const& textureInformation = srcTexture.getInformations();
                mav::Global::vulkanWrapper->copyImageToImage( srcTexture.getImage(), ownedTextures_[ownedTextureIndex][lastFrameIndex_].getImage(), textureInformation.width, textureInformation.height );
                
                // //Same for velocity texture...
                // vuw::Texture::TextureInformations const& velocityTextureInformation = srcVelocityTexture.getInformations();
                // mav::Global::vulkanWrapper->copyImageToImage( srcVelocityTexture.getImage(), lastFrameVelocityTextures_[lastFrameIndex_].getImage(), velocityTextureInformation.width, velocityTextureInformation.height );

            }

            void updateShader(vuw::Shader* shader, uint32_t currentFrame) override {

                TestInformations ti;

                //Retrieve the old matrix from the old new matrix then calculate the new matrix
                filterShaderInformations.view = environment_->camera->GetViewMatrix();
                filterShaderInformations.oldProjectionView = filterShaderInformations.newProjectionView;
                filterShaderInformations.newProjectionView = environment_->camera->Projection * filterShaderInformations.view;
                filterShaderInformations.velocityScalar = environment_->velocityScalar;
                filterShaderInformations.debug = false;//fmod(environment_->totalElapsedTime, 4) < 2.0;

                glm::vec4 viewPosition = filterShaderInformations.view * glm::vec4(environment_->sun->getPosition(), 1.0);
                
                glm::vec4 ndcPosition = environment_->camera->Projection * viewPosition;

                glm::vec2 screenPosition;

                screenPosition.x = (1.0 - ndcPosition.x / ndcPosition.w) * 0.5;
                screenPosition.y = (1.0 - ndcPosition.y / ndcPosition.w) * 0.5;

                ti.sunScreenPos = viewPosition.z <= 0 ? screenPosition : glm::vec2(300);

                //Update uniforms...
                shader->updateUniform(0, currentFrame, &ti, sizeof(ti));
                shader->updateUniform(1, currentFrame, &filterShaderInformations, sizeof(filterShaderInformations));
                
                //Update textures...
                for (size_t i = 0; i < textures_.size(); ++i) {
                    shader->updateTexture(i, &textures_[i]->at(currentFrame));
                }

                for (size_t i = 0; i < ownedTextures_.size(); ++i) {
                    shader->updateTexture(textures_.size() + i, &ownedTextures_[i][currentFrame]);
                }
                
                shader->updateDescriptorSets(currentFrame);

            }

            void initializeShaderLayout(vuw::Shader* shader) {

                // std::vector<> textures_;


                shader->addUniformBufferObjects({
                    {0, sizeof(mav::TestInformations), VK_SHADER_STAGE_FRAGMENT_BIT},
                    {1, sizeof(mav::FilterInformations), VK_SHADER_STAGE_FRAGMENT_BIT},
                });
                uint32_t nbUniforms = 2;

                uint32_t texturesFullSize = textures_.size() + ownedTextures_.size();

                for (uint32_t i = 0; i < texturesFullSize; ++i) {
                    
                    //Select between textures and ownedTextures
                    vuw::Texture const& currentTexture = i < textures_.size() ? textures_[i]->front() : ownedTextures_[i - textures_.size()].front();

                    shader->addTexture({
                        vuw::TextureShaderInformation{ currentTexture.getInformations(), i + nbUniforms, &currentTexture }
                    });

                }
                
            }

        private:
            Environment* environment_;

            FilterInformations filterShaderInformations;

            std::vector<const std::vector<vuw::Texture>*> textures_;
            
            //Save the last frame index to know which owned texture to use
            int lastFrameIndex_ = 0;
            std::vector<vuw::Texture::TextureInformations> ownedTexturesInformations_;
            std::vector<std::vector<vuw::Texture>> ownedTextures_;

    };

}
