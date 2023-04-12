
/*

This main file is only for testing the game engine,
you can use a totally different main file.

*/

#include "Core/Core.hpp"

#include <iostream>
#include <chrono>

#include <Game/Game.hpp>


static mav::Window gameWindow("Mavoxel", 1920, 1080);


int main(int argc, char const *argv[]){

	srand(time(NULL));

    Game game(&gameWindow);
    game.initChunks();

    //GL Setup
    glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);

    glEnable( GL_BLEND );
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);

    //Setup interface
	gameWindow.setMouseCallback([&game](double xPos, double yPos){ game.mouseMoving(xPos, yPos); });
	gameWindow.setKeyCallback([&game](int key, int scancode, int action, int mods){ game.keyCallback(key, scancode, action, mods); });
	gameWindow.setGraphicLoop([&game](float elapsedTime){ game.gameLoop(elapsedTime); });

    std::cout << "Starting main loop..." << std::endl;
	gameWindow.startLoop();

	return 0;
}
