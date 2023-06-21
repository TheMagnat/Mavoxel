
#pragma once

#include <vulkan/vulkan.h>
#include <VulkanWrapper/Instance.hpp>
#include <GLFW/glfw3.h>

#include <stdexcept>


namespace vuw {

    class Surface {

        public:
            Surface(GLFWwindow* window, Instance const& instance) : instancePtr_(instance.get()) {
                
                if (glfwCreateWindowSurface(instancePtr_, window, nullptr, &surface_) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create window surface !");
                }

            };

            ~Surface() {
                vkDestroySurfaceKHR(instancePtr_, surface_, nullptr);
            }

            Surface(Surface&&) = delete; //TODO: Declarer un move constructor
            Surface& operator=(Surface&&) = delete;

            Surface(const Surface&) = delete;
            Surface& operator=(const Surface&) = delete;

            VkSurfaceKHR get() const {
                return surface_;
            }
            

        private:
            VkInstance instancePtr_;

            VkSurfaceKHR surface_;


    };

}
