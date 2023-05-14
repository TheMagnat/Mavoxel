
#pragma once

#include <vulkan/vulkan.h>

#include <VulkanWrapper/Device.hpp>
#include <VulkanWrapper/Allocator.hpp>
#include <VulkanWrapper/Texture.hpp>

#include <VulkanWrapper/Helper/Buffer.hpp>

#include <optional>
#include <vector>
#include <array>

namespace vuw {

    struct UniformInformations {
        uint32_t binding;
        VkDeviceSize bufferSize;
        VkShaderStageFlags flags;
    };

    struct PushConstantInformations {
        VkDeviceSize bufferSize;
        VkShaderStageFlags flags;
    };

    struct UniformBufferWrapper {

        UniformBufferWrapper(uint16_t nbFrames, UniformInformations const& uniformInformationP)
            : informations(uniformInformationP), uniformBuffers(nbFrames), uniformBuffersAllocation(nbFrames), uniformBuffersMapped(nbFrames) {}

        void allocate(VmaAllocator allocator, VkPhysicalDevice physicalDevice) {

            for (size_t i = 0; i < uniformBuffers.size(); i++) {
                VmaAllocationInfo allocationInfo;
                //TODO: pas sur pour les flags
                Buffer::create(allocator, informations.bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, uniformBuffers[i], uniformBuffersAllocation[i], allocationInfo);

                uniformBuffersMapped[i] = allocationInfo.pMappedData;
            }

            allocated_ = true;

        }

        void deallocate(VmaAllocator allocator) {
            if (allocated_) {

                for (size_t i = 0; i < uniformBuffers.size(); i++) {
                    vmaDestroyBuffer(allocator, uniformBuffers[i], uniformBuffersAllocation[i]);
                }

                allocated_ = false;
            }
        }

        UniformInformations informations;

        //Uniforms memory (Vulkan objects)
        std::vector<VkBuffer> uniformBuffers;
        std::vector<VmaAllocation> uniformBuffersAllocation;
        std::vector<void*> uniformBuffersMapped;

        bool allocated_ = false;

    };

    class Shader {

        public:
            Shader(const Device* device, uint16_t nbFrames, const std::string& vertexFilename, const std::string& fragmentFilename)
                : device_(device), nbFrames_(nbFrames), vertexFilename_(vertexFilename), fragmentFilename_(fragmentFilename) {}

            ~Shader() {
                clean();
            }

            void clean() {
                for (UniformBufferWrapper& uniformBufferWrapper : uniformBufferWrappers_)
                    uniformBufferWrapper.deallocate(device_->getAllocator());
                uniformBufferWrappers_.clear();

                if(descriptorPool_){
                    vkDestroyDescriptorPool(device_->get(), descriptorPool_, nullptr);
                    descriptorPool_ = nullptr;
                }

                if(descriptorSetLayout_){
                    vkDestroyDescriptorSetLayout(device_->get(), descriptorSetLayout_, nullptr);
                    descriptorSetLayout_ = nullptr;
                }
                
                if(pipelineLayout_){
                    vkDestroyPipelineLayout(device_->get(), pipelineLayout_, nullptr);
                    pipelineLayout_ = nullptr;
                }
            }

            Shader(Shader&& toMove) {

                //Vulkan objects
                device_ = std::move(toMove.device_);
                descriptorSetLayout_ = std::move(toMove.descriptorSetLayout_);
                toMove.descriptorSetLayout_ = nullptr; //To prevent destroy

                descriptorPool_ = std::move(toMove.descriptorPool_);
                toMove.descriptorPool_ = nullptr; //To prevent destroy

                descriptorSets_ = std::move(toMove.descriptorSets_);
                pipelineLayout_ = std::move(toMove.pipelineLayout_);
                toMove.pipelineLayout_ = nullptr; //To prevent destroy

                nbFrames_ = std::move(toMove.nbFrames_);
                vertexFilename_ = std::move(toMove.vertexFilename_);
                fragmentFilename_ = std::move(toMove.fragmentFilename_);

                pushConstantRange_ = std::move(toMove.pushConstantRange_);

                uniformBufferWrappers_ = std::move(toMove.uniformBufferWrappers_);
                nbUniforms_ = std::move(toMove.nbUniforms_);
                toMove.uniformBufferWrappers_.clear(); //To prevent deallocation

                textures_ = std::move(toMove.textures_);

            }

            Shader& operator=(Shader&&) = delete;

            Shader(const Shader&) = delete;
            Shader& operator=(const Shader&) = delete;

            void setDevice(const Device* device) {
                device_ = device;
            }

            void setPushConstant(PushConstantInformations const& pushContant) {
                VkPushConstantRange newPushConstantRange_;
                newPushConstantRange_.offset = 0;
                newPushConstantRange_.size = pushContant.bufferSize;
                newPushConstantRange_.stageFlags = pushContant.flags;

                pushConstantRange_ = newPushConstantRange_;
            }

