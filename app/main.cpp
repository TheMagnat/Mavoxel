
/*

This main file is only for testing the game engine,
you can use a totally different main file.

*/

//TODO: Ajouter dans le Cmake un projet app qui buildera ce main qui utilisera la lib Mavoxel. (et renomer le projet src en Mavoxel)

//TESTTT

//#include "PerlinNoise.hpp"
#include <FastNoise/FastNoise.h>

//FIN TESTTT

#include "Core/Global.hpp"
#include "Core/Core.hpp"

#include <Material/Material.hpp>

#include <Environment/Environment.hpp>

#include "GLObject/Shader.hpp"
#include "GLObject/Camera.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtx/scalar_multiplication.hpp>

#include <iostream>
#include <chrono>

#include <Collision/AABB.hpp>

#include <Game/Game.hpp>

// Parameters
#define VERT_LEN 700
#define VERT_ROW 700

// float voxelSize = 1.0f;
// size_t chunkSize = 64;


//DECLARE FUNC

void mainGraphicLoop(float elapsedTime);

void input(float deltaTime);



static mav::Window gameWindow("Mavoxel", 1920/2, 1080/2);



///MOUSE
float lastX = mav::Global::width / 2.0f;
float lastY = mav::Global::height / 2.0f;
bool firstMouse = true;




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
	gameWindow.setMouseCallback([&game](double xpos, double ypos){ game.mouseMoving(xpos, ypos); });
	gameWindow.setKeyCallback(key_callback);
	gameWindow.setGraphicLoop([&game](float elapsedTime){ game.gameLoop(elapsedTime); });

    std::cout << "Starting main loop..." << std::endl;
	gameWindow.startLoop();

	return 0;
}
