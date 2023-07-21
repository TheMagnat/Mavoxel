
#pragma once

#include <vulkan/vulkan.h>

#include <VulkanWrapper/Surface.hpp>
#include <VulkanWrapper/Device.hpp>
#include <VulkanWrapper/RenderPass.hpp>

#include <VulkanWrapper/Helper/SwapChainHelper.hpp>
#include <VulkanWrapper/Helper/Image.hpp>
#include <VulkanWrapper/Helper/Getter.hpp>
#include <VulkanWrapper/Helper/Buffer.hpp>


#include <stdexcept>


namespace vuw {

    //To correct circular include
    class RenderPass;

    class SwapChain {

        public:

            //Without anti-aliasing
            SwapChain(GLFWwindow* window, Surface const& surface, Device const& device, bool depthCheck = false) : devicePtr_(device.get()), allocatorPtr_(device.getAllocator()), depthCheck_(depthCheck), antiAliasing_(false) {
                initializeSwapChain(window, surface, device);
                initializeImageViews();
                if (depthCheck_) initializeDepthResources(device);
            };

            //With anti-aliasing
            SwapChain(GLFWwindow* window, Surface const& surface, Device const& device, VkSampleCountFlagBits msaaSamples, bool depthCheck = false) : devicePtr_(device.get()), allocatorPtr_(device.getAllocator()), depthCheck_(depthCheck), antiAliasing_(true), msaaSamples_(msaaSamples) {                
                initializeSwapChain(window, surface, device);
                initializeImageViews();
                if (antiAliasing_) initializeColorResources(device);
                if (depthCheck_) initializeDepthResources(device);
            };

            ~SwapChain() {
                clean();
            }
            
            SwapChain(SwapChain&&) = delete; //TODO: Declarer un move constructor
            SwapChain& operator=(SwapChain&&) = delete;

            SwapChain(const SwapChain&) = delete;
            SwapChain& operator=(const SwapChain&) = delete;


            void clean() {
                for (size_t i = 0; i < swapChainFramebuffers_.size(); i++) {
                    vkDestroyFramebuffer(devicePtr_, swapChainFramebuffers_[i], nullptr);
                }

                for (size_t i = 0; i < swapChainImageViews_.size(); i++) {
                    vkDestroyImageView(devicePtr_, swapChainImageViews_[i], nullptr);
                }

                vkDestroySwapchainKHR(devicePtr_, swapChain_, nullptr);

                if (antiAliasing_) {
                    vkDestroyImageView(devicePtr_, colorImageView_, nullptr);
                    vmaDestroyImage(allocatorPtr_, colorImage_, colorImageAllocation_);
                }

                if (depthCheck_) {
                    vkDestroyImageView(devicePtr_, depthImageView_, nullptr);
                    vmaDestroyImage(allocatorPtr_, depthImage_, depthImageAllocation_);
                }

            }

            void initializeSwapChain(GLFWwindow* window, Surface const& surface, Device const& device) {

                SwapChainHelper::SwapChainSupportDetails swapChainSupport = SwapChainHelper::querySwapChainSupport(device.getPhysical(), surface.get());

                VkSurfaceFormatKHR surfaceFormat = SwapChainHelper::chooseSwapSurfaceFormat(swapChainSupport.formats);
                VkPresentModeKHR presentMode = SwapChainHelper::chooseSwapPresentMode(swapChainSupport.presentModes);
                //To force no v-sync
                // VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

                VkExtent2D extent = SwapChainHelper::chooseSwapExtent(swapChainSupport.capabilities, window);

                //TODO: voir ce qu'il se passe si on change le imageCount

                // Selection the number of images that will contain our swap chain.
                uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
                if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
                    imageCount = swapChainSupport.capabilities.maxImageCount;
                }

                // Fill the information structure to create the swap chain
                VkSwapchainCreateInfoKHR createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
                createInfo.surface = surface.get(); //We specify the surface on which we will render

                createInfo.minImageCount = imageCount;
                createInfo.imageFormat = surfaceFormat.format;
                createInfo.imageColorSpace = surfaceFormat.colorSpace;
                createInfo.imageExtent = extent;
                createInfo.imageArrayLayers = 1; //This is always 1 except for application that require multiple images on one render like stereoscopic 3D
                createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //This mean we will directly render to the image, but we could also render on a separate image first to do post-processing, in this case VK_IMAGE_USAGE_TRANSFER_DST_BIT could be used


                // Specify how to handle swap chain images that are getting used from multiple queues families
                QueueFamily::QueueFamilyIndices indices = QueueFamily::findQueueFamilies(device.getPhysical(), surface.get());
                uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
                
