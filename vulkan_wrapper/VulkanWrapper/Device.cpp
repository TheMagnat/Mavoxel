
#pragma once

#include <VulkanWrapper/Device.hpp>

#include <VulkanWrapper/Helper/Checker.hpp>


namespace vuw {
        
    Device::Device(Instance const& instance, Surface const& surface, bool enableValidationLayers) {
        pickPhysicalDevice(instance.get(), surface.get());
        createLogicalDevice(surface.get(), enableValidationLayers);
        allocator_.initializeAllocator(instance, *this);
    };

    Device::~Device() {
        allocator_.~Allocator();
        vkDestroyDevice(device_, nullptr);
    }

    void Device::pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {

        physicalDevice_ = PhysicalDevices::getBestPhysicalDevice(instance, surface);

        if (physicalDevice_ == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU !");
        }
        
    }

    void Device::createLogicalDevice(VkSurfaceKHR surface, bool enableValidationLayers) {
        QueueFamily::QueueFamilyIndices indices = QueueFamily::findQueueFamilies(physicalDevice_, surface);

        //The requested queue families
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;

            //Priority of the queue, from 0.0 (low) to 1.0 (Max)
            queueCreateInfo.pQueuePriorities = &queuePriority;

            //Add the queue informations to the vector of queue informations
            queueCreateInfos.push_back(queueCreateInfo);
        }

        
        //Specify the special features of the device we want to use in the queue (can be empty)
        // VkPhysicalDeviceFeatures deviceFeatures{};
        // deviceFeatures.samplerAnisotropy = VK_TRUE;
        // NOW USING VkPhysicalDeviceFeatures2

        //Logical device informations
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = VK_NULL_HANDLE;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        

        //This may be useless, validations layers are now useless in device since they use now the same as the instance validation layer
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        //VK_EXT_robustness2 extension : This will allow use to use VK_NULL_HANDLE ass buffer for our SSBOs
        VkPhysicalDeviceRobustness2FeaturesEXT robustness2Features{};
        robustness2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
        
        // robustness2Features.robustBufferAccess2 = VK_FALSE; // Optional
        // robustness2Features.robustImageAccess2 = VK_FALSE; // Optional
        robustness2Features.nullDescriptor = VK_TRUE; //Here only nullDescriptor interests us

        VkPhysicalDeviceVulkan12Features deviceFeaturesVulkan12{};
        deviceFeaturesVulkan12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

        VkPhysicalDeviceFeatures2 deviceFeatures2{};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.features.samplerAnisotropy = VK_TRUE;
        
        deviceFeatures2.pNext = &deviceFeaturesVulkan12;
        deviceFeaturesVulkan12.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
        
        vkGetPhysicalDeviceFeatures2(physicalDevice_, &deviceFeatures2);
        
        if (!deviceFeaturesVulkan12.shaderStorageBufferArrayNonUniformIndexing) {
            throw std::runtime_error("Dynamic indexing not available.");
        }

        deviceFeaturesVulkan12.pNext = &robustness2Features;
        deviceFeatures2.pNext = &deviceFeaturesVulkan12;
        createInfo.pNext = &deviceFeatures2;

        if (vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        //Retrieve the queues we want to use and keep a pointer to them
        vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 0, &graphicsQueue_);
        vkGetDeviceQueue(device_, indices.presentFamily.value(), 0, &presentQueue_);

    }

}
