
#pragma once

#include <Core/Global.hpp>

#include <Generator/VoxelMapGenerator.hpp>

#include <Entity/Player.hpp>
#include <World/World.hpp>
#include <Mesh/Plane.hpp>
#include <Mesh/Voxel.hpp>
#include <Mesh/Face.hpp>
#include <Mesh/LightVoxel.hpp>
#include <Mesh/Lines.hpp>

#include <glm/gtx/scalar_multiplication.hpp>

#include <Physics/Gravity.hpp>
#include <Collision/Frustum.hpp>
#include <Helper/FrustumPoints.hpp>
#include <Helper/Benchmark/Profiler.hpp>

#define SOLO_CHUNK false
#define GENERATE_CHUNK false
#define NB_CHUNK_PER_AXIS 4

//TODO: Set la render distance ici, et set en global aussi ? pour pouvoir setup la perspective partout en même temps, et utilisert la perspective de la caméra partout.
#define CHUNK_SIZE 32 //size_t
#define VOXEL_SIZE 0.5f //float
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

        // mav::World world();
        // mav::Voxel selectionVoxel(&selectVoxelShader, &environment, grassMaterial, VOXEL_SIZE);
        // mav::LightVoxel sun(&whiteShader, &environment, sunMaterial, 50);

        Game(const mav::Window* window) : window_(window), totalElapsedTime_(0),
            player(glm::vec3(-5, 0, 0), VOXEL_SIZE), generator(0, CHUNK_SIZE, VOXEL_SIZE), gravity(9.81),
            world(&chunkShader, &environment, CHUNK_SIZE, VOXEL_SIZE),
            selectionFace(&selectVoxelShader, &environment, grassMaterial, VOXEL_SIZE),
            sun(&whiteShader, &environment, sunMaterial, 50),
            lines(&colorShader, player.getCamera())
        {

            //Init shaders
            #ifndef NDEBUG
                mav::Global::debugShader.load("Shaders/only_color_uni.vs", "Shaders/only_color_uni.fs");
            #endif

            chunkShader.load("Shaders/simple_voxel.vs", "Shaders/simple_voxel.fs");
            selectVoxelShader.load("Shaders/basic_color.vs", "Shaders/select_color.fs");
            whiteShader.load("Shaders/basic_color.vs", "Shaders/sun_color.fs");
            colorShader.load("Shaders/only_color.vs", "Shaders/only_color.fs");

            player.setGravity(&gravity);

            //Init environment
            environment.sun = &sun;
            environment.camera = player.getCamera();

            #ifndef NDEBUG
                mav::Global::debugEnvironment.sun = &sun;
                mav::Global::debugEnvironment.camera = player.getCamera();
            #endif

            //Generate date of voxel faces
            mav::SimpleVoxel::generateGeneralFaces(VOXEL_SIZE);

            //Initialize single meshes
            selectionFace.initialize();
            lines.initialize();

            sun.initialize(true);

            //Sun
            //sun.setPosition(0, 200, 0);

        }

        void initChunks() {

            if (SOLO_CHUNK) {
                //world.createChunk(1, 0, 0, &generator);
                // world.createChunk(1, 0, 1, &generator);
                //world.createChunk(1, 0, -1, &generator);
                // world.createChunk(2, 0, 0, &generator);
                world.createChunk(0, 0, 0, &generator);
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

                    currentlyLookingFace->chunk->addVoxel(newVoxelPositionToChunk, 1);

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
                    
                    camera->setPerspectiveProjectionMatrix(glm::radians(45.0f), (float)mav::Global::width / (float)mav::Global::height, 0.1f, 100.0f);
                    
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

        void gameLoop(float elapsedTime) {
            
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
                        
            sun.setPosition(-800.f, 800.f, 0.f); // Fix position
            // sun.setPosition(0.f, 0.f, 0.0f); // Center position
            // sun.setPosition(cos(tempoTotalTime/5.0f) * 400.f + player.getCamera()->Position.x, sin(tempoTotalTime/5.0f) * 400.f + player.getCamera()->Position.y, 0.0f + player.getCamera()->Position.z); // Simulate a sun rotation around you
            // sun.setPosition(cos(totalElapsedTime_/5.0f) * 400.f, sin(totalElapsedTime_/5.0f) * 400.f, 0.0f); // Simulate a sun rotation
            // sun.setPosition(player.getCamera()->Position.x, player.getCamera()->Position.y, player.getCamera()->Position.z); // Light on yourself

            //Drawing phase
            //glClearColor(0.5, 0.5, 0.5, 1);
            glClearColor(0.5294f, 0.8078f, 0.9216f, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //Draw non transparent objects
            glDisable( GL_BLEND );

            //Compare both draw perf
            world.drawAll();
            //world.draw(player.getCamera()->Position, RENDER_DISTANCE);

            sun.draw();
            lines.draw();
            player.draw();
            

            //Draw transparent objects
            glEnable( GL_BLEND );

            // If we find a voxel in front of the user
            currentlyLookingFace = world.castRay(player.getCamera()->Position, player.getCamera()->Front);
            if ( currentlyLookingFace ) {
                float offsetValue = 0.0001f + 0.02f * std::log(glm::distance(player.getCamera()->Position, currentlyLookingFace->points[0])/10.0f + 1);
                selectionFace.generateVertices(currentlyLookingFace->getOffsettedPoints( offsetValue ));
                selectionFace.graphicUpdate();
                selectionFace.draw();
            }

            // std::cout << "Camera = x: " << player.getCamera()->Position.x << " y: " << player.getCamera()->Position.y << " z: " << player.getCamera()->Position.z << std::endl;
            // std::cout << "Position = x: " << player.position.x << " y: " << player.position.y << " z: " << player.position.z << std::endl;

        }

    private:

        const mav::Window* window_;

        //Shaders
        mav::Shader chunkShader;
        mav::Shader selectVoxelShader;
        mav::Shader whiteShader;
        mav::Shader colorShader;


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
        mav::LightVoxel sun;
        mav::Lines lines;

        std::optional<mav::CollisionFace> currentlyLookingFace;

};
