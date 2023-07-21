
#pragma once

#include <Core/Global.hpp>

#include <GLObject/BufferTemplates.hpp>
#include <GLObject/DrawableSingle.hpp>

#include <Generator/VoxelMapGenerator.hpp>

#include <Entity/Player.hpp>
#include <World/World.hpp>
#include <Mesh/Voxel.hpp>
#include <Mesh/Face.hpp>
#include <Mesh/LightVoxel.hpp>
#include <Mesh/Lines.hpp>

#include <RayCasting/RayCastingRenderer.hpp>
#include <Filter/FilterRenderer.hpp>

#include <glm/gtx/scalar_multiplication.hpp>

#include <Physics/Gravity.hpp>
#include <Collision/Frustum.hpp>
#include <Helper/FrustumPoints.hpp>
#include <Helper/Benchmark/Profiler.hpp>

#ifndef NDEBUG
#include <Core/DebugGlobal.hpp>
#endif

#define SOLO_CHUNK false
#define GENERATE_CHUNK false
#define NB_CHUNK_PER_AXIS 3

//TODO: Set la render distance ici, et set en global aussi ? pour pouvoir setup la perspective partout en même temps, et utiliser la perspective de la caméra partout.

//SVO_DEPTH is set in the CMAKE
#define VOXEL_SIZE 0.5f //float
#define CHUNK_SIZE (uint32_t)std::pow(2, SVO_DEPTH) //uint32_t
#define RENDER_DISTANCE (CHUNK_SIZE * VOXEL_SIZE) * 4

//Player variables
#define PLAYER_SPEED 12
#define JUMP_FORCE 7