            //Add uniforms to the shader structure and allocate the memory for them
            virtual void addUniformBufferObjects(std::vector<UniformInformations> const& uniformsInformations) {

                for (UniformInformations const& uniformInformation : uniformsInformations) {
                    uniformBufferWrappers_.emplace_back(nbFrames_, uniformInformation);
                    uniformBufferWrappers_.back().allocate(device_->getAllocator(), device_->getPhysical());
                }

                nbUniforms_ += uniformsInformations.size();

            }

            void addTexture(VkCommandPool commandPool, VkQueue queue, std::vector<uint8_t> const& texture, Texture::TextureInformations const& textureInformations) {

                textures_.emplace_back(
                    device_, commandPool, queue,
                    texture, textureInformations
                );
                
            }

            void addTexture(Texture&& texture) {
                textures_.emplace_back(std::move(texture));
            }

            void generateBindingsAndSets() {
                createLayoutPart();
                createDescriptor();
            }

            //CPU Part
            //Note: this part must be done before creating the graphic pipeline
            virtual void createLayoutPart() {
                createDescriptorSetLayout();
                createPipelineLayout();
            }

            //GPU Part
            //Note: this part should be done for each object that share this shader and have different uniforms
            //and can be done after the graphic pipeline, it just need to be done before calling draw and assigning uniform buffers
            void createDescriptor() {
                createDescriptorPool();
                createDescriptorSets();
            }

