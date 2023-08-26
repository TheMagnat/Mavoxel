
#pragma once

#include <vulkan/vulkan.h>

#include <VulkanWrapper/Device.hpp>
#include <VulkanWrapper/SwapChain.hpp>
#include <VulkanWrapper/Texture.hpp>

#include <stdexcept>


namespace vuw {

    //To correct circular include
    class SwapChain;

    class RenderPass {

        public:
            RenderPass(Device const& device, SwapChain const& swapChain, bool depthCheck = false) : devicePtr_(device.get()), nbColors_(1) {
                initializeRenderPass(swapChain, depthCheck);
            };

            RenderPass(Device const& device, bool depthCheck = false) : devicePtr_(device.get()), nbColors_(1) {
                initializeRenderPass(depthCheck);
            };

            RenderPass(Device const& device, size_t nbColors, bool depthCheck = false) : devicePtr_(device.get()), nbColors_(nbColors) {
                initializeRenderPass(depthCheck);
            };

            RenderPass(Device const& device) : devicePtr_(device.get()) {}

            ~RenderPass() {
                clean();
            }

            void clean() {
                if (renderPass_) vkDestroyRenderPass(devicePtr_, renderPass_, nullptr);
            }

            RenderPass(RenderPass&& toMove)
                : devicePtr_(toMove.devicePtr_), renderPass_(toMove.renderPass_), nbColors_(toMove.nbColors_)
            {
                toMove.devicePtr_ = nullptr;
                toMove.renderPass_ = nullptr;
                toMove.nbColors_ = 0;
            }

            RenderPass& operator=(RenderPass&& toMove) {
                
                devicePtr_ = toMove.devicePtr_;
                renderPass_ = toMove.renderPass_;
                nbColors_ = toMove.nbColors_;

                toMove.devicePtr_ = nullptr;
                toMove.renderPass_ = nullptr;
                toMove.nbColors_ = 0;
            };

            RenderPass(const RenderPass&) = delete;
            RenderPass& operator=(const RenderPass&) = delete;

            void initializeRenderPass(SwapChain const& swapChain, bool depthCheck);
            void initializeRenderPass(bool depthCheck, VkFormat depthFormat = VK_FORMAT_D32_SFLOAT);

            VkRenderPass get() const {
                return renderPass_;
            }

            size_t getNbColors() const {
                return nbColors_;
            }
            

        private:
            VkDevice devicePtr_;

            VkRenderPass renderPass_;

            size_t nbColors_;

    };

}
