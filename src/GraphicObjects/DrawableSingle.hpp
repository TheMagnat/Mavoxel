
#pragma once

#include <GraphicObjects/DrawableContainer.hpp>
#include <GraphicObjects/Drawable.hpp>


namespace mav {

    class DrawableSingle : public DrawableContainer  {

        public:
            DrawableSingle(vuw::Shader* shader, Drawable* drawableItem) : DrawableContainer(shader), drawableItem_(drawableItem) {}

            void initializePipeline(int renderIndex, VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST) {
                DrawableContainer::initializePipeline(drawableItem_->getVertexAttributesSizes(), renderIndex, topology);
            }

            void initializePipelineAntialiasing(VkSampleCountFlagBits msaaSamples, VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST) {
                DrawableContainer::initializePipelineAntialiasing(drawableItem_->getVertexAttributesSizes(), msaaSamples, topology);
            }

            void initializeVertices() {
                drawableItem_->initialize();
            }

            Drawable* getDrawable() {
                return drawableItem_;
            }
            
            //Note: you must call "initializePipeline" and "initializeVertices" before calling draw
            void draw(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
                
                if (drawableItem_->empty()) return;

                drawableItem_->updateShader(shader_, currentFrame);
                DrawableContainer::bind(commandBuffer, currentFrame);
                drawableItem_->draw(commandBuffer);
            }

        private:
            Drawable* drawableItem_;

    };

}
