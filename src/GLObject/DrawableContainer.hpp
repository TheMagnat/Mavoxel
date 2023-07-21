
#pragma once

#include <VulkanWrapper/GraphicsPipeline.hpp>
#include <VulkanWrapper/Shader.hpp>

#include <Core/Global.hpp>

#include <vector>

namespace mav {

    class DrawableContainer {

        public:
            DrawableContainer(vuw::Shader* shader)
                : shader_(shader) {}

            ~DrawableContainer() {
                Global::vulkanWrapper->removeGraphicPipeline(&graphicsPipeline_);
            }

            /**
             * To initialize the graphic pipeline.
             * It must be called before doing any bind/draw
            */
            void initializePipeline(std::vector<uint32_t> const& vertexAttributesSizes, bool filterPipeline = false, VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST) {
                graphicsPipeline_ = filterPipeline ? Global::vulkanWrapper->generateGraphicsPipelineForFilter(*shader_, vertexAttributesSizes, topology) : Global::vulkanWrapper->generateGraphicsPipeline(*shader_, vertexAttributesSizes, topology);
                Global::vulkanWrapper->saveGraphicPipeline(&graphicsPipeline_);
            }

            void initializePipelineAntialiasing(std::vector<uint32_t> const& vertexAttributesSizes, VkSampleCountFlagBits msaaSamples, VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST) {
                graphicsPipeline_ = Global::vulkanWrapper->generateGraphicsPipelineAntialiasing(*shader_, vertexAttributesSizes, msaaSamples, topology);
                Global::vulkanWrapper->saveGraphicPipeline(&graphicsPipeline_);
            }

            void bind(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
                graphicsPipeline_.bind(commandBuffer);
                shader_->bind(commandBuffer, currentFrame);
            }

        protected:
            //OpenGL
            vuw::Shader* shader_;
            vuw::GraphicsPipeline graphicsPipeline_;

            // VAO vao_;
            // size_t attributesSum_;
            // std::vector<VAO::Attribute> attributes_;

            // std::vector<float> vertices_;
            
            // std::vector<int> indices_;
            // size_t indicesSize_;

        };

}
