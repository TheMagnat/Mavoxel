
#pragma once

#include <vulkan/vulkan.h>

#include <VulkanWrapper/Helper/QueueFamily.hpp>
#include <VulkanWrapper/Helper/Checker.hpp>
#include <VulkanWrapper/Helper/SwapChainHelper.hpp>

#include <VulkanWrapper/Configuration.hpp>

#include <vector>


class PhysicalDevices {

    public:

        static VkPhysicalDevice getBestPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {

            std::vector<VkPhysicalDevice> devices = PhysicalDevices::getPhysicalDevices(instance);

            //If no device support vulkan, return no handle
            if (devices.empty()) return VK_NULL_HANDLE;

            VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
            int currentBestScore = 0;

            for (VkPhysicalDevice device : devices) {
                int score = rateDevice(device, surface);

                if (score > currentBestScore) {
                    bestDevice = device;
                    currentBestScore = score;
                }

            }

            return bestDevice;

        }


        //Note: Equivalent to isDeviceSuitable from the vulkan tutorial but return a score instead of a bool, a score of 0 is like a false bool
        //Attribute a score to a device
        static int rateDevice(VkPhysicalDevice device, VkSurfaceKHR surface) {
            
            //TODO: Better score calculation
            VkPhysicalDeviceProperties deviceProperties;
            VkPhysicalDeviceFeatures deviceFeatures;

            //Get the properties of the GPU device
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            
            //Get the special features of the GPU device
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

            int score = 0;

            // Discrete GPUs have a significant performance advantage
            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                score += 10000;
            }

            // Maximum possible size of textures affects graphics quality
            score += deviceProperties.limits.maxImageDimension2D;

            // Application can't function without geometry shaders
            if (!deviceFeatures.geometryShader) {
                return 0;
            }

            // We verify that the device have access to a graphic family queue.
            QueueFamily::QueueFamilyIndices indices = QueueFamily::findQueueFamilies(device, surface);
            if (!indices.isComplete()) {
                return 0;
            }

            bool extensionsSupported = Checker::deviceExtensionSupport(device);
            bool swapChainAdequate = false;
            if (extensionsSupported) {
                SwapChainHelper::SwapChainSupportDetails swapChainSupport = SwapChainHelper::querySwapChainSupport(device, surface);
                swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
            }
            
            //Verify the supported feature (here sampler anisotropy)
            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(device, &supportedFeatures);


            //If extensions are not supported or the swap chain support don't match our needs we return 0
            if (!extensionsSupported || !swapChainAdequate || !supportedFeatures.samplerAnisotropy) {
                return 0;
            }

            return score;

        }


        //Get all available GPU devices compatible with Vulkan
        static std::vector<VkPhysicalDevice> getPhysicalDevices(VkInstance instance) {

            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

            return devices;

        }

};