constexpr mav::Material grassMaterial {
    {0.0f, 1.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {0.5f, 0.5f, 0.5f},
    32.0f
};

constexpr mav::Material sunMaterial {
    {0.1f, 0.1f, 0.1f},
    {0.5f, 0.5f, 0.5f},
    {1.0f, 1.0f, 1.0f}
};

class Game {

    public:

        Game(const vuw::Window* window) : window_(window),
            //Shaders
            chunkShader(mav::Global::vulkanWrapper->generateShader("Shaders/simple_voxel.vert.spv", "Shaders/simple_voxel.frag.spv")),
            selectVoxelShader(mav::Global::vulkanWrapper->generateShader("Shaders/basic_color.vert.spv", "Shaders/select_color.frag.spv")),
            whiteShader(mav::Global::vulkanWrapper->generateShader("Shaders/basic_color.vert.spv", "Shaders/sun_color.frag.spv")),
            colorShader(mav::Global::vulkanWrapper->generateShader("Shaders/only_color.vert.spv", "Shaders/only_color.frag.spv")),
            rayCastingShader(mav::Global::vulkanWrapper->generateShader("Shaders/only_texPos.vert.spv", "Shaders/ray_tracing.frag.spv")),
            filterShader(mav::Global::vulkanWrapper->generateShader("Shaders/only_texPos.vert.spv", "Shaders/filter.frag.spv")),

            totalElapsedTime_(0),
            player(glm::vec3(-5, 0, 0), VOXEL_SIZE), generator(0, CHUNK_SIZE, VOXEL_SIZE), gravity(9.81),
            world(&chunkShader, &environment, SVO_DEPTH, VOXEL_SIZE),

            ////Single objects
            //Objects
            selectionFace(&environment, grassMaterial, VOXEL_SIZE),
            sun(&environment, sunMaterial, 1),
            lines(player.getCamera()),

            //Wrappers
            selectionFaceWrapper(&selectVoxelShader, &selectionFace),
            sunWrapper(&whiteShader, &sun),
            linesWrapper(&colorShader, &lines),

            //Ray casting
            RCRenderer(&world, &environment, SVO_DEPTH),
            RCRendererWrapper(&rayCastingShader, &RCRenderer),

            //Filter
            filterRenderer(&mav::Global::vulkanWrapper->getFilterRenderer().getTextures()),
            filterRendererWrapper(&filterShader, &filterRenderer)

        {

            //Init shaders
            #ifndef NDEBUG
                mav::DebugGlobal::debugShader->addUniformBufferObjects({
                    {0, sizeof(ModelViewProjectionObjectNoNormal), VK_SHADER_STAGE_VERTEX_BIT},
                    {1, sizeof(glm::vec3), VK_SHADER_STAGE_FRAGMENT_BIT}
                });
                mav::DebugGlobal::debugShader->generateBindingsAndSets();
            
            #endif

            chunkShader.addUniformBufferObjects({
                {0, sizeof(ModelViewProjectionObject), VK_SHADER_STAGE_VERTEX_BIT},
                {1, sizeof(glm::vec3), VK_SHADER_STAGE_FRAGMENT_BIT},
                {2, sizeof(LightObject), VK_SHADER_STAGE_FRAGMENT_BIT}
            });
            chunkShader.generateBindingsAndSets();

            selectVoxelShader.addUniformBufferObjects({
                {0, sizeof(ModelViewProjectionObject), VK_SHADER_STAGE_VERTEX_BIT},
                {1, sizeof(float), VK_SHADER_STAGE_FRAGMENT_BIT}
            });
            selectVoxelShader.generateBindingsAndSets();

            whiteShader.addUniformBufferObjects({
                {0, sizeof(ModelViewProjectionObject), VK_SHADER_STAGE_VERTEX_BIT}
            });
            whiteShader.generateBindingsAndSets();

            colorShader.addUniformBufferObjects({
                {0, sizeof(ViewProjectionObject), VK_SHADER_STAGE_VERTEX_BIT}
            });
            colorShader.generateBindingsAndSets();

            //Ray Casting
            rayCastingShader.addUniformBufferObjects({
                {0, sizeof(RayCastInformations), VK_SHADER_STAGE_FRAGMENT_BIT},
                {1, sizeof(WorldOctreeInformations), VK_SHADER_STAGE_FRAGMENT_BIT}
            });
            rayCastingShader.addSSBO({
                //Binding, Stage flag, count, ssbo ptr
                2, VK_SHADER_STAGE_FRAGMENT_BIT, RAYTRACING_SVO_SIZE, std::vector<const vuw::SSBO*>(RAYTRACING_SVO_SIZE, nullptr)
            });
            rayCastingShader.addUniformBufferObjects({
                {3, sizeof(int), VK_SHADER_STAGE_FRAGMENT_BIT, RAYTRACING_SVO_SIZE},
            });

            
            rayCastingShader.generateBindingsAndSets();

            //Filters
            vuw::SceneRenderer const& filterSceneRenderer = mav::Global::vulkanWrapper->getFilterRenderer();
            filterShader.addTexture({
                vuw::TextureShaderInformation{filterSceneRenderer.getTextures().front().getInformations(), &filterSceneRenderer.getTextures().front()},
            });

            filterShader.generateBindingsAndSets();

            // Init objects
            world.initializePipeline(true);

            selectionFaceWrapper.initializePipeline(true);
            
            sunWrapper.initializePipeline(true);
            sunWrapper.initializeVertices();

            //Note: Lines use line primitive topology type.
            linesWrapper.initializePipeline(true, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
            
            //Ray casting
            RCRendererWrapper.initializePipeline(true);
            RCRendererWrapper.initializeVertices();

            //Filter
            filterRendererWrapper.initializePipeline(false);
            filterRendererWrapper.initializeVertices();


            player.setGravity(&gravity);

            //Init environment
            environment.sun = &sun;
            environment.camera = player.getCamera();

            #ifndef NDEBUG
                mav::DebugGlobal::debugEnvironment.sun = &sun;
                mav::DebugGlobal::debugEnvironment.camera = player.getCamera();
            #endif

            //Generate date of voxel faces
            mav::SimpleVoxel::generateGeneralFaces(VOXEL_SIZE);

            //Initialize single meshes
            // selectionFace.initialize();
            // lines.initialize();

            //Sun
            //sun.setPosition(0, 200, 0);

        }

        void initChunks() {

            if (SOLO_CHUNK) {
                // world.createChunk(0, 0, -1, &generator);
                // world.createChunk(1, 0, 1, &generator);
                // world.createChunk(0, 0, -1, &generator);
                // world.createChunk(0, 0, 1, &generator);
                // world.createChunk(2, 0, 0, &generator);
                world.createChunk(0, 0, 0, &generator);

                //DEBUG RAY-CAST
                // rayCastingShader.addUniformBufferObjects({
                //     {0, sizeof(RayCastInformations), VK_SHADER_STAGE_FRAGMENT_BIT}
                // });
                // rayCastingShader.addTexture(mav::Global::vulkanWrapper->generateTexture3D(
                //     world.getChunk(0, 0, 0)->voxels_.voxelMatrix,
                //     {1, CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, VK_SHADER_STAGE_FRAGMENT_BIT}
                // ));
                // rayCastingShader.generateBindingsAndSets();
                
                // //Ray casting
                // RCRendererWrapper.initializePipeline();
                // RCRendererWrapper.initializeVertices();

            }
            else {

                glm::vec3 center(0, 0, 0);
                world.bulkCreateChunk(center, NB_CHUNK_PER_AXIS * CHUNK_SIZE * VOXEL_SIZE, true, &generator);

            }

        }

        void mouseClickCallback(int button, int action, int mods){

            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

                if (currentlyLookingFace) {

                    #ifdef TIME
			            Profiler profiler("Click on mouse delete");
		            #endif

                    currentlyLookingFace->chunk->deleteVoxel(currentlyLookingFace->voxel->getChunkPosition());

                    //Threader les generateVertices
                    for (mav::Chunk* chunkPtr : world.needToRegenerateChunks) {
                        chunkPtr->generateVertices();
                        chunkPtr->graphicUpdate();
                    }

                    world.needToRegenerateChunks.clear();

                }

            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
                
                if (currentlyLookingFace) {

                    #ifdef TIME
			            Profiler profiler("Click on mouse add");
		            #endif

                    glm::ivec3 newVoxelPositionToChunk = currentlyLookingFace->voxel->getChunkPosition();
                    newVoxelPositionToChunk += currentlyLookingFace->normal;

                    currentlyLookingFace->chunk->addVoxel(newVoxelPositionToChunk, 4);

                    for (mav::Chunk* chunkPtr : world.needToRegenerateChunks) {
                        chunkPtr->generateVertices();
                        chunkPtr->graphicUpdate();
                    }

                    world.needToRegenerateChunks.clear();

                }

            }

        }

        void keyCallback(int key, int scancode, int action, int mods){

            if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
                window_->closeWindow();
            }

            if(key == GLFW_KEY_G && action == GLFW_PRESS){

                player.freeFlight = !player.freeFlight;

            }

            if(key == GLFW_KEY_P && action == GLFW_PRESS) {
                Profiler::printProfiled(std::cout);
            }

            if(key == GLFW_KEY_TAB && action == GLFW_PRESS) {
                std::cout << "Camera = x: " << player.getCamera()->Position.x << " y: " << player.getCamera()->Position.y << " z: " << player.getCamera()->Position.z << std::endl;
                std::cout << "FPS: " << (1.0f/(totalElapsedTime_/(float)frameCount)) << std::endl;
            }

            if (key == GLFW_KEY_L && action == GLFW_PRESS) {
                int newCursorMode;
                int currentCursorMode = glfwGetInputMode(window_->get(), GLFW_CURSOR);

                if (currentCursorMode == GLFW_CURSOR_DISABLED) {
                    newCursorMode = GLFW_CURSOR_NORMAL;
                }
                else {
                    newCursorMode = GLFW_CURSOR_DISABLED;
                }

                glfwSetInputMode(window_->get(), GLFW_CURSOR, newCursorMode);

            }

            if(key == GLFW_KEY_T && action == GLFW_PRESS) {
                rayCasting = 1 - rayCasting;
            }

            if(key == GLFW_KEY_E && action == GLFW_PRESS){

                static bool first = true;
                // static int test = 1;
                // test++;
                // if(SOLO_CHUNK) {
                //     world.createChunk(0, 0, -test, &generator);
                // }
                
                if( !first ) {
                    //Drawing lines
                    // lines.addPoint(player.position);
                    // lines.graphicUpdate();
                }
                else {

                    //Drawing camera
                    mav::Camera* camera = player.getCamera();
                    glm::vec3 white = glm::vec3(1.0f);
                    glm::vec3 black = glm::vec3(0.0f);

                    //Save true projection
                    glm::mat4 projectionSave = camera->Projection;
                    
                    VkExtent2D const& extent = mav::Global::vulkanWrapper->getExtent();
                    camera->setPerspectiveProjectionMatrix(glm::radians(45.0f), (float)extent.width / (float)extent.height, 0.1f, 100.0f);
                    
                    camera->maintainFrustum = true;
                    camera->updateFrustum();
                    camera->maintainFrustum = false;

                    //Frustum

                    //Show camera frustum on screen
                    lines.clear();

                    //Add the camera axis lines
                    lines.addLine(std::make_pair(camera->Position, camera->Position + camera->Front * 10), glm::vec3(0.0f, 0.0f, 1.0f));
                    lines.addLine(std::make_pair(camera->Position, camera->Position + camera->Up * 10), glm::vec3(0.0f, 1.0f, 0.0f));
                    lines.addLine(std::make_pair(camera->Position, camera->Position + camera->Right * 10), glm::vec3(1.0f, 0.0f, 0.0f));
                    
                    //Generate frustum points and lines and then add them to the lines
                    std::array<glm::vec3, 8> frustumPoints = getFrustumPoints(camera->Projection * camera->GetViewMatrix());
                    for (std::pair<glm::vec3, glm::vec3> const& line : pointsToLines(frustumPoints)) {
                        lines.addLine(line, black);
                    }

                    lines.graphicUpdate();

                    //Put back true projection
                    camera->Projection = std::move(projectionSave);

                }
            }

        }

        void input(float deltaTime) {

            glm::vec3 direction(0.0f);

            if (window_->isPressed(GLFW_KEY_W)) {
                direction.z += 1.0f;
            }

            if (window_->isPressed(GLFW_KEY_S)) {
                direction.z -= 1.0f;
            }

            if (window_->isPressed(GLFW_KEY_A)) {
                direction.x -= 1.0f;
            }

            if (window_->isPressed(GLFW_KEY_D)) {
                direction.x += 1.0f;
            }

            player.addVelocity(direction, PLAYER_SPEED, deltaTime);

            if (window_->isPressed(GLFW_KEY_SPACE)) {
                player.jump(JUMP_FORCE);
            }

        }

        void mouseMoving(double xPos, double yPos){
            player.updateCamera(xPos, yPos);
        }

        void resizeCallback(int width, int height) {
            mav::Global::vulkanWrapper->gotResized();
        }

        void gameLoop(float elapsedTime) {

            // std::cout << "Elapsed time: " << elapsedTime << std::endl;
            // std::cout << "FPS: " << (1.0f/elapsedTime) << std::endl;
            ++frameCount;

            if (elapsedTime > 1.0f) {
                std::cout << "WARNING: Long elapsed time : " << elapsedTime << "s, reduced to 1.0s" << std::endl;
                elapsedTime = 1.0f;
            }

            //To modify
            totalElapsedTime_ += elapsedTime;
            environment.totalElapsedTime = totalElapsedTime_;
            //

            //Loading phase
            world.updateReadyChunk(4);
            if (!SOLO_CHUNK && GENERATE_CHUNK) {
                world.bulkCreateChunk(player.getCamera()->Position, CHUNK_SIZE * 3, true, &generator);
            }
            
            //Logic phase
            input(elapsedTime);

            //Update player position
            player.update(elapsedTime, world);
                        
            sun.setPosition(-200.f, 100.f, 0.f); // Fix position
            // sun.setPosition(player.getCamera()->Position + player.getCamera()->Front * 50); // Front of player
            // sun.setPosition(0.f, 0.f, 0.0f); // Center position
            // sun.setPosition(cos(tempoTotalTime/5.0f) * 400.f + player.getCamera()->Position.x, sin(tempoTotalTime/5.0f) * 400.f + player.getCamera()->Position.y, 0.0f + player.getCamera()->Position.z); // Simulate a sun rotation around you
            // sun.setPosition(cos(totalElapsedTime_/25.0f) * 300.f, sin(totalElapsedTime_/25.0f) * 300.f, 100.0f); // Simulate a sun rotation
            // sun.setPosition(player.getCamera()->Position.x, player.getCamera()->Position.y, player.getCamera()->Position.z); // Light on yourself

            //Drawing phase
            uint32_t currentFrame = mav::Global::vulkanWrapper->getCurrentFrame(); //First get the frame index
            VkCommandBuffer currentCommandBuffer = mav::Global::vulkanWrapper->beginRecordingDraw(); //Then get the command buffer
            if (!currentCommandBuffer) {
                std::cout << "Can't acquire new image to start recording a draw." << std::endl;
                return;
            }

            vuw::SceneRenderer const& filterSceneRenderer = mav::Global::vulkanWrapper->getFilterRenderer();

            //Start scene rendering into a texture
            filterSceneRenderer.beginRecordingCommandBuffer(currentCommandBuffer, currentFrame);

            //Can be here or before "beginRecordingDraw"
            // updateUniformBuffer(currentFrame, timeFromStart);
            
            // testShader_.recordPushConstant(currentCommandBuffer, &timeFromStart, sizeof(timeFromStart));

            // testGraphicsPipeline_.bind(currentCommandBuffer);
            // testShader_.bind(currentCommandBuffer, currentFrame);
            // vertexData_.bind(currentCommandBuffer);
            // vertexData_.draw(currentCommandBuffer);


            // If we find a voxel in front of the user
            currentlyLookingFace = world.castRay(player.getCamera()->Position, player.getCamera()->Front, 500);
            // currentlyLookingFace = world.castSVORay(player.getCamera()->Position, player.getCamera()->Front, 500);
            // currentlyLookingFaceTwo = world.castSVORay(player.getCamera()->Position, player.getCamera()->Front);
            if ( currentlyLookingFace ) {

                // glm::vec3 collisionFaceWorldPosition = glm::vec3(currentlyLookingFace->voxel->getChunkPosition()) + (glm::vec3(currentlyLookingFace->chunk->getPosition()) * std::pow(2, SVO_DEPTH)) - (float)(std::pow(2, SVO_DEPTH) / 2.0);

                collisionInformations = mav::CollisionInformations {
                    currentlyLookingFace->voxel->getChunkPosition(),
                    currentlyLookingFace->chunk->getPosition(),
                    currentlyLookingFace->normal 
                };

                environment.collisionInformations = &collisionInformations;
            }
            else {
                environment.collisionInformations = nullptr;
            }

            if (rayCasting) {
                RCRendererWrapper.draw(currentCommandBuffer, currentFrame);
            }
            else {

                world.drawAll(currentCommandBuffer, currentFrame);

                sunWrapper.draw(currentCommandBuffer, currentFrame);
                linesWrapper.draw(currentCommandBuffer, currentFrame);
                // player.draw();

                if ( currentlyLookingFace ) {
                    float offsetValue = 0.0001f + 0.02f * std::log(glm::distance(player.getCamera()->Position, currentlyLookingFace->points[0])/10.0f + 1);
                    selectionFace.generateVertices(currentlyLookingFace->getOffsettedPoints( offsetValue ));
                    selectionFace.graphicUpdate();
                    selectionFaceWrapper.draw(currentCommandBuffer, currentFrame);
                }

            }

            //End scene rendering into a texture
            filterSceneRenderer.endRecordingCommandBuffer(currentCommandBuffer);

            //Start rendering final scene
            mav::Global::vulkanWrapper->beginRecordingCommandBuffer(currentCommandBuffer);
            
            //TODO: use the filled texture as an input of a shader to use lower resolution scaled on full screen
            
            filterRendererWrapper.draw(currentCommandBuffer, currentFrame);

            //End rendering final scene
            mav::Global::vulkanWrapper->endRecordingCommandBuffer(currentCommandBuffer);

            //End the global rendering processus
            mav::Global::vulkanWrapper->endRecordingDraw();

            // std::cout << "Camera = x: " << player.getCamera()->Position.x << " y: " << player.getCamera()->Position.y << " z: " << player.getCamera()->Position.z << std::endl;
            // std::cout << "Position = x: " << player.position.x << " y: " << player.position.y << " z: " << player.position.z << std::endl;

        }

    private:

        const vuw::Window* window_;

        //Shaders
        // mav::Shader chunkShader;
        vuw::Shader chunkShader;
        vuw::Shader selectVoxelShader;
        vuw::Shader whiteShader;
        vuw::Shader colorShader;

        //Ray-Casting
        vuw::Shader rayCastingShader;

        //Filter
        vuw::Shader filterShader;


        float totalElapsedTime_;

        //Game objects
        mav::Player player;
        //mav::Camera camera;
        mav::Environment environment;
        ClassicVoxelMapGenerator generator;

        //Physics
        mav::Gravity gravity;

        mav::World world;

        mav::Face selectionFace;
        mav::DrawableSingle selectionFaceWrapper;

        mav::LightVoxel sun;
        mav::DrawableSingle sunWrapper;
        
        mav::Lines lines;
        mav::DrawableSingle linesWrapper;

        std::optional<mav::CollisionFace> currentlyLookingFace;
        mav::CollisionInformations collisionInformations;

        //Ray Casting
        mav::RayCastingRenderer RCRenderer;
        mav::DrawableSingle RCRendererWrapper;
        bool rayCasting = true;

        //Filter
        mav::FilterRenderer filterRenderer;
        mav::DrawableSingle filterRendererWrapper;

        //Debug / Benchmark
        size_t frameCount = 0;

};
