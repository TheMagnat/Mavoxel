#pragma once

#include <VulkanWrapper/VertexData.hpp>
#include <VulkanWrapper/Shader.hpp>

#include <vector>

namespace mav {

    class Drawable {

        public:
            Drawable();

            bool empty();

            //To generate vertices
            virtual void generateVertices() = 0;

            /**
             * Generate vertices and call graphic update to set graphic buffer data
            */
            void initialize();

            //To update the Vertex Data
            void graphicUpdate();

            virtual void draw(VkCommandBuffer commandBuffer) const;

            //Virtual global graphics setup
            virtual std::vector<uint32_t> getVertexAttributesSizes() const = 0;
            virtual void updateUniforms(vuw::Shader* shader, uint32_t currentFrame) const = 0;

        protected:
            vuw::VertexData vertexData_;

            std::vector<float> vertices_;            
            std::vector<uint32_t> indices_;

        };

}