
#pragma once

#include <vulkan/vulkan.h>

#include <VulkanWrapper/Device.hpp>

#include <vector>
#include <stdexcept>

struct SynchronisationObjects {

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    SynchronisationObjects(uint16_t framesInFlight, Device const& device) : devicePtr_(device.get()) {
        initializeSyncObjects(framesInFlight);
        
    };

    ~SynchronisationObjects() {
        for (size_t i = 0; i < imageAvailableSemaphores.size(); i++) {
			vkDestroySemaphore(devicePtr_, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(devicePtr_, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(devicePtr_, inFlightFences[i], nullptr);
		}
    }

    SynchronisationObjects(SynchronisationObjects&&) = delete; //TODO: Declarer un move constructor
    SynchronisationObjects& operator=(SynchronisationObjects&&) = delete;

    SynchronisationObjects(const SynchronisationObjects&) = delete;
    SynchronisationObjects& operator=(const SynchronisationObjects&) = delete;

    private:
        VkDevice devicePtr_;

        void initializeSyncObjects(uint16_t framesInFlight) {

            imageAvailableSemaphores.resize(framesInFlight);
            renderFinishedSemaphores.resize(framesInFlight);
            inFlightFences.resize(framesInFlight);

            VkSemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            for (size_t i = 0; i < framesInFlight; i++) {

                if (vkCreateSemaphore(devicePtr_, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                    vkCreateSemaphore(devicePtr_, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                    vkCreateFence(devicePtr_, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)  {

                    throw std::runtime_error("Failed to create syncronization objects for a frame !");
                }

            }

        }

};
