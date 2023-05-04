
#pragma once

#include <vulkan/vulkan.h>

#include <VulkanWrapper/Configuration.hpp>

#include <VulkanWrapper/Helper/Checker.hpp>
#include <VulkanWrapper/Helper/Getter.hpp>
#include <VulkanWrapper/Helper/DebugMessengerHelper.hpp>

#include <stdexcept>


class Instance {

    public:

        Instance(bool enableValidationLayers = false) {
            initializeInstance(enableValidationLayers);
        }

        ~Instance() {
            vkDestroyInstance(instance_, nullptr);
        }
        
        Instance(Instance&&) = delete; //TODO: Declarer un move constructor
        Instance& operator=(Instance&&) = delete;

        Instance(const Instance&) = delete;
        Instance& operator=(const Instance&) = delete;

        void initializeInstance(bool enableValidationLayers) {
            
            if (enableValidationLayers && !Checker::validationLayerSupport())
                throw std::runtime_error("validation layers requested, but not available!");        
            
            //Generate the informations of the application for the vulkan instance (stored in the vulkan instance create info structure)
            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Hello Triangle";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "No Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;


            //Code to get a list of all supported availableExtensions
            std::vector<VkExtensionProperties> availableExtensions = Getter::availableExtensions();
            
            // std::cout << "available extensions:\n";
            // for (const auto& extension : availableExtensions) {
            // 	std::cout << '\t' << extension.extensionName << '\n';
            // }

            std::vector<const char*> requiredExtensions = Getter::requiredExtensions(enableValidationLayers);

            //And verify if the required are availables
            if (Checker::requiredExtensionAreAvailable(requiredExtensions, availableExtensions) == false) {
                throw std::runtime_error("Required extensions are not available.");
            }
            
            //Generate the informations for the creation of the vulkan instance then create it
            VkInstanceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;
            createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
            createInfo.ppEnabledExtensionNames = requiredExtensions.data();

            //If enabled, add the validation layers
            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
            if (enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();

                debugCreateInfo = DebugMessengerHelper::generateCreateInfo();
                createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo; //Note: cast inutile ?
            }
            else {
                createInfo.enabledLayerCount = 0;
                createInfo.pNext = nullptr;
            }

            if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS) {
                throw std::runtime_error("failed to create instance!");
            }

        }

        VkInstance get() const {
            return instance_;
        }

    private:
        VkInstance instance_;

};