                if (indices.graphicsFamily != indices.presentFamily) {
                    //If we're using two different queue, we will have to use concurrent mode
                    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                    createInfo.queueFamilyIndexCount = 2;
                    createInfo.pQueueFamilyIndices = queueFamilyIndices;
                } else {
                    //If they are both the same queue, no need to use concurrent mode
                    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                    createInfo.queueFamilyIndexCount = 0; // Optional
                    createInfo.pQueueFamilyIndices = nullptr; // Optional
                }

                // Transformation applied to the images, capabilities.currentTransform mean no transformation applied
                createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

                createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

                createInfo.presentMode = presentMode;
                createInfo.clipped = VK_TRUE; //If we want to ignore pixels that are obscured by other windows for example. True give the best performances

                //If the swap chain get recreated, we want to store here a pointer to the old one
                createInfo.oldSwapchain = VK_NULL_HANDLE;

                // GENERATE the swap chain
                if (vkCreateSwapchainKHR(devicePtr_, &createInfo, nullptr, &swapChain_) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create swap chain !");
                }

                // Retrieve the images handles
                vkGetSwapchainImagesKHR(devicePtr_, swapChain_, &imageCount, nullptr);

                swapChainImages_.resize(imageCount);
                vkGetSwapchainImagesKHR(devicePtr_, swapChain_, &imageCount, swapChainImages_.data());

                // Store the formant and the extent
                swapChainImageFormat_ = surfaceFormat.format;
                swapChainExtent_ = extent;

            }
            
            void initializeImageViews() {

                swapChainImageViews_.resize(swapChainImages_.size());

                for (size_t i = 0; i < swapChainImages_.size(); i++) {
                    swapChainImageViews_[i] = Image::createImageView(devicePtr_, swapChainImages_[i], swapChainImageFormat_, VK_IMAGE_ASPECT_COLOR_BIT);
                }

            }

            void initializeColorResources(Device const& device) {

                Buffer::createImage(device.getAllocator(), swapChainExtent_.width, swapChainExtent_.height, swapChainImageFormat_, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, colorImage_, colorImageAllocation_, msaaSamples_);
                
                colorImageView_ = Image::createImageView(devicePtr_, colorImage_, swapChainImageFormat_, VK_IMAGE_ASPECT_COLOR_BIT);

            }

            void initializeDepthResources(Device const& device) {

                depthFormat_ = Getter::findDepthFormat(device.getPhysical());

                Buffer::createImage(device.getAllocator(), swapChainExtent_.width, swapChainExtent_.height, depthFormat_, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, depthImage_, depthImageAllocation_);

                depthImageView_ = Image::createImageView(devicePtr_, depthImage_, depthFormat_, VK_IMAGE_ASPECT_DEPTH_BIT);

            }

            //Note: Must be initialized after the RenderPass, but RenderPass require SwapChain to be initialized
            void initializeFramebuffers(RenderPass const& renderPass);

            //Depth related
            VkFormat getDepthFormat() const {
                return depthFormat_;
            }

            //Framebuffers
            std::vector<VkFramebuffer> const& getFramebuffers() {
                return swapChainFramebuffers_;
            }

            //Format and extent
            VkFormat getFormat() const {
                return swapChainImageFormat_;
            }

            VkExtent2D const& getExtent() const {
                return swapChainExtent_;
            }

            //Swapchain
            VkSwapchainKHR get() const {
                return swapChain_;
            }

            VkSampleCountFlagBits getAntiAliasingSample() const {
                return msaaSamples_;
            }

            bool isDepthCheck() const {
                return depthCheck_;
            }
            
            bool isAntiAliasing() const {
                return antiAliasing_;
            }

        private:

            //Save
            VkDevice devicePtr_;
            VmaAllocator allocatorPtr_;

            //
            VkSwapchainKHR swapChain_;
            std::vector<VkImage> swapChainImages_;
            std::vector<VkImageView> swapChainImageViews_;
            std::vector<VkFramebuffer> swapChainFramebuffers_;

            //Store the formant and the extent
            VkFormat swapChainImageFormat_;
            VkExtent2D swapChainExtent_;

            //Depth and stencil
            VkFormat depthFormat_;
            VkImage depthImage_;
            VmaAllocation depthImageAllocation_;
            VkImageView depthImageView_;

            //Antialiasing
            VkSampleCountFlagBits msaaSamples_;
            VkImage colorImage_;
            VmaAllocation colorImageAllocation_;
            VkImageView colorImageView_;

            //Variable
            bool depthCheck_;
            bool antiAliasing_;

    };

}
