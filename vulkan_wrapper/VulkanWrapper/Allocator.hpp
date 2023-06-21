#pragma once

#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

#include <VulkanWrapper/Instance.hpp>

namespace vuw {

    class Device;

    class Allocator {

        public:

            Allocator();
            
            Allocator(Instance const& instance, Device const& device);
            ~Allocator();
            
            Allocator(Allocator&&) = delete;
            Allocator& operator=(Allocator&&) = delete;

            Allocator(const Allocator&) = delete;
            Allocator& operator=(const Allocator&) = delete;

            void initializeAllocator(Instance const& instance, Device const& device);

            VmaAllocator get() const {
                return allocator_;
            }

        private:
            VmaAllocator allocator_;

    };

}
