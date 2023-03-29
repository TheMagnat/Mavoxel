
/*

This main file is only for testing the game engine,
you can use a totally different main file.

*/

#include "Core/Core.hpp"

#include <iostream>
#include <chrono>

#include <Collision/AABB.hpp>

#include <Game/Game.hpp>


static mav::Window gameWindow("Mavoxel", 1920/2, 1080/2);


void key_callback(int key, int scancode, int action, int mods){

    if(key == GLFW_KEY_ESCAPE){
        //TODO: etudier la possibiliter de kills les threads avant de tout fermer pour vite les stops
		gameWindow.closeWindow();
	}

    if(key == GLFW_KEY_E && action == GLFW_PRESS){
        
    }

}


int main(int argc, char const *argv[]){

    AABB(glm::vec3(10, 5, 3), 10);

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
	gameWindow.setKeyCallback(key_callback);
	gameWindow.setGraphicLoop([&game](float elapsedTime){ game.gameLoop(elapsedTime); });

    std::cout << "Starting main loop..." << std::endl;
	gameWindow.startLoop();

	return 0;
}
