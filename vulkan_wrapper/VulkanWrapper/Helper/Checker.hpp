
#pragma once

#include <vulkan/vulkan.h>

#include <VulkanWrapper/Configuration.hpp>

#include <vector>
#include <set>

class Checker {

    public:

        static bool validationLayerSupport() {
            
            //Get the size
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            //Initialize a vector with the size to fill it
            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
            
            //Verify if the validation layers are in the available layers
            for (const char* layerName : validationLayers) {
                bool layerFound = false;

                for (const auto& layerProperties : availableLayers) {
                    if (strcmp(layerName, layerProperties.layerName) == 0) {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound) {
                    return false;
                }
            }

            return true;
        }

        static bool verifyAvailability(std::vector<const char*> const& required, std::vector<const char*> const& available) {

            auto lam = [](const char* a, const char* b){return std::strcmp(a, b) < 0;};
            std::set<const char*, decltype(lam)> availableSet(available.begin(), available.end());

            for (const char* req : required )
                if (!availableSet.count(req)) return false;

            return true;

        }

        static bool requiredExtensionAreAvailable(std::vector<const char*> const& required, std::vector<VkExtensionProperties> const& available) {
            
            std::vector<const char*> availableStringVector;
            for ( VkExtensionProperties const& extension : available )
                availableStringVector.emplace_back(extension.extensionName);

            return verifyAvailability(required, availableStringVector);

        }

        static bool deviceExtensionSupport(VkPhysicalDevice physicalDevice) {
            
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

            std::vector<const char*> availableStringVector;
            for ( VkExtensionProperties const& extension : availableExtensions )
                availableStringVector.emplace_back(extension.extensionName);

            return verifyAvailability(deviceExtensions, availableStringVector);

        }

};
