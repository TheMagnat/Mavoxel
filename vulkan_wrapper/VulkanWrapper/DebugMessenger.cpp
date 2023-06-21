
#include <VulkanWrapper/DebugMessenger.hpp>

#include <VulkanWrapper/Helper/DebugMessengerHelper.hpp>

namespace {

    VkResult getInstanceAndCreateDebugMessenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

}

namespace vuw {

    DebugMessenger::DebugMessenger(Instance const& instance, bool initialize) {
        if (initialize) setup(instance.get());
    }

    DebugMessenger::~DebugMessenger() {
        clean();
    }

    //Setup and clean phase
    void DebugMessenger::setup(VkInstance instance) {
        
        assert(instance != nullptr);
        assert(debugMessenger_ == nullptr);

        instance_ = instance;

        VkDebugUtilsMessengerCreateInfoEXT createInfo = DebugMessengerHelper::generateCreateInfo();

        if (getInstanceAndCreateDebugMessenger(instance, &createInfo, nullptr, &debugMessenger_) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }

    }

    void DebugMessenger::clean(const VkAllocationCallbacks* pAllocator) {

        assert(instance_ != nullptr);

        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance_, debugMessenger_, pAllocator);
        }
    }

}
