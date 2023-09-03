
#pragma once

#include <Mesh/Simple/Quad.hpp>
#include <VulkanWrapper/Shader.hpp>
#include <VulkanWrapper/Texture.hpp>


namespace mav {

    class FilterRenderer : public Quad {

        public:

            //TODO: rendre le nb de texture paramétrable
            FilterRenderer(Environment* environment, const std::vector<vuw::Texture>* textures, const std::vector<std::vector<vuw::Texture>>* additionalTextures)
                : environment_(environment), textures_(textures), additionalTextures_(additionalTextures) {}

            void setTextures(const std::vector<vuw::Texture>* textures, const std::vector<std::vector<vuw::Texture>>* additionalTextures) {
                textures_ = textures;
                additionalTextures_ = additionalTextures;
            }

            void updateShader(vuw::Shader* shader, uint32_t currentFrame) override {

                TestInformations ti;

                //Retrieve the old matrix from the old new matrix then calculate the new matrix
                filterShaderInformations.view = environment_->camera->GetViewMatrix();
                filterShaderInformations.oldProjectionView = filterShaderInformations.newProjectionView;
                filterShaderInformations.newProjectionView = environment_->camera->Projection * filterShaderInformations.view;

                glm::vec4 viewPosition = filterShaderInformations.view * glm::vec4(environment_->sun->getPosition(), 1.0);
                
                glm::vec4 ndcPosition = environment_->camera->Projection * viewPosition;

                glm::vec2 screenPosition;

                screenPosition.x = (ndcPosition.x / ndcPosition.w + 1.0) * 0.5;
                screenPosition.y = (1.0 - ndcPosition.y / ndcPosition.w) * 0.5;

                ti.sunScreenPos = viewPosition.z <= 0 ? screenPosition : glm::vec2(300);

                //Update uniforms...
                shader->updateUniform(0, currentFrame, &ti, sizeof(ti));
                shader->updateUniform(1, currentFrame, &filterShaderInformations, sizeof(filterShaderInformations));
                
                //Update textures...
                shader->updateTexture(0, &textures_->at(currentFrame));

                for (size_t i = 0; i < additionalTextures_->size(); ++i) {
                    shader->updateTexture(i + 1, &additionalTextures_->at(i)[currentFrame]);
                }
                
                shader->updateDescriptorSets(currentFrame);

            }

            static void initializeShaderLayout(vuw::Shader* shader, const vuw::SceneRenderer* sceneRenderer) {

                //Global filter
                shader->addTexture({
                    vuw::TextureShaderInformation{sceneRenderer->getTextures().front().getInformations(), 0, &sceneRenderer->getTextures().front()},
                });
                shader->addTexture({
                    vuw::TextureShaderInformation{sceneRenderer->getAdditionalTextures()[0].front().getInformations(), 1, &sceneRenderer->getAdditionalTextures()[0].front()},
                });
                shader->addTexture({
                    vuw::TextureShaderInformation{sceneRenderer->getAdditionalTextures()[1].front().getInformations(), 2, &sceneRenderer->getAdditionalTextures()[1].front()},
                });
                shader->addUniformBufferObjects({
                    {3, sizeof(mav::TestInformations), VK_SHADER_STAGE_FRAGMENT_BIT},
                    {4, sizeof(mav::FilterInformations), VK_SHADER_STAGE_FRAGMENT_BIT},
                });
                
            }

        private:
            Environment* environment_;

            FilterInformations filterShaderInformations;

            const std::vector<vuw::Texture>* textures_;
            const std::vector<std::vector<vuw::Texture>>* additionalTextures_;
    };

}
