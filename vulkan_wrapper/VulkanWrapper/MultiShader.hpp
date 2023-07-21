
#pragma once

#include <VulkanWrapper/Shader.hpp>


namespace vuw {

    struct MultiUniformBufferWrapper {

        MultiUniformBufferWrapper(uint16_t nbFramesP, UniformInformations const& uniformInformationP)
            : informations(uniformInformationP), nbFrames(nbFramesP) {}

        void allocateOne(VmaAllocator allocator, VkPhysicalDevice physicalDevice) {

            multiUniformBuffers.emplace_back(nbFrames);
            multiUniformBuffersAllocation.emplace_back(nbFrames);
            multiUniformBuffersMapped.emplace_back(nbFrames);

            for (size_t i = 0; i < nbFrames; i++) {
                VmaAllocationInfo allocationInfo;
                //TODO: pas sur pour les flags
                Buffer::create(allocator, informations.bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, multiUniformBuffers.back()[i], multiUniformBuffersAllocation.back()[i], allocationInfo);

                multiUniformBuffersMapped.back()[i] = allocationInfo.pMappedData;
            }

        }

        void deallocateAll(VmaAllocator allocator) {
            
            for (size_t objectIndex = 0; objectIndex < multiUniformBuffers.size(); ++objectIndex) {
            
                for (size_t i = 0; i < multiUniformBuffers[objectIndex].size(); i++) {
                    vmaDestroyBuffer(allocator, multiUniformBuffers[objectIndex][i], multiUniformBuffersAllocation[objectIndex][i]);
                }

            }

            multiUniformBuffers.clear();
            multiUniformBuffersAllocation.clear();
            multiUniformBuffersMapped.clear();

        }

        UniformInformations informations;
        uint16_t nbFrames;


        //Uniforms memory (Vulkan objects)
        std::vector<std::vector<VkBuffer>> multiUniformBuffers;
        std::vector<std::vector<VmaAllocation>> multiUniformBuffersAllocation;
        std::vector<std::vector<void*>> multiUniformBuffersMapped;

    };

    /**
     * Like Shader but can assign multiple uniforms buffer (to render object using the same shader object without having to create a different shader object/different pipeline)
     * You must call addObject every time you create a new object that will be drawn (you can also call remove to notify the removal of an object)
    */
    class MultiShader : public Shader {

        public:
            MultiShader(const Device* device, uint16_t nbFrames, const std::string& vertexFilename, const std::string& fragmentFilename)
                : Shader(device, nbFrames, vertexFilename, fragmentFilename) {}

            ~MultiShader() {
                clean();
            }

            void clean() {
                
                Shader::clean();

                for (VkDescriptorPool& descriptorPool : multiDescriptorPool_) {
                    vkDestroyDescriptorPool(device_->get(), descriptorPool, nullptr);
                }
                multiDescriptorPool_.clear();

                for (MultiUniformBufferWrapper& multiUniformBufferWrapper : multiUniformBufferWrappers_)
                    multiUniformBufferWrapper.deallocateAll(device_->getAllocator());
                multiUniformBufferWrappers_.clear();

            }

            MultiShader(MultiShader&& toMove) : Shader(std::move(toMove)) {

                multiDescriptorSets_ = std::move(toMove.multiDescriptorSets_);

                multiUniformBufferWrappers_ = std::move(toMove.multiUniformBufferWrappers_);
                toMove.multiUniformBufferWrappers_.clear(); //To prevent deallocation

                nbObjects_ = std::move(toMove.nbObjects_);
                allocatedObjects_ = std::move(toMove.allocatedObjects_);

            }

            MultiShader& operator=(MultiShader&&) = delete;
            MultiShader(const MultiShader&) = delete;
            MultiShader& operator=(const MultiShader&) = delete;

            //Add an object that will use this shader and its uniforms buffers
            void addObject() {
                ++nbObjects_;

                //We allocate enough buffer for the new object
                if (nbObjects_ > allocatedObjects_) {
                    size_t nbToAllocate = nbObjects_ - allocatedObjects_;

                    for (size_t i = 0; i < nbToAllocate; ++i) {

                        for (MultiUniformBufferWrapper& multiUniformBufferWrapper : multiUniformBufferWrappers_) {
                            multiUniformBufferWrapper.allocateOne(device_->getAllocator(), device_->getPhysical());
                        }

                        multiDescriptorPool_.emplace_back();
                        multiDescriptorSets_.emplace_back();
                        createDescriptor(allocatedObjects_ + i);

                    }

                    //Now we allocated for nbObjects_ size.
                    allocatedObjects_ = nbObjects_;

                }

            }

