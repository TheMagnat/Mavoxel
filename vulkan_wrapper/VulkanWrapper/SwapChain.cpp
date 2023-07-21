
#include <VulkanWrapper/SwapChain.hpp>


namespace vuw {

    void SwapChain::initializeFramebuffers(RenderPass const& renderPass) {

        swapChainFramebuffers_.resize(swapChainImageViews_.size());

        std::vector<VkImageView> attachments(1 + depthCheck_ + antiAliasing_);
        if (depthCheck_) attachments[1] = depthImageView_;
        if (antiAliasing_) attachments[0] = colorImageView_;

        for (size_t i = 0; i < swapChainImageViews_.size(); i++) {
            
            if (antiAliasing_) attachments.back() = swapChainImageViews_[i];
            else attachments[0] = swapChainImageViews_[i];            

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass.get();
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent_.width;
            framebufferInfo.height = swapChainExtent_.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(devicePtr_, &framebufferInfo, nullptr, &swapChainFramebuffers_[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create framebuffer !");
            }
        }

    }

}
