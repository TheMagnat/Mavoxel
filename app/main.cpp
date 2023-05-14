
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

const uint16_t MAX_FRAMES_IN_FLIGHT = 2;
const bool DEPTH_CHECK = true;

const uint32_t WIDTH = 1920;
const uint32_t HEIGHT = 1080;


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
