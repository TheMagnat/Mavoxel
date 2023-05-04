
#pragma once

#include <vulkan/vulkan.h>

#include <VulkanWrapper/Allocator.hpp>
#include <VulkanWrapper/Instance.hpp>
#include <VulkanWrapper/Surface.hpp>

#include <VulkanWrapper/Helper/PhysicalDevices.hpp>

#include <stdexcept>


class Device {

    public:
        Device(Instance const& instance, Surface const& surface, bool enableValidationLayers = false);

        ~Device();

        Device(Device&&) = delete; //TODO: Declarer un move constructor
        Device& operator=(Device&&) = delete;

        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;

        void pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);

        void createLogicalDevice(VkSurfaceKHR surface, bool enableValidationLayers);
        
        inline VkDevice get() const {
            return device_;
        }

        inline VkPhysicalDevice getPhysical() const {
            return physicalDevice_;
        }

        inline VkQueue getGraphicsQueue() const {
            return graphicsQueue_;
        }

        inline VkQueue getPresentQueue() const {
            return presentQueue_;
        };

        inline VmaAllocator getAllocator() const {
            return allocator_.get();
        }

    private:
        VkDevice device_;
        VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;;

        //Queues (note: the queues are implicitly cleaned up when the device is destroyed)
        VkQueue graphicsQueue_;
        VkQueue presentQueue_;

        //Allocator to reserve memory on GPU
        Allocator allocator_;
};
