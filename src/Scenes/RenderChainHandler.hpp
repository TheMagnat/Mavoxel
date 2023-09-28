
#pragma once

#include <Scenes/SceneInformation.hpp>

#include <GraphicObjects/DrawableSingle.hpp>

//Renderer
#include <Filter/FilterRenderer.hpp>
#include <RayCasting/RayCastingRenderer.hpp>


#include <VulkanWrapper/SceneRenderer.hpp>


#include <vector>


namespace mav {

    class RenderChainHandler {

        public:

            RenderChainHandler(bool generateRenderers = false) : generateRenderers_(generateRenderers) {}

            //TODO: adapter au SceneWrapper
            // void addScene(mav::DrawableSingle* newScene, int rendererInIndex, int rendererIndex, SceneType sceneType) {
            //     scenes_.emplace_back(newScene, rendererInIndex, rendererIndex, sceneType, std::nullopt);
            // }

            //Automatic index addScene
            void addScene(SceneWrapper const& sceneWrapper, std::optional<glm::uvec2> outputResolution = std::nullopt) {
                
                int currentScenesSize = scenes_.size();

                //Put the precedent scene in texture
                if (!scenes_.empty()) {
                    scenes_.back().rendererIndex = currentScenesSize - 1;

                    //If generate renderers is true, generate scene renderer to render the precedent scene on it and use it as an input of the new scene
                    if (generateRenderers_) {
                        if (!scenes_.back().outputResolution) throw std::exception("Scene have no resolution but generateRenderers is set to true.");
                        //TODO: ne plus créer les scene après un add scene, mais les accumuler et tout résoudre quand la fonction initializeScenes est appelé pour permettre de ne plus dépendre du type pour savoir si on met les texture en mode copy autorisée mais plutôt de si la texture est copié
                        // mav::Global::vulkanWrapper->addFilterRenderer(*scenes_.back().outputResolution, (int)(scenes_.back().sceneType == SceneType::TAA));
                        mav::Global::vulkanWrapper->addFilterRenderer(*scenes_.back().outputResolution, 1);
                    }

                }
                
                //Take as input the precedent scene and as output the framebuffer until another scene is added
                scenes_.emplace_back(sceneWrapper, currentScenesSize - 1, -1, outputResolution);

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

                for (size_t sceneIndex = 0; sceneIndex < scenes_.size(); ++sceneIndex) {
                    
                    DrawableInformation& scene = scenes_[sceneIndex];

                    //Maybe add this security: if (scene.rendererInIndex >= 0)

                    switch (scene.sceneType) {
                        
                        case SceneType::RAY_TRACING:
                            break;

                        case SceneType::FILTER:
                        {
                            
                            //Generate the texture input of the renderer
                            std::vector<const std::vector<vuw::Texture>*> input;

                            input.push_back(&sceneRenderers_[scene.rendererInIndex]->getTextures());

                            for (std::vector<vuw::Texture> const& textures : sceneRenderers_[scene.rendererInIndex]->getAdditionalTextures()) {
                                input.push_back(&textures);
                            }

                            //If the scene contain owned textures, generate their information
                            std::vector<vuw::Texture::TextureInformations> rendererOwnedTexturesInformations;
                            if (scene.ownedTextures) {

                                for (size_t ownedTextureIndex = 0; ownedTextureIndex < scene.ownedTextures->size(); ++ownedTextureIndex) {
                                    
                                    OwnedTextureInformation const& ownedTextureInformation = scene.ownedTextures->at(ownedTextureIndex);

                                    //IF TYPE == COPY... add new type here

                                    vuw::Texture const& originalTexture = ownedTextureInformation.textureIndex == 0
                                        ? sceneRenderers_[ownedTextureInformation.rendererIndex]->getTextures().front()
                                        : sceneRenderers_[ownedTextureInformation.rendererIndex]->getAdditionalTextures()[ownedTextureInformation.textureIndex - 1].front();

                                    vuw::Texture::TextureInformations originalInformation = originalTexture.getInformations();
                                    rendererOwnedTexturesInformations.emplace_back(
                                        originalInformation.width, originalInformation.height, originalInformation.depth, VK_SHADER_STAGE_FRAGMENT_BIT, originalTexture.getFormat()
                                    );
                                    
                                    toCopy_.emplace_back(sceneIndex, ownedTextureIndex);

                                }

                            }

                            static_cast<FilterRenderer*>( scene.drawable->getDrawable() )->setTextures( input, rendererOwnedTexturesInformations );

                            break;
                        }

                    }

                }

            }

            void initializeShaders() {

                for (DrawableInformation& scene : scenes_) {
                    
                    switch (scene.sceneType) {
                        
                        case SceneType::RAY_TRACING:
                            RayCastingRenderer::initializeShaderLayout(scene.drawable->getShader());
                            break;

                        case SceneType::FILTER:

                            static_cast<FilterRenderer*>( scene.drawable->getDrawable() )->initializeShaderLayout(
                                scene.drawable->getShader()
                            );

                            break;

                    }

                    scene.drawable->getShader()->generateBindingsAndSets();

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

                //Post draw logic
                for (auto [sceneIndex, ownedTextureIndex] : toCopy_) {

                    DrawableInformation const& scene = scenes_[sceneIndex];

                    OwnedTextureInformation const& ownedTextureInformation = scene.ownedTextures->at(ownedTextureIndex);

                    vuw::Texture const& originalTexture = ownedTextureInformation.textureIndex == 0
                        ? sceneRenderers_[ownedTextureInformation.rendererIndex]->getTextures()[currentFrame]
                        : sceneRenderers_[ownedTextureInformation.rendererIndex]->getAdditionalTextures()[ownedTextureInformation.textureIndex - 1][currentFrame];

                    static_cast<FilterRenderer*>( scene.drawable->getDrawable() )->copyIntoOwnedTexture(currentFrame, originalTexture, ownedTextureIndex);

                }

            }

        private:

            bool generateRenderers_ = false;

            std::vector<DrawableInformation> scenes_;
            std::vector<const vuw::SceneRenderer*> sceneRenderers_;

            //Specific to scene type saves :

            //Save the texture to copy after a render. The pair correspond to the scene and the owned texture index.
            std::vector<std::pair<size_t, size_t>> toCopy_;

    };

}
