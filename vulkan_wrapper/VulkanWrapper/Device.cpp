
#pragma once

#include <VulkanWrapper/Device.hpp>


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
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    //Logical device informations
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    

    //This may be useless, validations layers are now useless in device since they use now the same as the instance validation layer
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    //Retrieve the queues we want to use and keep a pointer to them
    vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, indices.presentFamily.value(), 0, &presentQueue_);

}
        