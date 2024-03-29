
#pragma once

#include <vulkan/vulkan.h>

#include <VulkanWrapper/RenderPass.hpp>
#include <VulkanWrapper/FrameBuffer.hpp>
#include <VulkanWrapper/SwapChain.hpp>
#include <VulkanWrapper/Texture.hpp>

#include <glm/vec2.hpp>

#include <vector>

namespace {


    VkImageUsageFlags textureUsageToFlag(int usage) {

        if (usage == 1) {
            return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        return 0;

    }


}

namespace vuw {

    class SceneRenderer {

        public:
            
            SceneRenderer(Device const& device, VkCommandPool commandPool, uint16_t imageCount, glm::uvec2 imageResolution, int nbAdditionalImages = 0, bool depthCheck = false, int textureUsage = 0)
                : imageCount_(imageCount), depthCheck_(depthCheck), textureUsage_(textureUsage), renderPass_(device, 1 + nbAdditionalImages, depthCheck), additionalOutputImages_(nbAdditionalImages) {
                
                //TODO: Set ailleur
                sceneTextureInformations_ = Texture::TextureInformations{(uint32_t)imageResolution.x, (uint32_t)imageResolution.y, 1, VK_SHADER_STAGE_FRAGMENT_BIT};
                additionalOutputTextureInformations_ = Texture::TextureInformations{(uint32_t)imageResolution.x, (uint32_t)imageResolution.y, 1, VK_SHADER_STAGE_FRAGMENT_BIT};
                extent_ = VkExtent2D{sceneTextureInformations_.width, sceneTextureInformations_.height};


                for (uint16_t i = 0; i < imageCount_; ++i) {
                    images_.emplace_back(&device, commandPool, device.getGraphicsQueue(), sceneTextureInformations_, VK_FORMAT_R8G8B8A8_UNORM, textureUsageToFlag(textureUsage_));

                    //Create additional textures
                    for (size_t j = 0; j < nbAdditionalImages; ++j) {
                        additionalOutputImages_[j].emplace_back(&device, commandPool, device.getGraphicsQueue(), additionalOutputTextureInformations_, VK_FORMAT_R32G32B32A32_SFLOAT, textureUsageToFlag(textureUsage_));
                    }

                    //Create textures ptr vector for the framebuffer
                    std::vector<const Texture*> tempoTexturesPtr {&images_.back()};
                    for (std::vector<Texture> const& textures : additionalOutputImages_) {
                        tempoTexturesPtr.emplace_back(&textures.back());
                    }

                    if (depthCheck_) {
                        //TODO: voir si il faut pas d'autre type de texture pour la depth
                        depthTextures_.emplace_back(&device, commandPool, device.getGraphicsQueue(), sceneTextureInformations_);
                        framebuffers_.emplace_back(device, renderPass_, tempoTexturesPtr, &depthTextures_.back());
                        
                    }
                    else {
                        framebuffers_.emplace_back(device, renderPass_, tempoTexturesPtr);
                    }

                }

            }

            void beginRecordingCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentDrawingTargetImageIndex_) const {
                    
                    // VkCommandBufferBeginInfo beginInfo{};
                    // beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                    // beginInfo.flags = 0; // Optional
                    // beginInfo.pInheritanceInfo = nullptr; // Optional

                    // if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
                    //     throw std::runtime_error("failed to begin recording command buffer!");
                    // }

                    VkRenderPassBeginInfo renderPassInfo{};
                    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                    renderPassInfo.renderPass = renderPass_.get();
                    renderPassInfo.framebuffer = framebuffers_[currentDrawingTargetImageIndex_].get();

                    renderPassInfo.renderArea.offset = {0, 0};
                    renderPassInfo.renderArea.extent = {sceneTextureInformations_.width, sceneTextureInformations_.height};

                    //TODO: rendre ça paramétrable
                    std::vector<VkClearValue> clearValues(1 + additionalOutputImages_.size(), VkClearValue({{0.0f, 0.0f, 0.0f, 1.0f}}));
                    // clearValues[0] = {{0.0f, 0.0f, 0.0f, 1.0f}};
                    // clearValues[1] = {{0.0f, 0.0f, 0.0f, 1.0f}};
                    if (depthCheck_) clearValues.push_back( VkClearValue{1.0f, 0} );

                    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
                    renderPassInfo.pClearValues = clearValues.data();

                    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                    //ONLY if dynamic viewport and scissor activated during fixed pipeline's function specification
                    VkViewport viewport{};
                    viewport.x = 0.0f;
                    viewport.y = 0.0f;
                    viewport.width = static_cast<float>(sceneTextureInformations_.width);
                    viewport.height = static_cast<float>(sceneTextureInformations_.height);
                    viewport.minDepth = 0.0f;
                    viewport.maxDepth = 1.0f;
                    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
                    
                    // VkRect2D scissor{};
                    // scissor.offset = {0, 0};
                    // scissor.extent = swapChainExtent;
                    // vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

                }

            void endRecordingCommandBuffer(VkCommandBuffer commandBuffer) const {
                vkCmdEndRenderPass(commandBuffer);

                // if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
                //     throw std::runtime_error("failed to record command buffer !");
                // }
            }


            std::vector<Texture> const& getTextures() const {
                return images_;
            }

            std::vector<std::vector<Texture>> const& getAdditionalTextures() const {
                return additionalOutputImages_;
            }

            RenderPass const& getRenderpass() const {
                return renderPass_;
            }

            Texture::TextureInformations const& getTextureInformations() const {
                return sceneTextureInformations_;
            }

            VkExtent2D const& getExtent() const {
                return extent_;
            }

        private:
            uint16_t imageCount_;
            bool depthCheck_;
            int textureUsage_;

            Texture::TextureInformations sceneTextureInformations_;
            Texture::TextureInformations additionalOutputTextureInformations_;
            VkExtent2D extent_;

            RenderPass renderPass_;
            
            std::vector<Texture> images_;
            std::vector<std::vector<Texture>> additionalOutputImages_;
            std::vector<Texture> depthTextures_;

            std::vector<Framebuffer> framebuffers_;
            
    };

}
