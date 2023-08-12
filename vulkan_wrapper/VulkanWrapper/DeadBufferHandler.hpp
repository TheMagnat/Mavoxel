
#pragma once

#include <vulkan/vulkan.h>
#include <VulkanWrapper/SafeBufferWrapper.hpp>
#include <VulkanWrapper/Device.hpp>
#include <VulkanWrapper/SynchronisationObjects.hpp>

#include <list>
#include <algorithm>

namespace vuw {
        
    class DeadBufferHandler {

        public:
            DeadBufferHandler(uint16_t framesInFlight, const Device* device, const SynchronisationObjects* syncObjs) : framesInFlight_(framesInFlight), device_(device), syncObjs_(syncObjs) {}

            DeadBufferHandler(DeadBufferHandler&&) = delete; //TODO: Declarer un move constructor
            DeadBufferHandler& operator=(DeadBufferHandler&&) = delete;

            DeadBufferHandler(const DeadBufferHandler&) = delete;
            DeadBufferHandler& operator=(const DeadBufferHandler&) = delete;

            //TODO: Ajouter une fonction qui va venir vérifier l'état des buffers a delete et les delete si besoin (en les virant de la liste)

            void addBufferToDeleteQueue(VkBuffer buffer, VmaAllocation allocation) {
                
                std::vector<bool> bufferState_(framesInFlight_, false);
                uint8_t bufferStateTrueNb_ = 0;

                for (size_t i = 0; i < framesInFlight_; ++i) {

                    VkFence const& fence = syncObjs_->inFlightFences[i];

                    if (vkGetFenceStatus(device_->get(), fence) == VK_SUCCESS) {
                        bufferState_[i] = true;
                        ++bufferStateTrueNb_;
                    }

                }

                //Add to delete queue if at lease one frame is currently rendering
                if (bufferStateTrueNb_ < framesInFlight_) deleteQueue_.emplace_front(device_->getAllocator(), buffer, allocation, bufferState_, bufferStateTrueNb_);
                else vmaDestroyBuffer(device_->getAllocator(), buffer, allocation);

            }

            void notifyFrameFinished(uint32_t finishedFrame) {
                
                if (deleteQueue_.empty()) return;

                deleteQueue_.remove_if([this, finishedFrame](SafeBufferWrapper& safeBufferWrapper){
                    
                    //Verify if the current frame was already notified, if not, add it to the total number of finished frame
                    if (!safeBufferWrapper.bufferState_[finishedFrame]) {
                        safeBufferWrapper.bufferState_[finishedFrame] = true;
                        ++safeBufferWrapper.bufferStateTrueNb_;
                        return safeBufferWrapper.bufferStateTrueNb_ >= framesInFlight_; //If true, all state are at true and it mean the buffer is ready to be freed
                    }

                    return false;

                });

            }

        private:
            uint16_t framesInFlight_;

            const Device* device_;
            const SynchronisationObjects* syncObjs_;

            //Delete queue
            std::list<SafeBufferWrapper> deleteQueue_;
    };

}
