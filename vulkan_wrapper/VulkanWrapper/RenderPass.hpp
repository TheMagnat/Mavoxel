
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
            RenderPass(Device const& device, SwapChain const& swapChain, bool depthCheck = false) : devicePtr_(device.get()) {
                initializeRenderPass(swapChain, depthCheck);
            };

            RenderPass(Device const& device, bool depthCheck = false) : devicePtr_(device.get()) {
                initializeRenderPass(depthCheck);
            };

            RenderPass(Device const& device) : devicePtr_(device.get()) {}

            ~RenderPass() {
                clean();
            }

            void clean() {
                if (renderPass_) vkDestroyRenderPass(devicePtr_, renderPass_, nullptr);
            }

            RenderPass(RenderPass&&) = delete; //TODO: Declarer un move constructor
            RenderPass& operator=(RenderPass&&) = delete;

            RenderPass(const RenderPass&) = delete;
            RenderPass& operator=(const RenderPass&) = delete;

            void initializeRenderPass(SwapChain const& swapChain, bool depthCheck);
            void initializeRenderPass(bool depthCheck, VkFormat depthFormat = VK_FORMAT_D32_SFLOAT);

            VkRenderPass get() const {
                return renderPass_;
            }
            

        private:
            VkDevice devicePtr_;

            VkRenderPass renderPass_;


    };

}
