
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
#include <chrono>

#include <Generator/VoxelMapGenerator.hpp>



// Parameters
#define VERT_LEN 700
#define VERT_ROW 700

float voxelSize = 1.0f;
size_t chunkSize = 64;

//TEMPO:
int nbChunkPerAxis = 3;
bool soloChunk = false;


//DECLARE FUNC

void mainGraphicLoop(float elapsedTime);

void input(float deltaTime);



static mav::Window myWindow("Mavoxel", 1920, 1080);

static mav::Shader myShader;
static mav::Shader sunShader;

static mav::Environment environment;

static mav::Camera myCam(glm::vec3(0, 0, 0));

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

static mav::World myWorld(&myShader, &environment, chunkSize, voxelSize);
//static mav::Voxel myVoxel(&myShader, &environment, grassMaterial, 100);
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

    if(key == GLFW_KEY_ESCAPE){
        //TODO: etudier la possibiliter de kills les threads avant de tout fermer pour vite les stops
		myWindow.closeWindow();
	}

}




void input(float deltaTime){

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

float tempoTotalTime = 0;
void mainGraphicLoop(float elapsedTime){

    //To modify
    tempoTotalTime += elapsedTime;
    //

    //Loading phase
    myWorld.updateReadyChunk(4);

    //Logic phase
	input(elapsedTime);
    
    sun.setPosition(300.f, 400.f, 100.0f); // Fix position
    // sun.setPosition(0.f, 0.f, 0.0f); // Center position
    // sun.setPosition(cos(tempoTotalTime/5.0f) * 400.f + myCam.Position.x, sin(tempoTotalTime/5.0f) * 400.f + myCam.Position.y, 0.0f + myCam.Position.z); // Simulate a sun rotation
    //sun.setPosition(cos(tempoTotalTime/5.0f) * 400.f, sin(tempoTotalTime/5.0f) * 400.f, 0.0f); // Simulate a sun rotation
    sun.setPosition(myCam.Position.x, myCam.Position.y, myCam.Position.z); // Light on yourself

    //Drawing phase
    //glClearColor(0.5, 0.5, 0.5, 1);
    glClearColor(0.5294f, 0.8078f, 0.9216f, 1);
    
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	

	//myVoxel.draw();
    //myPlane.draw();

    myWorld.drawAll();

    sun.draw();

    std::cout << "Position = x: " << myCam.Position.x << " y: " << myCam.Position.y << " z: " << myCam.Position.z << std::endl;

}

int main(int argc, char const *argv[]){

	srand(time(NULL));

    #ifndef NDEBUG
        mav::Global::debugShader.load("Shaders/basic_color.vs", "Shaders/sun_color.fs");
    #endif

    //Init shaders
	myShader.load("Shaders/simple_voxel.vs", "Shaders/simple_voxel.fs");
	sunShader.load("Shaders/basic_color.vs", "Shaders/sun_color.fs");

    //Init environment
    environment.sun = &sun;
    environment.camera = &myCam;

    //Generators
    ClassicVoxelMapGenerator generator(0, chunkSize, voxelSize);


    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();

    //Init world
    mav::SimpleVoxel::generateGeneralFaces(voxelSize);

    if  (soloChunk) {
        myWorld.createChunk(0, 0, 0, &generator);
    }
    else {

        glm::vec3 center(0, 0, 0);
        std::vector<glm::vec3> allCoordinateToRender;

        // The state here notify the loop if it should add 1 to the value or not
        for (int x = 0, xState = 1; x <= nbChunkPerAxis; x = -1 * x + xState, xState = (xState+1)%2) {
            for (int y = 0, yState = 1; y <= nbChunkPerAxis; y = -1 * y + yState, yState = (yState+1)%2) {
                for (int z = 0, zState = 1; z <= nbChunkPerAxis; z = -1 * z + zState, zState = (zState+1)%2) {
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
            myWorld.createChunk(coordinate.x, coordinate.y, coordinate.z, &generator);
        }

    }
    

	// myWorld.createChunk(0, 1, 0, generateHeight_v2);
	// myWorld.createChunk(1, 0, 0, generateHeight_v2);
	// myWorld.createChunk(0, 0, -1, generateHeight_v2);
	// myWorld.createChunk(0, -1, 0, worldGeneratorNoise);
	// myWorld.createChunk(-1, 0, 0, worldGeneratorNoise);

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<float> fsec = end - begin;


    std::cout << "Time difference = " << fsec.count() << "s" << std::endl;


    //myVoxel.init();
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
