
#pragma once

#include <VulkanWrapper/GraphicsPipeline.hpp>
#include <VulkanWrapper/MultiShader.hpp>

#include <Core/Global.hpp>

#include <vector>

namespace mav {

    class DrawableMultiContainer {

        public:
            DrawableMultiContainer(vuw::MultiShader* shader)
                : shader_(shader) {}

            ~DrawableMultiContainer() {
                Global::vulkanWrapper->removeGraphicPipeline(&graphicsPipeline_);
            }

            /**
             * To initialize the graphic pipeline.
             * It must be called before doing any bind/draw
            */
            void initializePipeline(std::vector<uint32_t> const& vertexAttributesSizes, VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST) {
                graphicsPipeline_ = Global::vulkanWrapper->generateGraphicsPipeline(*shader_, vertexAttributesSizes, topology);
                Global::vulkanWrapper->saveGraphicPipeline(&graphicsPipeline_);
            }

            void bind(VkCommandBuffer commandBuffer, size_t objectIndex, uint32_t currentFrame) {
                graphicsPipeline_.bind(commandBuffer);
                shader_->bind(commandBuffer, objectIndex, currentFrame);
            }

        protected:
            //OpenGL
            vuw::MultiShader* shader_;
            vuw::GraphicsPipeline graphicsPipeline_;

            // VAO vao_;
            // size_t attributesSum_;
            // std::vector<VAO::Attribute> attributes_;

            // std::vector<float> vertices_;
            
            // std::vector<int> indices_;
            // size_t indicesSize_;

        };

}
