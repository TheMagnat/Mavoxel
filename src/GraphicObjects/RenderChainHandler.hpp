
#pragma once

#include <GraphicObjects/DrawableSingle.hpp>
#include <Filter/FilterRenderer.hpp>

#include <VulkanWrapper/SceneRenderer.hpp>


#include <vector>


namespace mav {

    class RenderChainHandler {
        
        struct DrawableInformation {
            DrawableInformation(mav::DrawableSingle* drawable_p, int rendererInIndex_p, int rendererIndex_p, std::optional<glm::uvec2> outputResolution_p) :
                drawable(drawable_p), rendererInIndex(rendererInIndex_p), rendererIndex(rendererIndex_p), outputResolution(outputResolution_p) {}

            mav::DrawableSingle* drawable;
            int rendererInIndex; //Renderer input
            int rendererIndex; //Renderer output
            std::optional<glm::uvec2> outputResolution;
        };

        public:

            RenderChainHandler(bool generateRenderers = false) : generateRenderers_(generateRenderers) {}

            void addScene(mav::DrawableSingle* newScene, int rendererInIndex, int rendererIndex) {
                scenes_.emplace_back(newScene, rendererInIndex, rendererIndex, std::optional<glm::uvec2>());
            }

            //Automatic index addScene
            void addScene(mav::DrawableSingle* newScene, std::optional<glm::uvec2> outputResolution = std::nullopt) {
                
                int currentScenesSize = scenes_.size();

                //Put the precedent scene in texture
                if (!scenes_.empty()) {
                    scenes_.back().rendererIndex = currentScenesSize - 1;

                    //If generate renderers is true, generate scene renderer to render the precedent scene on it and use it as an input of the new scene
                    if (generateRenderers_) {
                        if (!scenes_.back().outputResolution) throw std::exception("Scene have no resolution but generateRenderers is set to true.");
                        mav::Global::vulkanWrapper->addFilterRenderer(*scenes_.back().outputResolution);
                    }

                }

                
                //Take as input the precedent scene and as output the framebuffer until another scene is added
                scenes_.emplace_back(newScene, currentScenesSize - 1, -1, outputResolution);
            }

            void addRenderer(const vuw::SceneRenderer* renderer) {
                sceneRenderers_.push_back(renderer);
            }

            void addRenderers(std::vector<const vuw::SceneRenderer*> renderers) {
                sceneRenderers_.insert(sceneRenderers_.end(), renderers.begin(), renderers.end());
            }

            void addRenderers(std::vector<vuw::SceneRenderer> const& renderers) {
                std::transform(renderers.cbegin(), renderers.cend(), std::back_inserter(sceneRenderers_), [](vuw::SceneRenderer const& renderer) {
                    return &renderer;
                });
            }

            void initializeScenes() {

                if (generateRenderers_) addRenderers(mav::Global::vulkanWrapper->getFilterRenderers());

                for (DrawableInformation& scene : scenes_) {
                    
                    if (scene.rendererInIndex >= 0) {

                        static_cast<FilterRenderer*>( scene.drawable->getDrawable() )->setTextures(
                            &sceneRenderers_[scene.rendererInIndex]->getTextures(),
                            &sceneRenderers_[scene.rendererInIndex]->getAdditionalTextures()
                        );

                    }

                }

            }

            void initializePipelines() {

                for (DrawableInformation& scene : scenes_) {

                    scene.drawable->initializePipeline(scene.rendererIndex);
                    scene.drawable->initializeVertices();

                }

            }

            void draw() {

                //Drawing phase
                uint32_t currentFrame = mav::Global::vulkanWrapper->getCurrentFrame(); //First get the frame index
                VkCommandBuffer currentCommandBuffer = mav::Global::vulkanWrapper->beginRecordingDraw(); //Then get the command buffer
                if (!currentCommandBuffer) {
                    std::cout << "Can't acquire new image to start recording a draw." << std::endl;
                    return;
                }

                for (DrawableInformation const& scene : scenes_) {
                    
                    int rendererIndex = scene.rendererIndex;

                    if (rendererIndex < 0) mav::Global::vulkanWrapper->beginRecordingCommandBuffer(currentCommandBuffer);
                    else sceneRenderers_[rendererIndex]->beginRecordingCommandBuffer(currentCommandBuffer, currentFrame);

                    scene.drawable->draw(currentCommandBuffer, currentFrame);

                    if (rendererIndex < 0) mav::Global::vulkanWrapper->endRecordingCommandBuffer(currentCommandBuffer);
                    else sceneRenderers_[rendererIndex]->endRecordingCommandBuffer(currentCommandBuffer);

                }

                mav::Global::vulkanWrapper->endRecordingDraw();

            }

        private:

            bool generateRenderers_ = false;

            std::vector<DrawableInformation> scenes_;
            std::vector<const vuw::SceneRenderer*> sceneRenderers_;


    };

}
