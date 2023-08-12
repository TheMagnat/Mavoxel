
#pragma once

#include <vulkan/vulkan.h>

#include <VulkanWrapper/Device.hpp>
#include <VulkanWrapper/DeadBufferHandler.hpp>
#include <VulkanWrapper/Helper/Buffer.hpp>


#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>
#include <array>
#include <numeric>

namespace vuw {

    struct VertexData {

        VertexData(const Device* device, VkCommandPool commandPool, DeadBufferHandler* deadBufferHandler)
            : device_(device), commandPool_(commandPool), deadBufferHandler_(deadBufferHandler) {}

        ~VertexData() {
            
            clean();

        }

        void clean() {
            if (vertexBuffer_) {
                vmaDestroyBuffer(device_->getAllocator(), vertexBuffer_, vertexBufferAllocation_);
                vertexBuffer_ = nullptr;
                vertexBufferAllocation_ = nullptr;
            }

            if (indexBuffer_) {
                vmaDestroyBuffer(device_->getAllocator(), indexBuffer_, indexBufferAllocation_);
                indexBuffer_ = nullptr;
                indexBufferAllocation_ = nullptr;
            }
        }

        VertexData(VertexData&&) = delete; //TODO: Declarer un move constructor
        VertexData& operator=(VertexData&&) = delete;

        VertexData(const VertexData&) = delete;
        VertexData& operator=(const VertexData&) = delete;

        void setDevice(const Device* device) {
            device_ = device;
        }

        void bind(VkCommandBuffer commandBuffer) const {
            VkBuffer vertexBuffers[] = {vertexBuffer_};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer_, 0, VK_INDEX_TYPE_UINT32);
        }

        void draw(VkCommandBuffer commandBuffer) const {
            //Command buffer, number of indices, number of instances (commonly 1), vertices start index offset, instances start index offset
            vkCmdDrawIndexed(commandBuffer, indicesSize_, 1, 0, 0, 0);
        }

        VkResult setData(std::vector<float> const& vertices, std::vector<uint32_t> const& indices) {
            
            if (vertices.empty() || indices.empty()) return VK_SUCCESS;

            //The requested sizes
            VkDeviceSize vertexBufferSize = sizeof(float) * vertices.size();
            VkDeviceSize indexBufferSize = sizeof(uint32_t) * indices.size();

            //// Create vertex buffer

            //Staging buffer
            VkBuffer stagingVertexBuffer;
            VmaAllocation stagingVertexBufferAllocation;
            VmaAllocationInfo stagingVertexBufferAllocationInfo;

            //Create buffer then copy to the mapped pointer of this buffer the data we want to transfer to the GPU
            Buffer::create(device_->getAllocator(), vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, stagingVertexBuffer, stagingVertexBufferAllocation, stagingVertexBufferAllocationInfo);
            memcpy(stagingVertexBufferAllocationInfo.pMappedData, vertices.data(), (size_t) vertexBufferSize);

            if (!vertexBuffer_) {
                Buffer::create(device_->getAllocator(), vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 0, vertexBuffer_, vertexBufferAllocation_, vertexBufferAllocationInfo_);
            }
            else {
                if (vertexBufferAllocationInfo_.size < vertexBufferSize) {
                    deadBufferHandler_->addBufferToDeleteQueue(vertexBuffer_, vertexBufferAllocation_);
                    Buffer::create(device_->getAllocator(), vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 0, vertexBuffer_, vertexBufferAllocation_, vertexBufferAllocationInfo_);
                }
            }

            //TODO: Else si taille plus grande, delete aussi

            Buffer::copy(device_->get(), commandPool_, device_->getGraphicsQueue(), stagingVertexBuffer, vertexBuffer_, vertexBufferSize);

            // vkDestroyBuffer(device_->get(), stagingVertexBuffer, nullptr);
            // vkFreeMemory(device_->get(), stagingVertexBufferMemory, nullptr);
            vmaDestroyBuffer(device_->getAllocator(), stagingVertexBuffer, stagingVertexBufferAllocation);

            //// Create index buffer

            //Staging buffer
            VkBuffer stagingIndexBuffer;
            VmaAllocation stagingIndexBufferAllocation;
            VmaAllocationInfo stagingIndexBufferAllocationInfo;

            //Create buffer then copy to the mapped pointer of this buffer the indices we want to transfer to the GPU
            Buffer::create(device_->getAllocator(), indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, stagingIndexBuffer, stagingIndexBufferAllocation, stagingIndexBufferAllocationInfo);
            memcpy(stagingIndexBufferAllocationInfo.pMappedData, indices.data(), (size_t) indexBufferSize);

            if (!indexBuffer_) {
                Buffer::create(device_->getAllocator(), indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 0, indexBuffer_, indexBufferAllocation_, indexBufferAllocationInfo_);
            }
            else {
                if (indexBufferAllocationInfo_.size < indexBufferSize) {
                    deadBufferHandler_->addBufferToDeleteQueue(indexBuffer_, indexBufferAllocation_);
                    Buffer::create(device_->getAllocator(), indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 0, indexBuffer_, indexBufferAllocation_, indexBufferAllocationInfo_);
                }
            }

            Buffer::copy(device_->get(), commandPool_, device_->getGraphicsQueue(), stagingIndexBuffer, indexBuffer_, indexBufferSize);

            // vkDestroyBuffer(device_->get(), stagingIndexBuffer, nullptr);
            // vkFreeMemory(device_->get(), stagingIndexBufferMemory, nullptr);
            vmaDestroyBuffer(device_->getAllocator(), stagingIndexBuffer, stagingIndexBufferAllocation);

            indicesSize_ = indices.size();

            return VK_SUCCESS;

        }
        

