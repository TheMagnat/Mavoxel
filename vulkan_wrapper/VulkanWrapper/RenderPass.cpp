
#include <VulkanWrapper/RenderPass.hpp>


namespace vuw {

    void RenderPass::initializeRenderPass(SwapChain const& swapChain, bool depthCheck) {


        std::vector<VkAttachmentDescription> attachments(1 + swapChain.isDepthCheck() + swapChain.isAntiAliasing());
        
        // Color attachment description
        attachments[0].format = swapChain.getFormat();
        attachments[0].samples = swapChain.isAntiAliasing() ? swapChain.getAntiAliasingSample() : VK_SAMPLE_COUNT_1_BIT;

        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = swapChain.isAntiAliasing() ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // Subpasses
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        
        // Depth initialization to prevent scope destruction
        VkAttachmentReference depthAttachmentRef{};
        if (swapChain.isDepthCheck()) {
            // Depth attachment description
            attachments[1].format = swapChain.getDepthFormat();
            attachments[1].samples = swapChain.isAntiAliasing() ? swapChain.getAntiAliasingSample() : VK_SAMPLE_COUNT_1_BIT;

            attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            
            depthAttachmentRef.attachment = 1;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            //Add depth attachment ref to subpass
            subpass.pDepthStencilAttachment = &depthAttachmentRef;
        }

        // VkAttachmentDescription colorAttachmentResolve{};
        VkAttachmentReference colorAttachmentResolveRef{};
        if (swapChain.isAntiAliasing()) {

            VkAttachmentDescription& antiAliasingAttachment = attachments.back();

            antiAliasingAttachment.format = swapChain.getFormat();
            antiAliasingAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            antiAliasingAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            antiAliasingAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            antiAliasingAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            antiAliasingAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            antiAliasingAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            antiAliasingAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            colorAttachmentResolveRef.attachment = attachments.size() - 1;
            colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            subpass.pResolveAttachments = &colorAttachmentResolveRef;

        }



        // Dependency, to configure when to wait for acquisition (here we only start waiting the acquisition in the color writing)

        //TODO: voir si il faut l'array avec 2 dependency
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;

        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;



        // The render pass
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;


        if (vkCreateRenderPass(devicePtr_, &renderPassInfo, nullptr, &renderPass_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass !");
        }

    }


    //For output in texture
    void RenderPass::initializeRenderPass(bool depthCheck, VkFormat depthFormat) {

        std::vector<VkAttachmentDescription> attachments(1 + depthCheck);
        
        // Color attachment description
        attachments[0].format = VK_FORMAT_R8G8B8A8_UNORM;
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;

        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; //Here the end layout will be an input for another shader

        // Subpasses
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        
        // Depth initialization to prevent scope destruction
        VkAttachmentReference depthAttachmentRef{};
        if (depthCheck) {
            // Depth attachment description
            attachments[1].format = depthFormat;
            attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;

            attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            depthAttachmentRef.attachment = 1;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            //Add depth attachment ref to subpass
            subpass.pDepthStencilAttachment = &depthAttachmentRef;
        }

        // Dependency, to configure when to wait for acquisition (here we only start waiting the acquisition in the color writing)
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;

        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;

        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        // The render pass
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;


        if (vkCreateRenderPass(devicePtr_, &renderPassInfo, nullptr, &renderPass_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass !");
        }

    }

}
