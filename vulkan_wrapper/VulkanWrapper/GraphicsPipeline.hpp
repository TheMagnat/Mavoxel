
#pragma once

#include <vulkan/vulkan.h>

#include <VulkanWrapper/Device.hpp>
#include <VulkanWrapper/SwapChain.hpp>
#include <VulkanWrapper/RenderPass.hpp>
#include <VulkanWrapper/VertexData.hpp>
#include <VulkanWrapper/Shader.hpp>
#include <VulkanWrapper/Helper/ShaderHelper.hpp>


namespace vuw {

    class GraphicsPipeline {

        public:

            GraphicsPipeline() {};

            GraphicsPipeline(Device const& device, vuw::Shader const& shader, VkExtent2D const& swapChainExtent, RenderPass const& renderPass, std::vector<uint32_t> const& vertexAttributesSize, bool depthCheck = false, VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                : devicePtr_(device.get()), shaderPtr_(&shader), vertexAttributesSize_(vertexAttributesSize), depthCheck_(depthCheck), topology_(topology) {
                initialize(swapChainExtent, renderPass);
            }

            ~GraphicsPipeline() {
                clean();
            }

            void clean() {
                if (graphicsPipeline_) vkDestroyPipeline(devicePtr_, graphicsPipeline_, nullptr);
            }

            GraphicsPipeline(GraphicsPipeline&& movedPipeline) : devicePtr_(std::move(movedPipeline.devicePtr_)), graphicsPipeline_(std::move(movedPipeline.graphicsPipeline_)) {
                movedPipeline.graphicsPipeline_ = nullptr;
            }

            GraphicsPipeline& operator=(GraphicsPipeline&& movedPipeline) {
                
                if (graphicsPipeline_) vkDestroyPipeline(devicePtr_, graphicsPipeline_, nullptr);

                devicePtr_ = std::move(movedPipeline.devicePtr_);
                
                shaderPtr_ = std::move(movedPipeline.shaderPtr_);
                vertexAttributesSize_ = std::move(movedPipeline.vertexAttributesSize_);

                depthCheck_ = std::move(movedPipeline.depthCheck_);
                topology_ = std::move(movedPipeline.topology_);

                //Owned Vk Object
                graphicsPipeline_ = std::move(movedPipeline.graphicsPipeline_);
                movedPipeline.graphicsPipeline_ = nullptr;

                return *this;
            }

            GraphicsPipeline(const GraphicsPipeline&) = delete;
            GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

            void initialize(VkExtent2D const& swapChainExtent, RenderPass const& renderPass) {
                
                std::vector<char> vertShaderCode = ShaderHelper::readFile(shaderPtr_->getVertexFilename());
                std::vector<char> fragShaderCode = ShaderHelper::readFile(shaderPtr_->getFragmentFilename());

                VkShaderModule vertShaderModule = ShaderHelper::createShaderModule(devicePtr_, vertShaderCode);
                VkShaderModule fragShaderModule = ShaderHelper::createShaderModule(devicePtr_, fragShaderCode);

                // Vertex hader
                VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
                vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

                vertShaderStageInfo.module = vertShaderModule;
                vertShaderStageInfo.pName = "main";
                vertShaderStageInfo.pSpecializationInfo = nullptr;

                // Fragment Shader
                VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
                fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

                fragShaderStageInfo.module = fragShaderModule;
                fragShaderStageInfo.pName = "main";
                fragShaderStageInfo.pSpecializationInfo = nullptr;

                // Dynamic shaders array
                VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

                //// Static part

                //Vertex Buffer part
                auto [bindingDescription, attributeDescriptions] = VertexData::getDescriptions(vertexAttributesSize_);
                //std::vector<VkVertexInputAttributeDescription> const& attributeDescriptions = vertexData_.getAttributeDescriptions();


                /// Describe the structure of the data vertices (Leur type, le décalage entre donnée etc)
                VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
                vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

                vertexInputInfo.vertexBindingDescriptionCount = 1;
                vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional

                vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
                vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional
                
                /// Describe how to link vertices together (We can change to line here for example)
                VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
                inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                inputAssembly.topology = topology_;
                inputAssembly.primitiveRestartEnable = VK_FALSE;

                /// Viewport. It describe where we should draw on the frameBuffer
                VkViewport viewport{};
                viewport.x = 0.0f;
                viewport.y = 0.0f;
                viewport.width = (float) swapChainExtent.width;
                viewport.height = (float) swapChainExtent.height;
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;

                // Mask filter, only the pixels within will be drawn
                VkRect2D scissor{};
                scissor.offset = {0, 0};
                scissor.extent = swapChainExtent;

                // Merging viewport and scissor to create one unique viewport
                VkPipelineViewportStateCreateInfo viewportState{};
                viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                viewportState.viewportCount = 1;
                viewportState.pViewports = &viewport;
                viewportState.scissorCount = 1;
                viewportState.pScissors = &scissor;

                /// Rasterizer
                VkPipelineRasterizationStateCreateInfo rasterizer{};
                rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                rasterizer.depthClampEnable = VK_FALSE; //If true, geometries that are not in between the far and near planes will be clamped
                rasterizer.rasterizerDiscardEnable = VK_FALSE; //If true, every geometry will be discarded

                /**
                 * VK_POLYGON_MODE_FILL : Fill the polygons with fragments
                 * VK_POLYGON_MODE_LINE : Only draw lines
                 * VK_POLYGON_MODE_POINT : Only draw points
                 */
                rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

                rasterizer.lineWidth = 1.0f; //Define the thickness of the lines. If not 1.0, the extension "wideLines" should be activated
    
                //rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
                rasterizer.cullMode = VK_CULL_MODE_NONE;
                rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //Clockwise or not to evaluate front face
                // rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; //Clockwise or not to evaluate front face

                //Parameters to alter the depth
                rasterizer.depthBiasEnable = VK_FALSE;
                rasterizer.depthBiasConstantFactor = 0.0f; // Optional
                rasterizer.depthBiasClamp = 0.0f; // Optional
                rasterizer.depthBiasSlopeFactor = 0.0f; // Optional


                /// Multi-sampling (It allow anti-aliasing)

                // Deactivated :
                VkPipelineMultisampleStateCreateInfo multisampling{};
                multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                multisampling.sampleShadingEnable = VK_FALSE;
                multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
                multisampling.minSampleShading = 1.0f; // Optional
                multisampling.pSampleMask = nullptr; // Optional
                multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
                multisampling.alphaToOneEnable = VK_FALSE; // Optional

                /// Color blending (Configure how we combine colors that are already present in the framebuffer)

                // One per framebuffer
                // Deactivated :
                VkPipelineColorBlendAttachmentState colorBlendAttachment{};
                colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                colorBlendAttachment.blendEnable = VK_FALSE;
                colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
                colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
                colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
                colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
                colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
                colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

                // Global color blend settings
                VkPipelineColorBlendStateCreateInfo colorBlending{};
                colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                colorBlending.logicOpEnable = VK_FALSE;
                colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional

                colorBlending.attachmentCount = 1;
                colorBlending.pAttachments = &colorBlendAttachment;

                colorBlending.blendConstants[0] = 0.0f; // Optional
                colorBlending.blendConstants[1] = 0.0f; // Optional
                colorBlending.blendConstants[2] = 0.0f; // Optional
                colorBlending.blendConstants[3] = 0.0f; // Optional

                std::vector<VkDynamicState> dynamicStates = {
                    VK_DYNAMIC_STATE_VIEWPORT
                };

                VkPipelineDepthStencilStateCreateInfo depthStencil{};
                if (depthCheck_) {
                    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                    depthStencil.depthTestEnable = VK_TRUE;
                    depthStencil.depthWriteEnable = VK_TRUE;
                
                    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

                    depthStencil.depthBoundsTestEnable = VK_FALSE;
                    // depthStencil.minDepthBounds = 0.0f; // Optionnel
                    // depthStencil.maxDepthBounds = 1.0f; // Optionnel

                    depthStencil.stencilTestEnable = VK_FALSE;
                    // depthStencil.front = {}; // Optionnel
                    // depthStencil.back = {}; // Optionnel

                }

                VkPipelineDynamicStateCreateInfo dynamicState{};
                dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
                dynamicState.pDynamicStates = dynamicStates.data();


                /// Pipeline Layout (This will indicates our need in uniforms)
                // VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
                // pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                // pipelineLayoutInfo.setLayoutCount = 1;
                // pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
                // pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
                // pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

                // if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
                // 	throw std::runtime_error("Failed to create the pipeline layout !");
                // }


                VkGraphicsPipelineCreateInfo pipelineInfo{};
                pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                pipelineInfo.stageCount = 2;
                pipelineInfo.pStages = shaderStages;


                pipelineInfo.pVertexInputState = &vertexInputInfo;
                pipelineInfo.pInputAssemblyState = &inputAssembly;
                pipelineInfo.pViewportState = &viewportState;
                pipelineInfo.pRasterizationState = &rasterizer;
                pipelineInfo.pMultisampleState = &multisampling;
                
                pipelineInfo.pDepthStencilState = depthCheck_ ? &depthStencil : nullptr;
                
                pipelineInfo.pColorBlendState = &colorBlending;
                pipelineInfo.pDynamicState = &dynamicState; // Optional

                pipelineInfo.layout = shaderPtr_->getPipelineLayout();

                pipelineInfo.renderPass = renderPass.get();
                pipelineInfo.subpass = 0;

                pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
                pipelineInfo.basePipelineIndex = -1; // Optional

                if (vkCreateGraphicsPipelines(devicePtr_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline_) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create the graphic pipeline !");
                }

                //Now delete the shader modules since we don't need them anymore
                vkDestroyShaderModule(devicePtr_, fragShaderModule, nullptr);
                vkDestroyShaderModule(devicePtr_, vertShaderModule, nullptr);

            }

            void bind(VkCommandBuffer commandBuffer) const {
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_);
            }

            VkPipeline get() const {
                return graphicsPipeline_;
            }

        private:
            VkDevice devicePtr_;

            //Parameter saved
            const vuw::Shader* shaderPtr_;
            std::vector<uint32_t> vertexAttributesSize_;
            
            bool depthCheck_;
            VkPrimitiveTopology topology_;

            VkPipeline graphicsPipeline_ = VK_NULL_HANDLE;

    };

}