        static std::pair<VkVertexInputBindingDescription, std::vector<VkVertexInputAttributeDescription>> getDescriptions(std::vector<uint32_t> const& attributesSize) {

            return { generateBindingDescription(attributesSize), generateAttributeDescriptions(attributesSize) };

        }

        static VkVertexInputBindingDescription generateBindingDescription(std::vector<uint32_t> const& attributesSize) {
            VkVertexInputBindingDescription bindingDescription;

            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(float) * std::reduce(attributesSize.begin(), attributesSize.end());
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::vector<VkVertexInputAttributeDescription> generateAttributeDescriptions(std::vector<uint32_t> const& attributesSize) {

            std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

            attributeDescriptions.resize( attributesSize.size() );

            for (uint32_t i = 0, acc = 0; i < attributeDescriptions.size(); ++i) {
                
                uint32_t numberOfFloat = attributesSize[i];

                attributeDescriptions[i].binding = 0;
                attributeDescriptions[i].location = i;

                if      (numberOfFloat == 1) attributeDescriptions[i].format = VK_FORMAT_R32_SFLOAT;
                else if (numberOfFloat == 2) attributeDescriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
                else if (numberOfFloat == 3) attributeDescriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
                else if (numberOfFloat == 4) attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
                else throw std::runtime_error("One of the attributes size is not legal !");
                
                attributeDescriptions[i].offset = acc * sizeof(float);

                acc += numberOfFloat;

            }

            return attributeDescriptions;

        }

        private:

            //Vulkan objects save
            const Device* device_;
            VkCommandPool commandPool_;
            DeadBufferHandler* deadBufferHandler_;

            VkBuffer vertexBuffer_ = nullptr;
            VmaAllocation vertexBufferAllocation_ = nullptr;
            VmaAllocationInfo vertexBufferAllocationInfo_;

            VkBuffer indexBuffer_ = nullptr;
            VmaAllocation indexBufferAllocation_ = nullptr;
            VmaAllocationInfo indexBufferAllocationInfo_;

            uint32_t indicesSize_ = 0;

    };

}
