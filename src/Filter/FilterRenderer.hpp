
#pragma once

#include <Mesh/Simple/Quad.hpp>
#include <VulkanWrapper/Shader.hpp>
#include <VulkanWrapper/Texture.hpp>

namespace mav {

    class FilterRenderer : public Quad {

        public:

            FilterRenderer(const std::vector<vuw::Texture>* textures) : textures_(textures) {}

            void updateShader(vuw::Shader* shader, uint32_t currentFrame) const override {
                
                shader->updateTexture(0, &textures_->at(currentFrame));
                
                shader->updateDescriptorSets(currentFrame);

            }

        private:
            const std::vector<vuw::Texture>* textures_;
    };

}
