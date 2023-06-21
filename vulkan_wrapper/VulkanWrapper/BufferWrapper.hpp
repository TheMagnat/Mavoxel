
#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>


namespace vuw {

    class BufferWrapper {

        public:

            BufferWrapper();
            BufferWrapper(VmaAllocator allocator, VkBuffer buffer, VmaAllocation bufferAllocation) : allocator_(allocator), buffer_(buffer), bufferAllocation_(bufferAllocation) {}

            ~BufferWrapper() {
                clean();
            }

            BufferWrapper(BufferWrapper&&) = delete; //TODO: Declarer un move constructor
            BufferWrapper& operator=(BufferWrapper&&) = delete;

            BufferWrapper(const BufferWrapper&) = delete;
            BufferWrapper& operator=(const BufferWrapper&) = delete;

            void clean() {
                if (allocator_) {
                    vmaDestroyBuffer(allocator_, buffer_, bufferAllocation_);
                    buffer_ = nullptr;
                    bufferAllocation_ = nullptr;
                }
            }

        private:
            VmaAllocator allocator_;
            VkBuffer buffer_;
            VmaAllocation bufferAllocation_;

    };

}
