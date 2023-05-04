#pragma once

#include <vulkan/vulkan.h>

#include <VulkanWrapper/Instance.hpp>

#include <iostream>
#include <cassert>


class DebugMessenger {

    public:
        DebugMessenger(Instance const& instance, bool initialize);

        ~DebugMessenger();

        DebugMessenger(DebugMessenger&&) = delete; //TODO: Declarer un move constructor
        DebugMessenger& operator=(DebugMessenger&&) = delete;

        DebugMessenger(const DebugMessenger&) = delete;
        DebugMessenger& operator=(const DebugMessenger&) = delete;
        
        //Setup and clean phase
        void setup(VkInstance instance);

        void clean(const VkAllocationCallbacks* pAllocator = nullptr);


    private:
        VkDebugUtilsMessengerEXT debugMessenger_ = nullptr;

        //Save a pointer to the Vulkan instance
        VkInstance instance_ = nullptr;

};
