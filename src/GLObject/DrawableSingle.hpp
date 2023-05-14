
#include <GLObject/DrawableContainer.hpp>
#include <GLObject/Drawable.hpp>


namespace mav {

    class DrawableSingle : private DrawableContainer  {

        public:
            DrawableSingle(vuw::Shader* shader, Drawable* drawableItem) : DrawableContainer(shader), drawableItem_(drawableItem) {}

            void initializePipeline(VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST) {
                DrawableContainer::initializePipeline(drawableItem_->getVertexAttributesSizes(), topology);
            }

            void initializeVertices() {
                drawableItem_->generateVertices();
                drawableItem_->graphicUpdate();
            }
            
            //Note: you must call "initializePipeline" and "initializeVertices" before calling draw
            void draw(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
                
                if (drawableItem_->empty()) return;

                drawableItem_->updateUniforms(shader_, currentFrame);
                DrawableContainer::bind(commandBuffer, currentFrame);
                drawableItem_->draw(commandBuffer);
            }

        private:
            Drawable* drawableItem_;

    };

}
