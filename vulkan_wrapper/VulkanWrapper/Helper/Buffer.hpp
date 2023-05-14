
#pragma once

#include <vulkan/vulkan.h>

#include <VulkanWrapper/Allocator.hpp>
#include <VulkanWrapper/Helper/Command.hpp>

#include <stdexcept>


class Buffer {

    public:

        static void createImage(VmaAllocator allocator, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaAllocationCreateFlags flags, VkImage& image, VmaAllocation& imageAllocation) {
            
            VkImageCreateInfo imageCreateInfo{};
            imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            imageCreateInfo.extent.width = width;
            imageCreateInfo.extent.height = height;
            imageCreateInfo.extent.depth = 1;
            imageCreateInfo.mipLevels = 1;
            imageCreateInfo.arrayLayers = 1;

            imageCreateInfo.format = format;

            //TODO: voir l'impact de changer ça pour "VK_IMAGE_TILING_LINEAR" 
            imageCreateInfo.tiling = tiling;

            imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            
            imageCreateInfo.usage = usage;

            imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageCreateInfo.flags = 0; // Optional

            // if (vkCreateImage(device, &imageCreateInfo, nullptr, &image) != VK_SUCCESS) {
            //     throw std::runtime_error("Failed to create image !");
            // }

            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocCreateInfo.flags = flags;
            allocCreateInfo.priority = 1.0f;
            
            VkResult result = vmaCreateImage(allocator, &imageCreateInfo, &allocCreateInfo, &image, &imageAllocation, nullptr);
            if (result != VK_SUCCESS)
                throw std::exception("Failed to allocate memory for image buffer.");

            // VkMemoryRequirements memRequirements;
            // vkGetImageMemoryRequirements(device, image, &memRequirements);

            // VkMemoryAllocateInfo allocInfo{};
            // allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            // allocInfo.allocationSize = memRequirements.size;
            // allocInfo.memoryTypeIndex = Buffer::findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

            // if (vkAllocateMemory(device, &allocInfo, nullptr, & imageMemory) != VK_SUCCESS) {
            //     throw std::runtime_error("Failed to allocate memory for the image !");
            // }

            // vkBindImageMemory(device, image,  imageMemory, 0);

        }

        static void create(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaAllocationCreateFlags flags, VkBuffer& buffer, VmaAllocation& bufferAllocation, VmaAllocationInfo& bufferAllocInfo) {
            VkBufferCreateInfo bufCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
            bufCreateInfo.size = size;
            bufCreateInfo.usage = usage;
            bufCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            
            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocCreateInfo.flags = flags;
            
            //VkBuffer buf;
            //VmaAllocation alloc;
            //VmaAllocationInfo allocInfo;
            VmaTotalStatistics stats;
            vmaCalculateStatistics (allocator, &stats);

            VkResult result = vmaCreateBuffer(allocator, &bufCreateInfo, &allocCreateInfo, &buffer, &bufferAllocation, &bufferAllocInfo);
            if (result != VK_SUCCESS)
                throw std::exception("Failed to allocate memory for buffer.");

            vmaCalculateStatistics (allocator, &stats);

            // if (vkCreateBuffer(device, &bufCreateInfo, nullptr, &buffer) != VK_SUCCESS) {
            //     throw std::runtime_error("Failed to create a buffer !");
            // }

            // VkMemoryRequirements memRequirements;
            // vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

            // VkMemoryAllocateInfo allocInfo{};
            // allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            // allocInfo.allocationSize = memRequirements.size;
            // allocInfo.memoryTypeIndex = Buffer::findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

            // if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            //     throw std::runtime_error("Failed to allocate memory !");
            // }

            // vkBindBufferMemory(device, buffer, bufferMemory, 0);
        }

        static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
            
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }

            throw std::runtime_error("No type of memory satisfies the buffer !");


        }

        static void copy(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {

            VkCommandBuffer commandBuffer = Command::beginSingleTimeCommands(device, commandPool);

            VkBufferCopy copyRegion{};
            // copyRegion.srcOffset = 0; // Optional
            // copyRegion.dstOffset = 0; // Optional
            copyRegion.size = size;

            if (dstBuffer == VK_NULL_HANDLE) {
                std::cout << "Null" << std::endl;
            }

            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

            // End recording
            Command::endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);
        }

        static void copyToImage(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
            
            VkCommandBuffer commandBuffer = Command::beginSingleTimeCommands(device, commandPool);

            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            region.imageOffset = {0, 0, 0};
            region.imageExtent = {
                width,
                height,
                1
            };

            vkCmdCopyBufferToImage(
                commandBuffer,
                buffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region
            );

            Command::endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);

	}

};
