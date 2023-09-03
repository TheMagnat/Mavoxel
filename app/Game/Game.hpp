
#pragma once

#include <Core/Global.hpp>

#include <GraphicObjects/BufferTemplates.hpp>
#include <GraphicObjects/DrawableSingle.hpp>
#include <Scenes/RenderChainHandler.hpp>

#include <Generator/VoxelMapGenerator.hpp>

#include <Entity/Player.hpp>
#include <World/World.hpp>
#include <World/EntityManager.hpp>

#include <Mesh/Voxel.hpp>
#include <Mesh/Face.hpp>
#include <Mesh/LightVoxel.hpp>
#include <Mesh/Lines.hpp>

#include <RayCasting/RayCastingRenderer.hpp>
#include <Filter/FilterRenderer.hpp>

#include <glm/gtx/scalar_multiplication.hpp>

#include <Physics/PhysicSystem.hpp>
#include <Collision/Frustum.hpp>
#include <Helper/FrustumPoints.hpp>
#include <Helper/Benchmark/Profiler.hpp>

#include <Files/FileHandler.hpp>
#include <Files/DataFileConverter.hpp>

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
#define PLAYER_SPEED 25
//#define JUMP_FORCE 4 // Realistic
#define JUMP_FORCE 7
#define PLAYER_MASS 75

//World variables
#define WORLD_GRAVITY_FORCE 9.81