            void removeObject() {
                --nbObjects_;
            }

            //Add uniforms to the shader structure and allocate the memory for them
            void addUniformBufferObjects(std::vector<UniformInformations> const& uniformsInformations) override {

                for (UniformInformations const& uniformInformation : uniformsInformations) {
                    multiUniformBufferWrappers_.emplace_back(nbFrames_, uniformInformation);
                }

                nbUniforms_ += uniformsInformations.size();

            }

            //Never call this
            void generateBindingsAndSets() {
                createLayoutPart();
                //Shader::createDescriptorPool();
            }

            void createLayoutPart() override {
                createDescriptorSetLayout();
                Shader::createPipelineLayout();
            }

            void createDescriptor(size_t objectIndex) {
                createDescriptorPool(objectIndex);
                createDescriptorSets(objectIndex);
            }

            void createDescriptorSetLayout() override {

                std::vector<VkDescriptorSetLayoutBinding> layoutBindings(nbUniforms_ + textureInformations_.size());

                // Set the uniforms layout bindings
                for (size_t i = 0; i < nbUniforms_; ++i) {

                    layoutBindings[i].binding = multiUniformBufferWrappers_[i].informations.binding;
                    layoutBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    layoutBindings[i].descriptorCount = 1; //If array, put more than 1
                    layoutBindings[i].pImmutableSamplers = nullptr; // Optional

                    // Precise in which shader we will use the uniform
                    layoutBindings[i].stageFlags = multiUniformBufferWrappers_[i].informations.flags;

                }

                // Set the SSBO layout bindings
                for (size_t i = 0; i < SSBOInformations_.size(); ++i) {

                    size_t currentIndex = uniformBufferWrappers_.size() + i;

                    SSBOInformation const& currentSSBOInformations = SSBOInformations_[i];
                    
                    layoutBindings[currentIndex].binding = currentSSBOInformations.binding;
                    layoutBindings[currentIndex].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    layoutBindings[currentIndex].descriptorCount = currentSSBOInformations.count;
                    layoutBindings[currentIndex].pImmutableSamplers = nullptr;

                    // Precise in which shader we will use the SSBO
                    layoutBindings[currentIndex].stageFlags = currentSSBOInformations.flags;
                }

                // Set the texture layout bindings
                for (size_t i = 0; i < textureInformations_.size(); ++i) {
                    
                    size_t currentIndex = uniformBufferWrappers_.size() + SSBOInformations_.size() + i;

                    Texture::TextureInformations const& currentTextureInformations = textureInformations_[i].informations;

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


            void createDescriptorPool(size_t objectIndex) {

                std::vector<VkDescriptorPoolSize> poolSizes;
                if (nbUniforms_ > 0) {
                    poolSizes.emplace_back();
                    poolSizes.back().type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    poolSizes.back().descriptorCount = static_cast<uint32_t>(nbUniforms_ * nbFrames_);
                }

                if (!SSBOInformations_.empty()) {
                    poolSizes.emplace_back();
                    poolSizes.back().type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    poolSizes.back().descriptorCount = static_cast<uint32_t>(nbSsbos_ * nbFrames_);
                }

                if (!textureInformations_.empty()) {
                    poolSizes.emplace_back();
                    poolSizes.back().type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSizes.back().descriptorCount = static_cast<uint32_t>(textureInformations_.size() * nbFrames_);
                }
                

                VkDescriptorPoolCreateInfo poolInfo{};
                poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                
                poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
                poolInfo.pPoolSizes = poolSizes.data();

                //Note: This represent the max values of set that we can allocate.
                //A set is a "set of uniforms" that the shaders will have access to. So here we create 1 set for each frame.
                poolInfo.maxSets = static_cast<uint32_t>(nbFrames_);

                if (vkCreateDescriptorPool(device_->get(), &poolInfo, nullptr, &multiDescriptorPool_[objectIndex]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create descriptor pool !");
                }

            }

            //GPU Part
            //Note: this part should be done for each object that share this shader and have different uniforms
            //and can be done after the graphic pipeline, it just need to be done before calling draw and assigning uniform buffers
            void createDescriptorSets(size_t objectIndex) {

                std::vector<VkDescriptorSetLayout> layouts(nbFrames_, descriptorSetLayout_);

                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = multiDescriptorPool_[objectIndex];
                allocInfo.descriptorSetCount = static_cast<uint32_t>(nbFrames_);
                allocInfo.pSetLayouts = layouts.data();

                multiDescriptorSets_[objectIndex].resize(nbFrames_);
                if (vkAllocateDescriptorSets(device_->get(), &allocInfo, multiDescriptorSets_[objectIndex].data()) != VK_SUCCESS) {
                    throw std::runtime_error("failed to allocate descriptor sets !");
                }

                for (size_t frameIndex = 0; frameIndex < nbFrames_; frameIndex++) {

                    std::vector<VkDescriptorBufferInfo> buffersInfos(nbUniforms_);
                    std::vector<VkDescriptorImageInfo> imagesInfos(textureInformations_.size());

                    std::vector<VkWriteDescriptorSet> writeDescriptors(nbUniforms_ + textureInformations_.size());
                    
                    // Uniform descriptors
                    for (size_t uniformIndex = 0; uniformIndex < nbUniforms_; uniformIndex++) {

                        MultiUniformBufferWrapper const& multiUniformBufferWrapper = multiUniformBufferWrappers_[uniformIndex];

                        // VkDescriptorBufferInfo bufferInfo{};
                        buffersInfos[uniformIndex].buffer = multiUniformBufferWrapper.multiUniformBuffers[objectIndex][frameIndex];
                        buffersInfos[uniformIndex].offset = 0;
                        buffersInfos[uniformIndex].range = multiUniformBufferWrapper.informations.bufferSize;

                        // VkWriteDescriptorSet descriptorWrite{};
                        writeDescriptors[uniformIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        writeDescriptors[uniformIndex].dstSet = multiDescriptorSets_[objectIndex][frameIndex];
                        writeDescriptors[uniformIndex].dstBinding = multiUniformBufferWrapper.informations.binding; //Binding in shader
                        writeDescriptors[uniformIndex].dstArrayElement = 0;

                        writeDescriptors[uniformIndex].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        writeDescriptors[uniformIndex].descriptorCount = 1;

                        writeDescriptors[uniformIndex].pBufferInfo = &buffersInfos[uniformIndex];

                    }

                    // Texture descriptors
                    for (size_t textureIndex = 0; textureIndex < textureInformations_.size(); ++textureIndex) {
                        
                        size_t currentIndex = nbUniforms_ + textureIndex;

                        const Texture* currentTexture = textureInformations_[textureIndex].texturePtr;

                        imagesInfos[textureIndex].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        imagesInfos[textureIndex].imageView = currentTexture->getImageView();
                        imagesInfos[textureIndex].sampler = currentTexture->getSampler();

                        writeDescriptors[currentIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        writeDescriptors[currentIndex].dstSet = multiDescriptorSets_[objectIndex][frameIndex];
                        writeDescriptors[currentIndex].dstBinding = currentTexture->getInformations().binding;
                        writeDescriptors[currentIndex].dstArrayElement = 0;

                        writeDescriptors[currentIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        writeDescriptors[currentIndex].descriptorCount = 1;

                        writeDescriptors[currentIndex].pImageInfo = &imagesInfos[textureIndex];

                    }

                    vkUpdateDescriptorSets(device_->get(), static_cast<uint32_t>(writeDescriptors.size()), writeDescriptors.data(), 0, nullptr);

                }

            }
            /// <-


            inline void bind(VkCommandBuffer commandBuffer, uint32_t objectIndex, uint32_t frameIndex) {
                currentObjectIndex_ = objectIndex;
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, 1, &multiDescriptorSets_[currentObjectIndex_][frameIndex], 0, nullptr);
            }

            void updateUniform(size_t uniformIndex, uint32_t frameIndex, void * data, size_t dataSize) const override {
                memcpy(multiUniformBufferWrappers_[uniformIndex].multiUniformBuffersMapped[currentObjectIndex_][frameIndex], data, dataSize);
            }

        private:

            std::vector<VkDescriptorPool> multiDescriptorPool_;
            //1 buffer per frame FOR EACH object
            std::vector<std::vector<VkDescriptorSet>> multiDescriptorSets_;

            //Uniforms memory
            std::vector<MultiUniformBufferWrapper> multiUniformBufferWrappers_;
            //size_t nbUniforms_ = 0;
            size_t nbObjects_ = 0;
            size_t allocatedObjects_ = 0;

            //Save for draw
            uint32_t currentObjectIndex_ = 0;
    };

}
