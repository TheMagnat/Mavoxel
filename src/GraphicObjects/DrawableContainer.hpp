
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

            vuw::Shader* getShader() {
                return shader_;
            }

            /**
             * To initialize the graphic pipeline.
             * It must be called before doing any bind/draw
             * 
             * Note: renderIndex must be equal to the desired render in the global vulkanWrapper class.
             *       Less than 0 mean using the normal renderer, any other index mean using the corresponding filter render.
            */
            void initializePipeline(std::vector<uint32_t> const& vertexAttributesSizes, int renderIndex, VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST) {
                //TODO: meilleur solution pour récupérer la pipeline ?
                graphicsPipeline_ = Global::vulkanWrapper->generateGraphicsPipeline(*shader_, vertexAttributesSizes, renderIndex, topology);
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
