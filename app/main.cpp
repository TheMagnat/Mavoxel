
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

const uint32_t WIDTH = 1920;
const uint32_t HEIGHT = 1080;

/*
int main(int argc, char const *argv[]){

	srand(time(NULL));

    vuw::Window gameWindow("Mavoxel", WIDTH, HEIGHT);
    vuw::VulkanWrapper vulkanWrapper(gameWindow.get(), MAX_FRAMES_IN_FLIGHT, DEPTH_CHECK);

    //Mendatory : Fill global values (for release and debug)
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
*/

//Octree main test
int main(int argc, char const *argv[]){

    SparseVoxelOctree tree(2);
    
    auto valueTest2 = tree.get({3, 3, 3});

    tree.set({3, 3, 3}, 11);
    tree.set({2, 3, 3}, 11);
    tree.set({3, 2, 3}, 11);
    tree.set({2, 2, 3}, 11);
    tree.set({3, 3, 2}, 11);
    tree.set({2, 3, 2}, 11);
    tree.set({3, 2, 2}, 11);
    tree.set({2, 2, 2}, 11);

    tree.set({2, 2, 2}, 11);

    tree.set({3, 3, 3}, 0);
    tree.set({2, 3, 3}, 0);
    tree.set({3, 2, 3}, 0);
    tree.set({2, 2, 3}, 0);
    tree.set({3, 3, 2}, 0);
    tree.set({2, 3, 2}, 0);
    tree.set({3, 2, 2}, 0);
    tree.set({2, 2, 2}, 0);

    auto valueTest = tree.get({3, 3, 3});

    // tree.set({0, 0, 0}, 22);

    tree.set({3, 2, 2}, 33);

    auto value = tree.get({3, 0, 1}); //0
    auto value2 = tree.get({3, 0, 2}); //0

    auto value3 = tree.get({3, 3, 3}); //11
    auto value4 = tree.get({3, 0, 2}); //0
    auto value5 = tree.get({3, 0, 2}); //0
    auto value6 = tree.get({3, 2, 2}); //33


    return 0;
}