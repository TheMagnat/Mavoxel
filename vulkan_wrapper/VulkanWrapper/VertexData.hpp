
#pragma once

#include <vulkan/vulkan.h>

#include <VulkanWrapper/Device.hpp>
#include <VulkanWrapper/Helper/Buffer.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>
#include <array>
#include <numeric>

struct VertexData {

    VertexData(const Device* device, VkCommandPool commandPool)
        : device_(device), commandPool_(commandPool) {}

    ~VertexData() {
        
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

        void* data;

        //// Create vertex buffer
        VkDeviceSize vertexBufferSize = sizeof(float) * vertices.size();

		VkBuffer stagingVertexBuffer;
		VmaAllocation stagingVertexBufferAllocation;
		VmaAllocationInfo stagingVertexBufferAllocationInfo;

		Buffer::create(device_->getAllocator(), vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, stagingVertexBuffer, stagingVertexBufferAllocation, &stagingVertexBufferAllocationInfo);

		// vkMapMemory(device_->get(), stagingVertexBufferMemory, 0, vertexBufferSize, 0, &data);
		memcpy(stagingVertexBufferAllocationInfo.pMappedData, vertices.data(), (size_t) vertexBufferSize);
		// vkUnmapMemory(device_->get(), stagingVertexBufferMemory);

		Buffer::create(device_->getAllocator(), vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 0, vertexBuffer_, vertexBufferAllocation_, nullptr);

		Buffer::copy(device_->get(), commandPool_, device_->getGraphicsQueue(), stagingVertexBuffer, vertexBuffer_, vertexBufferSize);

		// vkDestroyBuffer(device_->get(), stagingVertexBuffer, nullptr);
		// vkFreeMemory(device_->get(), stagingVertexBufferMemory, nullptr);
        vmaDestroyBuffer(device_->getAllocator(), stagingVertexBuffer, stagingVertexBufferAllocation);

        //// Create index buffer
        VkDeviceSize indexBufferSize = sizeof(uint32_t) * indices.size();

        VkBuffer stagingIndexBuffer;
        VmaAllocation stagingIndexBufferAllocation;
        VmaAllocationInfo stagingIndexBufferAllocationInfo;

        Buffer::create(device_->getAllocator(), indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, stagingIndexBuffer, stagingIndexBufferAllocation, &stagingIndexBufferAllocationInfo);

        // vkMapMemory(device_->get(), stagingIndexBufferMemory, 0, indexBufferSize, 0, &data);
        memcpy(stagingIndexBufferAllocationInfo.pMappedData, indices.data(), (size_t) indexBufferSize);
        // vkUnmapMemory(device_->get(), stagingIndexBufferMemory);

        Buffer::create(device_->getAllocator(), indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 0, indexBuffer_, indexBufferAllocation_, nullptr);

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

        VkBuffer vertexBuffer_ = nullptr;
        VmaAllocation vertexBufferAllocation_ = nullptr;

        VkBuffer indexBuffer_ = nullptr;
        VmaAllocation indexBufferAllocation_ = nullptr;

        uint32_t indicesSize_ = 0;

};
