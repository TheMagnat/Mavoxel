
/*

This main file is only for testing the game engine,
you can use a totally different main file.

*/

#include <VulkanWrapper/Window.hpp>
#include <VulkanWrapper/VulkanWrapper.hpp>

#include <iostream>
#include <chrono>

#include <Core/Global.hpp>
#include <Core/DebugGlobal.hpp>
#include <Game/Game.hpp>

#ifndef NDEBUG
#include <Core/DebugGlobal.hpp>
#endif

//Test
#include <Octree/SparseVoxelOctree.hpp>

const uint16_t MAX_FRAMES_IN_FLIGHT = 2;
const bool DEPTH_CHECK = true;
const bool VALIDATION_LAYER = true;

const uint32_t WIDTH = 1920;
const uint32_t HEIGHT = 1080;


int main(int argc, char const *argv[]){

	srand(time(NULL));

    vuw::Window gameWindow("Mavoxel", WIDTH, HEIGHT);
    vuw::VulkanWrapper vulkanWrapper(gameWindow.get(), MAX_FRAMES_IN_FLIGHT, DEPTH_CHECK, VALIDATION_LAYER);

    //Mandatory : Fill global values (for release and debug)
    mav::Global::vulkanWrapper = &vulkanWrapper;
    #ifndef NDEBUG
    mav::DebugGlobal::debugShader = std::make_unique<vuw::MultiShader>(mav::Global::vulkanWrapper->generateMultiShader("Shaders/only_color_uni.vert.spv", "Shaders/only_color_uni.frag.spv"));
    #endif


    Game game(&gameWindow);
    game.initChunks();

    //GL Setup
    // glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);

    // glEnable( GL_BLEND );
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // glEnable(GL_DEPTH_TEST);

    //Setup interface
	gameWindow.setMouseCallback([&game](double xPos, double yPos){ game.mouseMoving(xPos, yPos); });
	gameWindow.setMouseClickCallback([&game](int button, int action, int mods){ game.mouseClickCallback(button, action, mods); });
	gameWindow.setKeyCallback([&game](int key, int scancode, int action, int mods){ game.keyCallback(key, scancode, action, mods); });
	gameWindow.setResizeCallback([&game](int width, int height){ game.resizeCallback(width, height); });
	gameWindow.setGraphicLoop([&game](float elapsedTime){ game.gameLoop(elapsedTime); });


    std::cout << "Starting main loop..." << std::endl;

	gameWindow.startLoop();
    vulkanWrapper.waitIdle();

    //If debug, we need to free first
    #ifndef NDEBUG
    mav::DebugGlobal::debugShader->clean();
    #endif

	return 0;
}

/*

//Octree main test
#include <unordered_map>

namespace std {
    // Define a hash function for ChunkCoordinates
    template <>
    struct hash<glm::uvec3> {
        size_t operator()(const glm::uvec3& coords) const {
            constexpr int PRIME_1 = 73856093;
            constexpr int PRIME_2 = 19349663;
            constexpr int PRIME_3 = 83492791;
            
            // Map coordinates to a smaller range
            int x = coords.x % PRIME_1;
            int y = coords.y % PRIME_2;
            int z = coords.z % PRIME_3;
            
            // Hash the coordinates
            std::size_t x_hash = std::hash<int>()(x);
            std::size_t y_hash = std::hash<int>()(y);
            std::size_t z_hash = std::hash<int>()(z);
            
            // Combine the hashes
            return x_hash ^ (y_hash << 1) ^ (z_hash << 2);
        }
    };

}


int main(int argc, char const *argv[]){

    mav::SparseVoxelOctree tree(5);

    size_t maxLen = tree.getLen();
    
    std::unordered_map<glm::uvec3, uint32_t> testPositions;

    size_t nbPositionsToTest = 5000;

    for(size_t i = 0; i < nbPositionsToTest; ++i) {
        
        int randomX = rand() % maxLen;
        int randomY = rand() % maxLen;
        int randomZ = rand() % maxLen;

        int randomValue = rand() % 5;

        glm::uvec3 newPos(randomX, randomY, randomZ);

        testPositions[newPos] = randomValue;

        tree.set(newPos, randomValue);

    }

    for (auto const& checker : testPositions) {

        if( tree.get(checker.first).first != checker.second ) {
            std::cout << "Error" << std::endl;
        }

    }

    //Ray casting
    glm::vec3 position = glm::vec3(0.5, 0.5, 0.5);
    glm::vec3 direction = glm::vec3(1, 1, 0);
    float maxDistance = 100.0f;

    //Prétraitement a faire dans la fonction global de castRay (dans World)
    float voxelSize_ = 1;

    //Put the position and distance to the world size. The result voxel must be *= voxelSize_
    position /= voxelSize_;
    maxDistance /= voxelSize_;

    auto rez = tree.castRay(position, direction, maxDistance);

    switch (rez.first) {

        case -1:
            position.x -= maxLen; 
            break;

        default:
            break;

    }

    auto rez2 = tree.castRay(position, glm::vec3(1, 1, 0), 100);

    return 0;
}
*/