            ///  -> Finishing after all uniform creation
            //CPU
            virtual void createDescriptorSetLayout() {

                std::vector<VkDescriptorSetLayoutBinding> layoutBindings(nbUniforms_ + textures_.size());

                // Set the uniforms layout bindings
                for (size_t i = 0; i < nbUniforms_; ++i) {

                    layoutBindings[i].binding = uniformBufferWrappers_[i].informations.binding;
                    layoutBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    layoutBindings[i].descriptorCount = 1; //If array, put more than 1
                    layoutBindings[i].pImmutableSamplers = nullptr; // Optional

                    // Precise in which shader we will use the uniform
                    layoutBindings[i].stageFlags = uniformBufferWrappers_[i].informations.flags;

                }

                // Set the texture layout bindings
                for (size_t i = 0; i < textures_.size(); ++i) {
                    
                    size_t currentIndex = nbUniforms_ + i;

                    Texture::TextureInformations const& currentTextureInformations = textures_[i].getInformations();

                    layoutBindings[currentIndex].binding = currentTextureInformations.binding;
                    layoutBindings[currentIndex].descriptorCount = 1;
                    layoutBindings[currentIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    layoutBindings[currentIndex].pImmutableSamplers = nullptr;

                    // Precise in which shader we will use the texture
                    layoutBindings[currentIndex].stageFlags = currentTextureInformations.flags;

                }

                VkDescriptorSetLayoutCreateInfo layoutInfo{};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
                layoutInfo.pBindings = layoutBindings.data();

                if (vkCreateDescriptorSetLayout(device_->get(), &layoutInfo, nullptr, &descriptorSetLayout_) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create a descriptor set !");
                }

            }

            void createPipelineLayout() {

                VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
                pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                pipelineLayoutInfo.setLayoutCount = 1;
                pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_;

                if (pushConstantRange_) {
                    pipelineLayoutInfo.pushConstantRangeCount = 1;    // Optional
                    pipelineLayoutInfo.pPushConstantRanges = &(*pushConstantRange_); // Optional
                }
                else {
                    pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
                    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
                }

                if (vkCreatePipelineLayout(device_->get(), &pipelineLayoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create the pipeline layout !");
                }
            }

            //GPU
            void createDescriptorPool() {

                std::vector<VkDescriptorPoolSize> poolSizes;
                if (nbUniforms_ > 0) {
                    poolSizes.emplace_back();
                    poolSizes.back().type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    poolSizes.back().descriptorCount = static_cast<uint32_t>(nbUniforms_ * nbFrames_);
                }

                if (!textures_.empty()) {
                    poolSizes.emplace_back();
                    poolSizes.back().type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes.back().descriptorCount = static_cast<uint32_t>(textures_.size() * nbFrames_);
                }
                

                VkDescriptorPoolCreateInfo poolInfo{};
                poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                
                poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                poolInfo.pPoolSizes = poolSizes.data();

                //Note: This represent the max values of set that we can allocate.
                //A set is a "set of uniforms" that the shaders will have access to. So here we create 1 set for each frame.
                poolInfo.maxSets = static_cast<uint32_t>(nbFrames_);

                if (vkCreateDescriptorPool(device_->get(), &poolInfo, nullptr, &descriptorPool_) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create descriptor pool !");
                }

            }

            void createDescriptorSets() {

                std::vector<VkDescriptorSetLayout> layouts(nbFrames_, descriptorSetLayout_);

                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = descriptorPool_;
                allocInfo.descriptorSetCount = static_cast<uint32_t>(nbFrames_);
                allocInfo.pSetLayouts = layouts.data();

                descriptorSets_.resize(nbFrames_);
                if (vkAllocateDescriptorSets(device_->get(), &allocInfo, descriptorSets_.data()) != VK_SUCCESS) {
                    throw std::runtime_error("failed to allocate descriptor sets !");
                }

                for (size_t frameIndex = 0; frameIndex < nbFrames_; frameIndex++) {

                    std::vector<VkDescriptorBufferInfo> buffersInfos(nbUniforms_);
                    std::vector<VkDescriptorImageInfo> imagesInfos(textures_.size());

                    std::vector<VkWriteDescriptorSet> writeDescriptors(nbUniforms_ + textures_.size());
                    
                    // Uniform descriptors
                    for (size_t uniformIndex = 0; uniformIndex < nbUniforms_; uniformIndex++) {

                        UniformBufferWrapper const& uniformBufferWrapper = uniformBufferWrappers_[uniformIndex];

                        // VkDescriptorBufferInfo bufferInfo{};
                        buffersInfos[uniformIndex].buffer = uniformBufferWrapper.uniformBuffers[frameIndex];
                        buffersInfos[uniformIndex].offset = 0;
                        buffersInfos[uniformIndex].range = uniformBufferWrapper.informations.bufferSize;

                        // VkWriteDescriptorSet descriptorWrite{};
                        writeDescriptors[uniformIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        writeDescriptors[uniformIndex].dstSet = descriptorSets_[frameIndex];
                        writeDescriptors[uniformIndex].dstBinding = uniformBufferWrapper.informations.binding; //Binding in shader
                        writeDescriptors[uniformIndex].dstArrayElement = 0;

                        writeDescriptors[uniformIndex].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        writeDescriptors[uniformIndex].descriptorCount = 1;

                        writeDescriptors[uniformIndex].pBufferInfo = &buffersInfos[uniformIndex];

                    }

                    // Texture descriptors
                    for (size_t textureIndex = 0; textureIndex < textures_.size(); ++textureIndex) {
                        
                        size_t currentIndex = nbUniforms_ + textureIndex;

                        Texture const& currentTexture = textures_[textureIndex];

                        imagesInfos[textureIndex].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        imagesInfos[textureIndex].imageView = currentTexture.getImageView();
                        imagesInfos[textureIndex].sampler = currentTexture.getSampler();

                        writeDescriptors[currentIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        writeDescriptors[currentIndex].dstSet = descriptorSets_[frameIndex];
                        writeDescriptors[currentIndex].dstBinding = currentTexture.getInformations().binding;
                        writeDescriptors[currentIndex].dstArrayElement = 0;

                        writeDescriptors[currentIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        writeDescriptors[currentIndex].descriptorCount = 1;

                        writeDescriptors[currentIndex].pImageInfo = &imagesInfos[textureIndex];

                    }

                    vkUpdateDescriptorSets(device_->get(), static_cast<uint32_t>(writeDescriptors.size()), writeDescriptors.data(), 0, nullptr);

                }

            }
            /// <-

            inline void recordPushConstant(VkCommandBuffer commandBuffer, void * data, uint32_t dataSize) {
                vkCmdPushConstants(commandBuffer, pipelineLayout_, pushConstantRange_->stageFlags, 0, dataSize, data);
            }

            inline void bind(VkCommandBuffer commandBuffer, uint32_t frameIndex) const {
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, 1, &descriptorSets_[frameIndex], 0, nullptr);
            }

            virtual void updateUniform(size_t uniformIndex, uint32_t frameIndex, void * data, size_t dataSize) const {
                memcpy(uniformBufferWrappers_[uniformIndex].uniformBuffersMapped[frameIndex], data, dataSize);
            }

            inline VkPipelineLayout getPipelineLayout() const {
                return pipelineLayout_;
            }

            inline std::string const& getVertexFilename() const {
                return vertexFilename_;
            }

            inline std::string const& getFragmentFilename() const {
                return fragmentFilename_;
            }

        protected:

            //Vulkan objects
            const Device* device_;

            //Vulkan uniforms objects
            VkDescriptorSetLayout descriptorSetLayout_ = nullptr;
            VkDescriptorPool descriptorPool_ = nullptr;
            std::vector<VkDescriptorSet> descriptorSets_;
            VkPipelineLayout pipelineLayout_ = nullptr;

            //Shader variables
            uint16_t nbFrames_;
            std::string vertexFilename_;
            std::string fragmentFilename_;

            //Push constant memory
            std::optional<VkPushConstantRange> pushConstantRange_;

            //Uniforms memory
            std::vector<UniformBufferWrapper> uniformBufferWrappers_;
            size_t nbUniforms_ = 0;

            //Texture memory
            std::vector<Texture> textures_;

    };

}
