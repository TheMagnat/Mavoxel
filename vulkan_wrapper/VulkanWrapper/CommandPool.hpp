
#pragma once

#include <vulkan/vulkan.h>

#include <VulkanWrapper/Surface.hpp>
#include <VulkanWrapper/Device.hpp>

#include <stdexcept>


namespace vuw {

    class CommandPool {

        public:
            CommandPool(Surface const& surface, Device const& device) : devicePtr_(device.get()) {
                initializeCommandPool(surface, device);
            };

            ~CommandPool() {
                vkDestroyCommandPool(devicePtr_, commandPool_, nullptr);
            }

            CommandPool(CommandPool&&) = delete; //TODO: Declarer un move constructor
            CommandPool& operator=(CommandPool&&) = delete;

            CommandPool(const CommandPool&) = delete;
            CommandPool& operator=(const CommandPool&) = delete;

            void initializeCommandPool(Surface const& surface, Device const& device) {
                QueueFamily::QueueFamilyIndices queueFamilyIndices = QueueFamily::findQueueFamilies(device.getPhysical(), surface.get());

                VkCommandPoolCreateInfo poolInfo{};
                poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Allow te rewrite command buffer every frame
                poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(); //TODO: rendre paramétrable la graphic family

                if (vkCreateCommandPool(device.get(), &poolInfo, nullptr, &commandPool_) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create command pool !");
                }

            }

            VkCommandPool get() const {
                return commandPool_;
            }
            

        private:
            VkDevice devicePtr_;

            VkCommandPool commandPool_;


    };

}
