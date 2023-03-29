
#include <Core/Global.hpp>

#include <Generator/VoxelMapGenerator.hpp>

#include <Entity/Player.hpp>
#include <World/World.hpp>
#include <Mesh/Plane.hpp>
#include <Mesh/Voxel.hpp>
#include <Mesh/LightVoxel.hpp>

#define SOLO_CHUNK true
#define NB_CHUNK_PER_AXIS 3


#define CHUNK_SIZE 64 //size_t
#define VOXEL_SIZE 1.0f //float

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
        // mav::LightVoxel sun(&sunShader, &environment, sunMaterial, 50);


        Game(const mav::Window* window) : window_(window), totalElapsedTime_(0),
            player(glm::vec3(0, 0, 0)), generator(0, CHUNK_SIZE, VOXEL_SIZE),
            world(&chunkShader, &environment, CHUNK_SIZE, VOXEL_SIZE),
            selectionVoxel(&selectVoxelShader, &environment, grassMaterial, VOXEL_SIZE),
            sun(&sunShader, &environment, sunMaterial, 50)
        {

            //Init shaders
            #ifndef NDEBUG
                mav::Global::debugShader.load("Shaders/basic_color.vs", "Shaders/sun_color.fs");
            #endif

            chunkShader.load("Shaders/simple_voxel.vs", "Shaders/simple_voxel.fs");
            selectVoxelShader.load("Shaders/basic_color.vs", "Shaders/select_color.fs");
            sunShader.load("Shaders/basic_color.vs", "Shaders/sun_color.fs");

            //Init environment
            environment.sun = &sun;
            environment.camera = player.getCamera();

            //Generate date of voxel faces
            mav::SimpleVoxel::generateGeneralFaces(VOXEL_SIZE);

            //Initialize single voxels
            selectionVoxel.init();
            sun.init();

            //Sun
            //sun.setPosition(0, 200, 0);

        }

        void initChunks() {

            if (SOLO_CHUNK) {
                // world.createChunk(1, 0, 0, &generator);
                // world.createChunk(1, 0, 1, &generator);
                // world.createChunk(1, 0, -1, &generator);
                // world.createChunk(2, 0, 0, &generator);
                world.createChunk(0, 0, 0, &generator);
            }
            else {

                glm::vec3 center(0, 0, 0);
                std::vector<glm::vec3> allCoordinateToRender;

                // The state here notify the loop if it should add 1 to the value or not
                for (int x = -NB_CHUNK_PER_AXIS, xState = 1; x <= NB_CHUNK_PER_AXIS; ++x) {
                    for (int y = -NB_CHUNK_PER_AXIS, yState = 1; y <= NB_CHUNK_PER_AXIS; ++y) {
                        for (int z = -NB_CHUNK_PER_AXIS, zState = 1; z <= NB_CHUNK_PER_AXIS; ++z) {
                            allCoordinateToRender.emplace_back(x, y, z);
                        }
                    }
                }

                // Sort the coordinates vector to have the nearest to the camera first
                std::sort(allCoordinateToRender.begin(), allCoordinateToRender.end(),
                    [&center](glm::vec3 const& coordA, glm::vec3 const& coordB) -> bool {
                        return glm::distance(coordA, center) < glm::distance(coordB, center);
                    }
                );

                for(glm::vec3 const& coordinate : allCoordinateToRender) {
                    world.createChunk(coordinate.x, coordinate.y, coordinate.z, &generator);
                }

            }
        }

        void input(float deltaTime) {

            if (window_->isPressed(GLFW_KEY_W)) {
                player.addVelocity(mav::Direction::FRONT, 75.0f, deltaTime);
                //myCam.ProcessKeyboard(mav::FORWARD, deltaTime);
                // const mav::SimpleVoxel* collisionVoxel = myWorld.getVoxel(myCam.Position.x, myCam.Position.y, myCam.Position.FRONT);
                // if (collisionVoxel) myCam.ProcessKeyboard(mav::BACKWARD, deltaTime);
            }

            if (window_->isPressed(GLFW_KEY_S)) {
                player.addVelocity(mav::Direction::FRONT, -75.0f, deltaTime);
                //myCam.ProcessKeyboard(mav::BACKWARD, deltaTime);
                // const mav::SimpleVoxel* collisionVoxel = myWorld.getVoxel(myCam.Position.x, myCam.Position.y, myCam.Position.z);
                // if (collisionVoxel) myCam.ProcessKeyboard(mav::FORWARD, deltaTime);
            }

            if (window_->isPressed(GLFW_KEY_A)) {
                player.addVelocity(mav::Direction::RIGHT, -75.0f, deltaTime);
                //myCam.ProcessKeyboard(mav::LEFT, deltaTime);
                // const mav::SimpleVoxel* collisionVoxel = myWorld.getVoxel(myCam.Position.x, myCam.Position.y, myCam.Position.z);
                // if (collisionVoxel) myCam.ProcessKeyboard(mav::RIGHT, deltaTime);
            }

            if (window_->isPressed(GLFW_KEY_D)) {
                player.addVelocity(mav::Direction::RIGHT, 75.0f, deltaTime);
                //myCam.ProcessKeyboard(mav::RIGHT, deltaTime);
                // const mav::SimpleVoxel* collisionVoxel = myWorld.getVoxel(myCam.Position.x, myCam.Position.y, myCam.Position.z);
                // if (collisionVoxel) myCam.ProcessKeyboard(mav::LEFT, deltaTime);
            }

        }

        void mouseMoving(double xPos, double yPos){
            player.updateCamera(xPos, yPos);
        }

        void gameLoop(float elapsedTime) {

            //To modify
            totalElapsedTime_ += elapsedTime;
            //

            //Loading phase
            world.updateReadyChunk(4);
            if (!SOLO_CHUNK)
                world.bulkCreateChunk(player.getCamera()->Position, CHUNK_SIZE * 3, false, &generator);

            //Logic phase
            input(elapsedTime);
            player.update(elapsedTime);
            
            const mav::SimpleVoxel* testVoxel = world.getVoxel(player.getCamera()->Position.x, player.getCamera()->Position.y, player.getCamera()->Position.z);
            if (testVoxel) {
                std::cout << "Found VOXEL ! " << player.getCamera()->Position.x << " " << player.getCamera()->Position.y << " " << player.getCamera()->Position.z << std::endl;
            }
            else {
                std::cout << "NO " << player.getCamera()->Position.x << " " << player.getCamera()->Position.y << " " << player.getCamera()->Position.z << std::endl;
            }
            
            const mav::SimpleVoxel* foundVoxel = world.CastRay(player.getCamera()->Position, player.getCamera()->Front);

            sun.setPosition(-800.f, 800.f, 0.f); // Fix position
            // sun.setPosition(0.f, 0.f, 0.0f); // Center position
            // sun.setPosition(cos(tempoTotalTime/5.0f) * 400.f + player.getCamera()->Position.x, sin(tempoTotalTime/5.0f) * 400.f + player.getCamera()->Position.y, 0.0f + player.getCamera()->Position.z); // Simulate a sun rotation
            // sun.setPosition(cos(tempoTotalTime/5.0f) * 400.f, sin(tempoTotalTime/5.0f) * 400.f, 0.0f); // Simulate a sun rotation
            // sun.setPosition(player.getCamera()->Position.x, player.getCamera()->Position.y, player.getCamera()->Position.z); // Light on yourself

            //Drawing phase
            //glClearColor(0.5, 0.5, 0.5, 1);
            glClearColor(0.5294f, 0.8078f, 0.9216f, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // If we previously found a voxel 
            if ( foundVoxel != nullptr) {
                selectionVoxel.setPosition(foundVoxel->getPosition());
                selectionVoxel.updatePosition();
                selectionVoxel.draw();
            }
            
            //myPlane.draw();


            world.draw(player.getCamera()->Position, 130);

            sun.draw();

            // std::cout << "Position = x: " << player.getCamera()->Position.x << " y: " << player.getCamera()->Position.y << " z: " << player.getCamera()->Position.z << std::endl;

        }

    private:

        const mav::Window* window_;

        //Shaders
        mav::Shader chunkShader;
        mav::Shader selectVoxelShader;
        mav::Shader sunShader;


        float totalElapsedTime_;

        //Game objects
        mav::Player player;
        //mav::Camera camera;
        mav::Environment environment;
        ClassicVoxelMapGenerator generator;

        mav::World world;
        mav::Voxel selectionVoxel;
        mav::LightVoxel sun;
        //mav::Plane myPlane(&myShader, &camera, 100);

};
