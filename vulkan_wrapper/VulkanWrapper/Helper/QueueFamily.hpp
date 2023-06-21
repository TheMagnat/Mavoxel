
#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

class QueueFamily {

    public:

    	struct QueueFamilyIndices {
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> presentFamily;

            bool isComplete() {
                return graphicsFamily.has_value() && presentFamily.has_value();
            }
        };

        //This function require the device to retrieve the queue families from and a surface to verify the capability of the queues to present to the surface 
        static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {

            QueueFamilyIndices indices;
            
            // Logic to find queue family indices to populate struct with
            std::vector<VkQueueFamilyProperties> queueFamilies = QueueFamily::getQueueFamilies(device);

            for (int i = 0; i < queueFamilies.size(); ++i) {
                
                if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphicsFamily = i;
                }

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

                if (presentSupport) {
                    indices.presentFamily = i;
                }


                if (indices.isComplete()) {
                    break;
                }

            }
            
            return indices;
        }

        //Get all available queues in the device
        static std::vector<VkQueueFamilyProperties> getQueueFamilies(VkPhysicalDevice device) {

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            return queueFamilies;

        }

};
