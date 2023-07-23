
#pragma once

#include <vulkan/vulkan.h>

#include <VulkanWrapper/Device.hpp>
#include <VulkanWrapper/RenderPass.hpp>
#include <VulkanWrapper/Texture.hpp>

#include <vector>


namespace vuw {


    class RenderPass;

    class Framebuffer {


        public:
            Framebuffer(Device const& device, RenderPass const& renderPass, const Texture* imageTexture, const Texture* depthTexture = nullptr) : devicePtr_(device.get()) {
                initialize(renderPass, imageTexture, depthTexture);
            }

            Framebuffer(Device const& device, RenderPass const& renderPass, std::vector<const Texture*> const& colorTextures, const Texture* depthTexture = nullptr) : devicePtr_(device.get()) {
                initialize(renderPass, colorTextures, depthTexture);
            }

            Framebuffer(Framebuffer&& movedFramebuffer) :
                devicePtr_(std::move(movedFramebuffer.devicePtr_)),
                framebuffer_(std::move(movedFramebuffer.framebuffer_)),
                depthCheck_(std::move(movedFramebuffer.depthCheck_))
            {
                movedFramebuffer.framebuffer_ = nullptr;
            }

            Framebuffer& operator=(Framebuffer&& movedTexture) = delete;

            Framebuffer(const Framebuffer&) = delete;
            Framebuffer& operator=(const Framebuffer&) = delete;

            ~Framebuffer() {

                if (framebuffer_) {
                    vkDestroyFramebuffer(devicePtr_, framebuffer_, nullptr);
                    framebuffer_ = nullptr;
                }

            }

            void initialize(RenderPass const& renderPass, const Texture* imageTexture, const Texture* depthTexture = nullptr) {
                
                //To get the size of the texture
                Texture::TextureInformations const& textureInformations = imageTexture->getInformations();
                
                //If depth texture is present, create two attachments...
                std::vector<VkImageView> attachments(1 + (depthTexture != nullptr));

                //...and fill it
                if (depthTexture) attachments[1] = depthTexture->getImageView();

                attachments[0] = imageTexture->getImageView();

                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = renderPass.get();
                framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                framebufferInfo.pAttachments = attachments.data();
                framebufferInfo.width = textureInformations.width;
                framebufferInfo.height = textureInformations.height;
                framebufferInfo.layers = 1;

                if (vkCreateFramebuffer(devicePtr_, &framebufferInfo, nullptr, &framebuffer_) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create framebuffer !");
                }

            }

            //Here we assume that all color textures have the same informations
            void initialize(RenderPass const& renderPass, std::vector<const Texture*> const& colorTextures, const Texture* depthTexture = nullptr) {
                
                //To get the size of the texture
                Texture::TextureInformations const& textureInformations = colorTextures.front()->getInformations();
                
                //If depth texture is present, create one more attachment...
                std::vector<VkImageView> attachments(colorTextures.size() + (depthTexture != nullptr));

                //...and fill it
                if (depthTexture) attachments.back() = depthTexture->getImageView();

                for (size_t i = 0; i < colorTextures.size(); ++i) {
                    attachments[i] = colorTextures[i]->getImageView();
                }

                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = renderPass.get();
                framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                framebufferInfo.pAttachments = attachments.data();
                framebufferInfo.width = textureInformations.width;
                framebufferInfo.height = textureInformations.height;
                framebufferInfo.layers = 1;

                if (vkCreateFramebuffer(devicePtr_, &framebufferInfo, nullptr, &framebuffer_) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create framebuffer !");
                }

            }

            VkFramebuffer get() const {
                return framebuffer_;
            }


        private:
            VkDevice devicePtr_;

            VkFramebuffer framebuffer_ = nullptr;

            bool depthCheck_;

    };

}
