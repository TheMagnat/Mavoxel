#pragma once

#include <VulkanWrapper/Allocator.hpp>

#include <VulkanWrapper/Device.hpp>


Allocator::Allocator() {}

Allocator::~Allocator() {
    if (allocator_) {
        vmaDestroyAllocator(allocator_);
        allocator_ = nullptr;
    }
}

Allocator::Allocator(Instance const& instance, Device const& device) {
    initializeAllocator(instance, device);
}

void Allocator::initializeAllocator(Instance const& instance, Device const& device) {
    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    //allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_;
    allocatorCreateInfo.physicalDevice = device.getPhysical();
    allocatorCreateInfo.device = device.get();
    allocatorCreateInfo.instance = instance.get();
    
    vmaCreateAllocator(&allocatorCreateInfo, &allocator_);
}