#define GRAVITY_ON false //If we start in free flight or not
#define FREE_FLIGHT_MULTIPLIER 5.0f

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
            rayCastingShader(mav::Global::vulkanWrapper->generateShader("Shaders/only_texPos.vert.spv", "Shaders/RayTracing/ray_tracing.frag.spv")),
            motionBlurFilterShader(mav::Global::vulkanWrapper->generateShader("Shaders/only_texPos.vert.spv", "Shaders/Filter/motionBlur.frag.spv")),
            filterShader(mav::Global::vulkanWrapper->generateShader("Shaders/only_texPos.vert.spv", "Shaders/Filter/filter.frag.spv")),
            aaShader(mav::Global::vulkanWrapper->generateShader("Shaders/only_texPos.vert.spv", "Shaders/Filter/antiAliasing.frag.spv")),

            totalElapsedTime_(0),
            player(glm::vec3(-5, 0, 0), 0.5f * 0.95f, PLAYER_MASS), generator(0, CHUNK_SIZE, VOXEL_SIZE),
            physicSystem(WORLD_GRAVITY_FORCE), freeFlightPhysicSystem(0),
            world(&generator, SVO_DEPTH, VOXEL_SIZE, "demoWorld", false),
            entityManager(&physicSystem),

            sun(&environment, sunMaterial, 1),

            renderChainHandler(true),

            //Ray casting
            RCRenderer(&world, &entityManager, &environment, SVO_DEPTH),
            RCRendererWrapper(&rayCastingShader, &RCRenderer),

            //Filter
            filterRenderer(&environment, nullptr, nullptr),
            filterRendererWrapper(&filterShader, &filterRenderer),

            motionBlurRenderer(&environment, nullptr, nullptr),
            motionBlurRendererWrapper(&motionBlurFilterShader, &motionBlurRenderer),

            aaRenderer(&environment, nullptr, nullptr),
            aaRendererWrapper(&aaShader, &aaRenderer)

        {
            
            float rayTracingResolutionFactor = 0.75;
            glm::uvec2 rayTracingResolution(1920 * rayTracingResolutionFactor, 1080 * rayTracingResolutionFactor);

            //Add to render chain handler scenes to render
            renderChainHandler.addScene(&RCRendererWrapper, mav::SceneType::RAY_TRACING, rayTracingResolution);
            renderChainHandler.addScene(&motionBlurRendererWrapper, mav::SceneType::FILTER, rayTracingResolution);
            renderChainHandler.addScene(&filterRendererWrapper, mav::SceneType::FILTER, glm::uvec2(1920, 1080));
            renderChainHandler.addScene(&aaRendererWrapper, mav::SceneType::FILTER);

            renderChainHandler.initializeScenes();
            renderChainHandler.initializeShaders();
            renderChainHandler.initializePipelines();

            entityManager.addEntity( mav::AABB( glm::vec3(-5, 0, 0), VOXEL_SIZE ) );
            entityManager.addEntity( mav::AABB( glm::vec3(-5, 0, -5), VOXEL_SIZE ) );
            entityManager.addEntity( mav::AABB( glm::vec3(-5, 0, 5), VOXEL_SIZE ) );

            player.setPhysicSystem(freeFlight ? &freeFlightPhysicSystem : &physicSystem);

            //Init environment
            environment.sun = &sun;
            environment.camera = player.getCamera();

            #ifndef NDEBUG
                mav::DebugGlobal::debugEnvironment.sun = &sun;
                mav::DebugGlobal::debugEnvironment.camera = player.getCamera();
            #endif

            //Generate date of voxel faces
            mav::SimpleVoxel::generateGeneralFaces(VOXEL_SIZE);


        }

        void initChunks() {

            if (SOLO_CHUNK) {
                world.loadOrCreateChunk(glm::ivec3(0, -1, -1));
                // world.loadOrCreateChunk(glm::ivec3(0, 0, -1));
                // world.loadOrCreateChunk(glm::ivec3(1, 0, 1));
                // world.loadOrCreateChunk(glm::ivec3(0, 0, -1));
                // world.loadOrCreateChunk(glm::ivec3(0, 0, 1));
                
                // world.loadOrCreateChunk(glm::ivec3(0, -1, -2));

            }
            else {

                glm::vec3 center(0, 0, 0);
                world.bulkCreateChunk(center, NB_CHUNK_PER_AXIS * CHUNK_SIZE * VOXEL_SIZE, true);

            }

        }

        void mouseClickCallback(int button, int action, int mods){

            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

                if (currentlyLookingCollision) {

                    #ifdef TIME
			            Profiler profiler("Click on mouse delete");
		            #endif

                    currentlyLookingCollision->chunk->deleteVoxel(currentlyLookingCollision->position);

                    //Threader les generateVertices
                    for (mav::Chunk* chunkPtr : world.needToRegenerateChunks) {
                        chunkPtr->graphicUpdate();
                    }

                    world.needToRegenerateChunks.clear();

                }

            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
                
                if (currentlyLookingCollision) {

                    #ifdef TIME
			            Profiler profiler("Click on mouse add");
		            #endif

                    glm::ivec3 newVoxelPositionToChunk = currentlyLookingCollision->position;
                    newVoxelPositionToChunk += currentlyLookingCollision->normals.front();

                    currentlyLookingCollision->chunk->addVoxel(newVoxelPositionToChunk, 4);

                    for (mav::Chunk* chunkPtr : world.needToRegenerateChunks) {
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

                freeFlight = !freeFlight;
                // if (freeFlight) player.setPhysicSystem(nullptr);
                // else player.setPhysicSystem(&physicSystem);

                if (freeFlight) {
                    player.setPhysicSystem(&freeFlightPhysicSystem);
                    // physicSystem.setGravityStrength(0);
                    player.velocity = glm::vec3(0.0f);
                }
                else player.setPhysicSystem(&physicSystem);

            }

            if(key == GLFW_KEY_P && action == GLFW_PRESS) {
                Profiler::printProfiled(std::cout);
            }

            if(key == GLFW_KEY_TAB && action == GLFW_PRESS) {
                std::cout << "Camera = x: " << player.getCamera()->Position.x << " y: " << player.getCamera()->Position.y << " z: " << player.getCamera()->Position.z << std::endl;
                std::cout << "FPS: " << (1.0f/(totalElapsedTime_/(float)frameCount)) << std::endl;

                glm::vec3 sunPos = environment.sun->getPosition();

                glm::vec4 viewPosition = environment.camera->GetViewMatrix() * glm::vec4(environment.sun->getPosition(), 1.0);
                glm::vec4 ndcPosition = environment.camera->Projection * viewPosition;

                glm::vec2 screenPosition;
                glm::vec2 viewportSize(1920, 1080);

                screenPosition.x = (ndcPosition.x / ndcPosition.w + 1.0) * 0.5;
                screenPosition.y = (1.0 - ndcPosition.y / ndcPosition.w) * 0.5;

                std::cout << "Screen pos = x: " << screenPosition.x << " y: " << screenPosition.y << " zView: " << viewPosition.z << std::endl;

                if ( currentlyLookingCollision ) {
                    std::cout << "Face = x: " << currentlyLookingCollision->position.x << " y: " << currentlyLookingCollision->position.y << " z: " << currentlyLookingCollision->position.z << std::endl;
                }

                mav::AABB aabb = player.getBoundingBox();
                glm::vec3 maxPoint = aabb.center + aabb.extents;
                glm::vec3 minPoint = aabb.center - aabb.extents;

                std::cout << "AABB Max point = x: " << maxPoint.x << " y: " << maxPoint.y << " z: " << maxPoint.z << std::endl;
                std::cout << "AABB Min point = x: " << minPoint.x << " y: " << minPoint.y << " z: " << minPoint.z << std::endl;

                
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

            if(key == GLFW_KEY_O && action == GLFW_PRESS) {

                using DataType = mav::DataFileConverter::DataType;

                glm::vec3 pos = player.getPosition();
                float& yaw = player.getCamera()->Yaw;
                float& pitch = player.getCamera()->Pitch;

                std::ofstream stream = mav::getWriteFileStream("playerState.mvx");
                mav::DataFileConverter::convertIntoStream(stream, mav::DataFileConverter::DataFileDescription{
                    {DataType::VEC3, &pos},
                    {DataType::FLOAT, &yaw},
                    {DataType::FLOAT, &pitch}
                });

            }

            if(key == GLFW_KEY_I && action == GLFW_PRESS) {

                using DataType = mav::DataFileConverter::DataType;
                
                glm::vec3 vecOut(0);
                float yawOut = 0;
                float pitchOut = 0;

                std::ifstream streamIn = mav::getReadFileStream("playerState.mvx");
                mav::DataFileConverter::convertFromStream(streamIn, mav::DataFileConverter::DataFileDescription{
                    {DataType::VEC3, &vecOut},
                    {DataType::FLOAT, &yawOut},
                    {DataType::FLOAT, &pitchOut}
                });

                player.setState(vecOut, yawOut, pitchOut);

            }

            if(key == GLFW_KEY_E && action == GLFW_PRESS){

                mav::Camera* camera = player.getCamera();

                entityManager.addEntity( mav::AABB( camera->Position + camera->Front * 5 , VOXEL_SIZE * 0.95 * 3 ) );

            }

            if(key == GLFW_KEY_3 && action == GLFW_PRESS){

                mav::Camera* camera = player.getCamera();

                entityManager.back().addVelocity(camera->Front, 14, 1.0);

            }

            if(key == GLFW_KEY_R && action == GLFW_PRESS){

                world.save();
                std::cout << "World saved !" << std::endl;

            }

            if(key == GLFW_KEY_F && action == GLFW_PRESS){

                mav::Camera* camera = player.getCamera();

                camera->Position;

                world.getVoxel(camera->Position.x, camera->Position.y, camera->Position.z);

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

            player.setAcceleration(direction, PLAYER_SPEED * (freeFlight ? FREE_FLIGHT_MULTIPLIER : 1.0f), freeFlight);

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
                world.bulkCreateChunk(player.getCamera()->Position, CHUNK_SIZE * 3, true);
            }
            
            //Logic phase
            input(elapsedTime);

            {            
                using DataType = mav::DataFileConverter::DataType;

                glm::vec3 pos = player.getPosition();
                float& yaw = player.getCamera()->Yaw;
                float& pitch = player.getCamera()->Pitch;

                std::ofstream stream = mav::getWriteFileStream("playerStateAuto.mvx");
                mav::DataFileConverter::convertIntoStream(stream, mav::DataFileConverter::DataFileDescription{
                    {DataType::VEC3, &pos},
                    {DataType::FLOAT, &yaw},
                    {DataType::FLOAT, &pitch}
                });
            }

            //Update player position
            player.update(elapsedTime, world);
            entityManager.updateAll(elapsedTime, world);
            entityManager.updateBuffer();
                        
            sun.setPosition(-200.f, 100.f, 0.f); // Fix position
            // sun.setPosition(player.getCamera()->Position + player.getCamera()->Front * 50); // Front of player
            // sun.setPosition(0.f, 0.f, 0.0f); // Center position
            // sun.setPosition(cos(tempoTotalTime/5.0f) * 400.f + player.getCamera()->Position.x, sin(tempoTotalTime/5.0f) * 400.f + player.getCamera()->Position.y, 0.0f + player.getCamera()->Position.z); // Simulate a sun rotation around you
            // sun.setPosition(cos(totalElapsedTime_/25.0f) * 400.f, sin(totalElapsedTime_/25.0f) * 400.f, 100.0f); // Simulate a sun rotation
            // sun.setPosition(player.getCamera()->Position.x, player.getCamera()->Position.y, player.getCamera()->Position.z); // Light on yourself


            //Start scene rendering into a texture

            //Can be here or before "beginRecordingDraw"
            // updateUniformBuffer(currentFrame, timeFromStart);
            
            // testShader_.recordPushConstant(currentCommandBuffer, &timeFromStart, sizeof(timeFromStart));

            // testGraphicsPipeline_.bind(currentCommandBuffer);
            // testShader_.bind(currentCommandBuffer, currentFrame);
            // vertexData_.bind(currentCommandBuffer);
            // vertexData_.draw(currentCommandBuffer);

            {            
                using DataType = mav::DataFileConverter::DataType;

                glm::vec3 pos = player.getPosition();
                float& yaw = player.getCamera()->Yaw;
                float& pitch = player.getCamera()->Pitch;

                std::ofstream stream = mav::getWriteFileStream("playerStateAutoAfterUpdate.mvx");
                mav::DataFileConverter::convertIntoStream(stream, mav::DataFileConverter::DataFileDescription{
                    {DataType::VEC3, &pos},
                    {DataType::FLOAT, &yaw},
                    {DataType::FLOAT, &pitch}
                });
            }

            // If we find a voxel in front of the user
            currentlyLookingCollision = world.castRay(player.getCamera()->Position, player.getCamera()->Front, 500);
            // currentlyLookingCollision = world.castSVORay(player.getCamera()->Position, player.getCamera()->Front, 500);
            // currentlyLookingCollisionTwo = world.castSVORay(player.getCamera()->Position, player.getCamera()->Front);
            if ( currentlyLookingCollision ) {

                if (currentlyLookingCollision->chunk == nullptr) {
                    std::cout << "Wrong chunk ptr error" << std::endl;
                }
                else if (currentlyLookingCollision->chunk->state != 2) {
                    std::cout << "Chunk wrong state " << currentlyLookingCollision->chunk->state << std::endl;
                }

                collisionInformations = mav::CollisionInformations {
                    currentlyLookingCollision->position,
                    currentlyLookingCollision->chunk->getPosition(),
                    currentlyLookingCollision->normals.front()
                };

                environment.collisionInformations = &collisionInformations;
            }
            else {
                environment.collisionInformations = nullptr;
            }


            //Drawing phase
            renderChainHandler.draw();

            //Debug system, remove when we're sure that the game won't ever crash
            if (!mav::Global::vulkanWrapper->ok) {
                std::cout << "Camera = x: " << player.getCamera()->Position.x << " y: " << player.getCamera()->Position.y << " z: " << player.getCamera()->Position.z << std::endl;
                std::cout << "Position = x: " << player.getPosition().x << " y: " << player.getPosition().y << " z: " << player.getPosition().z << std::endl;
                
                std::cout << "Camera: Front:  x: " << player.getCamera()->Front.x << " y: " << player.getCamera()->Front.y << " z: " << player.getCamera()->Front.z << std::endl;
                std::cout << "Camera: Right:  x: " << player.getCamera()->Right.x << " y: " << player.getCamera()->Right.y << " z: " << player.getCamera()->Right.z << std::endl;
                std::cout << "Camera: Up:  x: " << player.getCamera()->Up.x << " y: " << player.getCamera()->Up.y << " z: " << player.getCamera()->Up.z << std::endl;
            
                std::cout << "Camera: Yaw: " << player.getCamera()->Yaw << " Pitch: " << player.getCamera()->Pitch << std::endl;
            }

            // std::cout << "Camera = x: " << player.getCamera()->Position.x << " y: " << player.getCamera()->Position.y << " z: " << player.getCamera()->Position.z << std::endl;
            // std::cout << "Position = x: " << player.position.x << " y: " << player.position.y << " z: " << player.position.z << std::endl;

        }

    private:

        const vuw::Window* window_;

        ////Shaders
        //Ray-Casting
        vuw::Shader rayCastingShader;

        //Filter
        vuw::Shader filterShader;
        vuw::Shader motionBlurFilterShader;
        vuw::Shader aaShader;

        float totalElapsedTime_;

        //Game objects
        mav::Player player;
        //mav::Camera camera;
        mav::Environment environment;
        ClassicVoxelMapGenerator generator;
        
        //Physics
        mav::PhysicSystem physicSystem;
        mav::PhysicSystem freeFlightPhysicSystem;

        mav::World world;
        mav::EntityManager entityManager;

        mav::LightVoxel sun;

        std::optional<mav::RayCollisionInformations> currentlyLookingCollision;
        mav::CollisionInformations collisionInformations;

        //Render chain
        mav::RenderChainHandler renderChainHandler;

        //Ray Casting
        mav::RayCastingRenderer RCRenderer;
        mav::DrawableSingle RCRendererWrapper;

        //Filter
        mav::FilterRenderer filterRenderer;
        mav::DrawableSingle filterRendererWrapper;

        mav::FilterRenderer motionBlurRenderer;
        mav::DrawableSingle motionBlurRendererWrapper;

        mav::FilterRenderer aaRenderer;
        mav::DrawableSingle aaRendererWrapper;

        //Debug / Benchmark
        size_t frameCount = 0;

        bool freeFlight = !GRAVITY_ON;

};
