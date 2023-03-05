
/*

This main file is only for testing the game engine,
you can use a totally different main file.

*/


//TESTTT

#include "PerlinNoise.hpp"

//FIN TESTTT

#include "Core/Global.hpp"
#include "Core/Core.hpp"

#include <Material/Material.hpp>

#include "World/World.hpp"
#include "Mesh/Plane.hpp"
#include <Mesh/Voxel.hpp>
#include <Mesh/LightVoxel.hpp>

#include <Environment/Environment.hpp>

#include "GLObject/Shader.hpp"
#include "GLObject/Camera.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>


#define VERT_LEN 700
#define VERT_ROW 700


//DECLARE FUNC

void mainGraphicLoop(float elapsedTime);

void input(float deltaTime);



static mav::Window myWindow("Mavoxel", 1920, 1080);

static mav::Shader myShader;
static mav::Shader sunShader;

static mav::Environment environment;

static mav::Camera myCam(glm::vec3(0, 0, 10));

static mav::Material grassMaterial {
    {0.0f, 1.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {0.5f, 0.5f, 0.5f},
    32.0f
};

static mav::Material sunMaterial {
    {0.1f, 0.1f, 0.1f},
    {0.5f, 0.5f, 0.5f},
    {1.0f, 1.0f, 1.0f}
};

static mav::World myWorld(&myShader, &environment, 32, 5);
static mav::Voxel myVoxel(&myShader, &environment, grassMaterial, 100);
static mav::LightVoxel sun(&sunShader, &environment, sunMaterial, 50);
//static mav::Plane myPlane(&myShader, &myCam, 100);


///MOUSE
float lastX = mav::Global::width / 2.0f;
float lastY = mav::Global::height / 2.0f;
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


void key_callback(int key, int scancode, int action, int mods){

}


int main(int argc, char const *argv[]){

	srand(time(NULL));

    //Init shaders
	myShader.load("Shaders/simple_voxel.vs", "Shaders/simple_voxel.fs");
	sunShader.load("Shaders/basic_color.vs", "Shaders/sun_color.fs");

    //Init environment
    environment.sun = &sun;
    environment.camera = &myCam;

    //Init world
	myWorld.createChunk(0, 0, 0);

    myVoxel.init();
    sun.init();

    //Sun
    sun.setPosition(0, 200, 0);



    //myPlane.init();
    //myPlane.set(VERT_LEN*VERT_ROW, VERT_LEN, std::vector<float>(VERT_LEN*VERT_ROW, 0));
	//myPlane.update();

    //GL Setup
    glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);

    //Setup interface
	myWindow.setMouseCallback(mouseMoving);
	myWindow.setKeyCallback(key_callback);
	myWindow.setGraphicLoop(mainGraphicLoop);

    std::cout << "Starting main loop..." << std::endl;
	myWindow.startLoop();

	return 0;
}

float tempoTotalTime = 0;
void mainGraphicLoop(float elapsedTime){

    //To modify
    tempoTotalTime += elapsedTime;
    //

    //Logic phase
	input(elapsedTime);
    
    sun.setPosition(0.0f, cos(tempoTotalTime/5.0f) * 400.f, sin(tempoTotalTime/5.0f) * 400.f);

    //Drawing phase
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	

	//myVoxel.draw();
    //myPlane.draw();

    myWorld.drawAll();

    sun.draw();

}

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

}