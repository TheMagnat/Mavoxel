
#pragma once

#include <Mesh/Simple/Quad.hpp>
#include <VulkanWrapper/Shader.hpp>
#include <VulkanWrapper/Texture.hpp>

namespace mav {

    class FilterRenderer : public Quad {

        public:

            //TODO: rendre le nb de texture paramétrable
            FilterRenderer(Environment* environment, const std::vector<vuw::Texture>* textures, const std::vector<vuw::Texture>* lightTextures)
                : environment_(environment), textures_(textures), lightTextures_(lightTextures) {}

            void updateShader(vuw::Shader* shader, uint32_t currentFrame) const override {

                TestInformations ti;

                glm::vec4 viewPosition = environment_->camera->GetViewMatrix() * glm::vec4(environment_->sun->getPosition(), 1.0);
                glm::vec4 ndcPosition = environment_->camera->Projection * viewPosition;

                glm::vec2 screenPosition;

                screenPosition.x = (ndcPosition.x / ndcPosition.w + 1.0) * 0.5;
                screenPosition.y = (1.0 - ndcPosition.y / ndcPosition.w) * 0.5;

                ti.sunScreenPos = viewPosition.z <= 0 ? screenPosition : glm::vec2(300);

                shader->updateUniform(0, currentFrame, &ti, sizeof(ti));


                shader->updateTexture(0, &textures_->at(currentFrame));
                shader->updateTexture(1, &lightTextures_->at(currentFrame));
                
                shader->updateDescriptorSets(currentFrame);

            }

        private:
            Environment* environment_;

            const std::vector<vuw::Texture>* textures_;
            const std::vector<vuw::Texture>* lightTextures_;
    };

}
