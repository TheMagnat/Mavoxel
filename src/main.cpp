
/*

This main file is only for testing the game engine,
you can use a totally different main file.

*/


//TESTTT

#include "PerlinNoise.hpp"

//FIN TESTTT


#include "Core/Core.hpp"
//#include "World/World.hpp"
#include "Mesh/Plane.hpp"
#include "GLObject/Shader.hpp"
#include "GLObject/Camera.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>





//DECLARE FUNC
std::vector<float> generateHeight(size_t len, size_t row, size_t seed);


static size_t actualSeed = 1;

//


static mav::Window myWindow("Mavoxel");




//static mav::World myWorld(32);

static mav::Shader myShader;
static mav::Camera myCam(glm::vec3(0, 10, 0));

static mav::Plane myPlane(&myShader, &myCam, 100);

static int octaveNumber = 1;

void input(float deltaTime){

	if(myWindow.isPressed(GLFW_KEY_ESCAPE)){
		myWindow.closeWindow();
	}

	if (myWindow.isPressed(GLFW_KEY_W)){
        myCam.ProcessKeyboard(mav::FORWARD, deltaTime);
    }

    if (myWindow.isPressed(GLFW_KEY_S)){
        myCam.ProcessKeyboard(mav::BACKWARD, deltaTime);
    }

    if (myWindow.isPressed(GLFW_KEY_A)){
        myCam.ProcessKeyboard(mav::LEFT, deltaTime);
    }

    if (myWindow.isPressed(GLFW_KEY_D)){
        myCam.ProcessKeyboard(mav::RIGHT, deltaTime);
    }

    if (myWindow.isPressed(GLFW_KEY_UP)){

		++octaveNumber;

    	std::vector<float> height(generateHeight(700, 700, actualSeed));

        myPlane.set(700*700, 700, height);
		myPlane.update();

    }

    if (myWindow.isPressed(GLFW_KEY_DOWN)){

		--octaveNumber;

    	std::vector<float> height(generateHeight(700, 700, actualSeed));

        myPlane.set(700*700, 700, height);
		myPlane.update();

    }

    if (myWindow.isPressed(GLFW_KEY_0)){
    	actualSeed = rand()%1000000;

    	std::vector<float> height(generateHeight(700, 700, actualSeed));

        myPlane.set(700*700, 700, height);
		myPlane.update();
    }

}


///MOUSE
float lastX = 800 / 2.0f;
float lastY = 600 / 2.0f;
bool firstMouse = true;

void mouseMoving(double xpos, double ypos){
    if(firstMouse){
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // inversé car va de bas en haut

    lastX = xpos;
    lastY = ypos;

    myCam.ProcessMouseMovement(xoffset, yoffset);
}


void mainGraphicLoop(float elapsedTime){
	input(elapsedTime);

	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	myPlane.draw();
	//std::cout << "Je ne fais rien : " << elapsedTime << std::endl;

	//myWorld.drawAll();

}

std::vector<float> generateHeight(size_t len, size_t row, size_t seed){



	siv::PerlinNoise test(seed);

	std::vector<float> heightResult;

	for (size_t i = 0; i < row; ++i) {
		for (size_t j = 0; j < len; ++j) {


			float tempoHeight(test.octaveNoise((j/(double)len)*3, (i/(double)row)*3, octaveNumber));

			heightResult.emplace_back(tempoHeight*30);


		}
	}

	return heightResult;
}



int main(int argc, char const *argv[]){

	srand(time(NULL));

	myShader.load("Shaders/basic.vs", "Shaders/basic.fs", "Shaders/basic.gs");

	myPlane.init();

	//myWorld.createChunk(0, 0);
	// myWorld.createChunk(1, 0);
	// myWorld.createChunk(0, 1);
	// myWorld.createChunk(1, 1);

	std::vector<float> height(generateHeight(1200, 1200, actualSeed));

	myPlane.set(1200*1200, 1200, height);
	myPlane.update();

	myWindow.setMouseCallback(mouseMoving);
	myWindow.setGraphicLoop(mainGraphicLoop);

	myWindow.startLoop();

	return 0;
}