
#pragma once

#include <VulkanWrapper/Helper/Buffer.hpp>

#include <vector>

namespace vuw {

    class SSBO {

        public:

            SSBO (const Device* device, VkCommandPool commandPool)
                : device_(device), commandPool_(commandPool) {}

            ~SSBO() {
                clean();
            }

            void clean() {

                if (buffer_) {
                    vmaDestroyBuffer(device_->getAllocator(), buffer_, bufferAllocation_);
                    buffer_ = nullptr;
                    bufferAllocation_ = nullptr;
                }

                if (stagingBuffer_) {
                    vmaDestroyBuffer(device_->getAllocator(), stagingBuffer_, stagingBufferAllocation_);
                    stagingBuffer_ = nullptr;
                    stagingBufferAllocation_ = nullptr;
                }

            }

            SSBO(SSBO&& movedSSBO) :
                //Vulkan objects
                device_(std::move(movedSSBO.device_)),
                commandPool_(std::move(movedSSBO.commandPool_)),

                //Buffer
                buffer_(std::move(movedSSBO.buffer_)),
                bufferAllocation_(std::move(movedSSBO.bufferAllocation_)),
                bufferAllocationInfo_(std::move(movedSSBO.bufferAllocationInfo_))
            {
                movedSSBO.buffer_ = nullptr;
                movedSSBO.bufferAllocation_ = nullptr;
            }

            SSBO& operator=(SSBO&& movedSSBO) = delete;

            SSBO(const SSBO&) = delete;
            SSBO& operator=(const SSBO&) = delete;
            
            template<typename T>
            void generate(std::vector<T> const& data) {

                size_t bufferSize = data.size() * sizeof(T);

                Buffer::create(device_->getAllocator(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, stagingBuffer_, stagingBufferAllocation_, stagingBufferAllocationInfo_);

                memcpy(stagingBufferAllocationInfo_.pMappedData, data.data(), bufferSize);
        
                Buffer::create(device_->getAllocator(), bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT , 0, buffer_, bufferAllocation_, bufferAllocationInfo_);

                // Copy the staged buffer into the GPU buffer
                Buffer::copy(device_->get(), commandPool_, device_->getGraphicsQueue(), stagingBuffer_, buffer_, bufferSize);

            }

            template<typename T>
            void update(std::vector<T> const& data) {

                size_t bufferSize = data.size() * sizeof(T);

                //Update size ?
                // Buffer::create(device_->getAllocator(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, stagingBuffer_, stagingBufferAllocation_, stagingBufferAllocationInfo_);

                if (!buffer_) {
                    Buffer::create(device_->getAllocator(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, stagingBuffer_, stagingBufferAllocation_, stagingBufferAllocationInfo_);
                }
                else {
                    if (stagingBufferAllocationInfo_.size < bufferSize) {
                        //TODO: ? deadBufferHandler_->addBufferToDeleteQueue(indexBuffer_, indexBufferAllocation_);
                        vmaDestroyBuffer(device_->getAllocator(), stagingBuffer_, stagingBufferAllocation_);
                        Buffer::create(device_->getAllocator(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, stagingBuffer_, stagingBufferAllocation_, stagingBufferAllocationInfo_);
                    }
                }

                memcpy(stagingBufferAllocationInfo_.pMappedData, data.data(), bufferSize);

                //Update size ?
                //Buffer::create(device_->getAllocator(), bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT , 0, buffer_, bufferAllocation_, bufferAllocationInfo_);

                if (!buffer_) {
                    Buffer::create(device_->getAllocator(), bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT , 0, buffer_, bufferAllocation_, bufferAllocationInfo_);
                }
                else {
                    if (bufferAllocationInfo_.size < bufferSize) {
                        //TODO: ? deadBufferHandler_->addBufferToDeleteQueue(indexBuffer_, indexBufferAllocation_);
                        vmaDestroyBuffer(device_->getAllocator(), buffer_, bufferAllocation_);
                        Buffer::create(device_->getAllocator(), bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT , 0, buffer_, bufferAllocation_, bufferAllocationInfo_);
                    }
                }

                // Copy the staged buffer into the GPU buffer
                Buffer::copy(device_->get(), commandPool_, device_->getGraphicsQueue(), stagingBuffer_, buffer_, bufferSize);

            }

            VkBuffer getBuffer() const {
                return buffer_;
            }

            VkDeviceSize getBufferSize() const {
                return bufferAllocationInfo_.size;
            }

        private:
            const Device* device_;
            VkCommandPool commandPool_;

            //Buffer
            VkBuffer buffer_ = nullptr;
            VmaAllocation bufferAllocation_ = nullptr;
            VmaAllocationInfo bufferAllocationInfo_;

            //For memory transfert
            VkBuffer stagingBuffer_ = nullptr;
            VmaAllocation stagingBufferAllocation_ = nullptr;
            VmaAllocationInfo stagingBufferAllocationInfo_;
    
            //Shaders
            VkDescriptorSetLayoutBinding layoutBinding_;
    };

}
