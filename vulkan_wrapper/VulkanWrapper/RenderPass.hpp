
#pragma once

#include <vulkan/vulkan.h>

#include <VulkanWrapper/Device.hpp>
#include <VulkanWrapper/SwapChain.hpp>

#include <stdexcept>


namespace vuw {

    //To correct circular include
    class SwapChain;

    class RenderPass {

        public:
            RenderPass(Device const& device, SwapChain const& swapChain, bool depthCheck = false) : devicePtr_(device.get()) {
                initializeRenderPass(swapChain, depthCheck);
            };

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

            VkRenderPass get() const {
                return renderPass_;
            }
            

        private:
            VkDevice devicePtr_;

            VkRenderPass renderPass_;


    };

}
