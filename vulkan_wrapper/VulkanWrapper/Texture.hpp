#pragma once

#include <vulkan/vulkan.h>

#include <VulkanWrapper/Device.hpp>
#include <VulkanWrapper/Helper/Buffer.hpp>
#include <VulkanWrapper/Helper/Image.hpp>

#include <vk_mem_alloc.h>

#include <vector>


class Texture {

    public:

        struct TextureInformations {
            uint32_t binding;
            uint32_t width;
            uint32_t height;
            uint32_t depth; //Note : should be 1 if not used
            VkShaderStageFlags flags;
        };

        Texture(const Device* device, size_t dataSize, TextureInformations const& textureInformations)
            : device_(device), size_(dataSize), textureInformations_(textureInformations) {}

        Texture(const Device* device, VkCommandPool commandPool, VkQueue queue,
            std::vector<uint8_t> const& data, TextureInformations const& textureInformations)
            : device_(device), size_(data.size()), textureInformations_(textureInformations) {
            
            createTextureBuffers(commandPool, queue, data);
            textureImageView_= Image::createImageView(device_->get(), textureImage_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
            createTextureSampler();

        }
        

        Texture(Texture&& movedTexture) :
            size_(std::move(movedTexture.size_)),
            textureInformations_(std::move(movedTexture.textureInformations_)),
            device_(std::move(movedTexture.device_)),
            textureImage_(std::move(movedTexture.textureImage_)),
            textureImageAllocation_(std::move(movedTexture.textureImageAllocation_)),
            textureImageView_(std::move(movedTexture.textureImageView_)),
            textureSampler_(std::move(movedTexture.textureSampler_))
        {
            movedTexture.textureImage_ = nullptr;
            movedTexture.textureImageAllocation_ = nullptr;
            movedTexture.textureImageView_ = nullptr;
            movedTexture.textureSampler_ = nullptr;
        }

        Texture& operator=(Texture&& movedTexture) = delete;

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;


        ~Texture() {
            if(textureSampler_) {
                vkDestroySampler(device_->get(), textureSampler_, nullptr);
                textureSampler_ = nullptr;
            }
            if (textureImageView_) {
                vkDestroyImageView(device_->get(), textureImageView_, nullptr);
                textureImageView_ = nullptr;
            }
            if (textureImage_) {
                vmaDestroyImage(device_->getAllocator(), textureImage_, textureImageAllocation_);
                textureImageAllocation_ = nullptr;
                textureImage_ = nullptr;
            }


        }

        virtual void createTextureBuffers(VkCommandPool commandPool, VkQueue queue, std::vector<uint8_t> const& data) {
            
            VkBuffer stagingBuffer;
            VmaAllocation stagingBufferAllocation;
            VmaAllocationInfo bufferAllocInfo;

            Buffer::create(device_->getAllocator(), size_, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, stagingBuffer, stagingBufferAllocation, bufferAllocInfo);

            memcpy(bufferAllocInfo.pMappedData, data.data(), static_cast<size_t>(size_));
    
            Buffer::createImage(device_->getAllocator(), textureInformations_.width, textureInformations_.height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 0, textureImage_, textureImageAllocation_); 

            // Change the organisation of the image to optimize the data reception
            Image::transitionImageLayout(device_->get(), commandPool, queue, textureImage_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            // Copy the buffer into the image
            Buffer::copyToImage(device_->get(), commandPool, queue, stagingBuffer, textureImage_, textureInformations_.width, textureInformations_.height);

            // Then change again the organisation of the image after the copy to optimize the read in the shader
            Image::transitionImageLayout(device_->get(), commandPool, queue, textureImage_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            vmaDestroyBuffer(device_->getAllocator(), stagingBuffer, stagingBufferAllocation);
        }

        virtual void createTextureSampler() {

            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;

            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

            samplerInfo.anisotropyEnable = VK_TRUE;
            samplerInfo.maxAnisotropy = 16.0f;

            samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

            //Si vrai coordonnée [0 - size[
            samplerInfo.unnormalizedCoordinates = VK_FALSE;

            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.mipLodBias = 0.0f;
            samplerInfo.minLod = 0.0f;
            samplerInfo.maxLod = 0.0f;

            if (vkCreateSampler(device_->get(), &samplerInfo, nullptr, &textureSampler_) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create sampler !");
            }

        }

        TextureInformations const& getInformations() const {
            return textureInformations_;
        }

        //TODO: verify const keyword
        const VkImageView getImageView() const {
            return textureImageView_;
        }

        const VkSampler getSampler() const {
            return textureSampler_;
        }

    protected:
        VkDeviceSize size_;
        TextureInformations textureInformations_;
        
        //Saved vulkan objects
        const Device* device_;
        // VkCommandPool commandPool_;
        // VkQueue queue_;

        VkImage textureImage_;
        VmaAllocation textureImageAllocation_;
        VkImageView textureImageView_;
        VkSampler textureSampler_;

};
