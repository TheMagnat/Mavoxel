
#pragma once

#include <VulkanWrapper/BufferWrapper.hpp>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <vector>

namespace vuw {

    struct SafeBufferWrapper {

        SafeBufferWrapper(VmaAllocator allocator, VkBuffer buffer, VmaAllocation bufferAllocation, std::vector<bool> const& bufferState, uint8_t bufferStateTrueNb)
            : bufferWrapper(allocator, buffer, bufferAllocation), bufferState_(bufferState), bufferStateTrueNb_(bufferStateTrueNb) {}

        SafeBufferWrapper(SafeBufferWrapper&&) = delete; //TODO: Declarer un move constructor
        SafeBufferWrapper& operator=(SafeBufferWrapper&&) = delete;

        SafeBufferWrapper(const SafeBufferWrapper&) = delete;
        SafeBufferWrapper& operator=(const SafeBufferWrapper&) = delete;
        
        BufferWrapper bufferWrapper;
        
        //The state of each value that may use the buffer, true mean it can be freed
        std::vector<bool> bufferState_;
        uint8_t bufferStateTrueNb_ = 0; //Keep the count of the number of true


    };

}