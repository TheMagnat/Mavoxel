#pragma once

#include <vulkan/vulkan.h>

#include <VulkanWrapper/Device.hpp>
#include <VulkanWrapper/Texture.hpp>
#include <VulkanWrapper/Helper/Buffer.hpp>
#include <VulkanWrapper/Helper/Image.hpp>

#include <vk_mem_alloc.h>

#include <vector>


namespace vuw {

    class Texture3D : public Texture {

        public:

            Texture3D(const Device* device, VkCommandPool commandPool, VkQueue queue,
                std::vector<uint8_t> const& data, TextureInformations const& textureInformations)
                : Texture(device, data.size(), textureInformations) {
                
                createTextureBuffers(commandPool, queue, data);
                textureImageView_= Image::createImageView(device_->get(), textureImage_, VK_FORMAT_R8_UINT, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_3D);
                createTextureSampler();
            }
            
            Texture3D(Texture3D&& movedTexture) : Texture(std::move(movedTexture)) {}

            Texture3D& operator=(Texture3D&& movedTexture) = delete;

            Texture3D(const Texture3D&) = delete;
            Texture3D& operator=(const Texture3D&) = delete;

            //TODO: verify default is ok
            //~Texture3D() {}

            void createTextureBuffers(VkCommandPool commandPool, VkQueue queue, std::vector<uint8_t> const& data) override {
                
                VkBuffer stagingBuffer;
                VmaAllocation stagingBufferAllocation;
                VmaAllocationInfo bufferAllocInfo;

                Buffer::create(device_->getAllocator(), size_, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, stagingBuffer, stagingBufferAllocation, bufferAllocInfo);

                memcpy(bufferAllocInfo.pMappedData, data.data(), static_cast<size_t>(size_));
        
                Buffer::create3DImage(device_->getAllocator(), textureInformations_.width, textureInformations_.height, textureInformations_.depth, VK_FORMAT_R8_UINT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 0, textureImage_, textureImageAllocation_); 

                // Change the organisation of the image to optimize the data reception
                Image::transitionImageLayout(device_->get(), commandPool, queue, textureImage_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

                // Copy the buffer into the image
                Buffer::copyTo3DImage(device_->get(), commandPool, queue, stagingBuffer, textureImage_, textureInformations_.width, textureInformations_.height, textureInformations_.depth);

                // Then change again the organisation of the image after the copy to optimize the read in the shader
                Image::transitionImageLayout(device_->get(), commandPool, queue, textureImage_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                vmaDestroyBuffer(device_->getAllocator(), stagingBuffer, stagingBufferAllocation);
            }

            void createTextureSampler() override {

                VkSamplerCreateInfo samplerInfo{};
                samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerInfo.magFilter = VK_FILTER_NEAREST;
                samplerInfo.minFilter = VK_FILTER_NEAREST;

                samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

                samplerInfo.anisotropyEnable = VK_FALSE;
                // samplerInfo.maxAnisotropy = 16.0f;

                samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

                //Si vrai coordonnée [0 - size[
                samplerInfo.unnormalizedCoordinates = VK_TRUE;

                samplerInfo.compareEnable = VK_FALSE;
                samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

                samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
                samplerInfo.mipLodBias = 0.0f;
                samplerInfo.minLod = 0.0f;
                samplerInfo.maxLod = 0.0f;

                if (vkCreateSampler(device_->get(), &samplerInfo, nullptr, &textureSampler_) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create sampler !");
                }

            }

    };

}
