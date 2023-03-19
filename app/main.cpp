
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

#include <glm/gtx/scalar_multiplication.hpp>

#include <iostream>
#include <chrono>

#include <Generator/VoxelMapGenerator.hpp>

#include <Collision/AABB.hpp>

#include <Entity/Player.hpp>

// Parameters
#define VERT_LEN 700
#define VERT_ROW 700

float voxelSize = 1.0f;
size_t chunkSize = 64;

//TEMPO:
int nbChunkPerAxis = 3;
bool soloChunk = true;


//DECLARE FUNC

void mainGraphicLoop(float elapsedTime);

void input(float deltaTime);



static mav::Window myWindow("Mavoxel", 1920, 1080);

static mav::Shader myChunkShader;
static mav::Shader myVoxelShader;
static mav::Shader sunShader;

static mav::Environment environment;

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


static mav::Player player(glm::vec3(0, 0, 0));
static mav::World myWorld(&myChunkShader, &environment, chunkSize, voxelSize);
static mav::Voxel myVoxel(&myVoxelShader, &environment, grassMaterial, voxelSize);
static mav::LightVoxel sun(&sunShader, &environment, sunMaterial, 50);
//static mav::Plane myPlane(&myShader, &myCam, 100);


///MOUSE
float lastX = mav::Global::width / 2.0f;
float lastY = mav::Global::height / 2.0f;
bool firstMouse = true;



void mouseMoving(double xPos, double yPos){
    // if(firstMouse){
    //     lastX = xPos;
    //     lastY = yPos;
    //     firstMouse = false;
    // }

    // float xOffset = xPos - lastX;
    // float yOffset = lastY - yPos; // inversé car va de bas en haut

    // lastX = xPos;
    // lastY = yPos;

    // myCam.ProcessMouseMovement(xOffset, yOffset);

    player.updateCamera(xPos, yPos);
}


void key_callback(int key, int scancode, int action, int mods){

    if(key == GLFW_KEY_ESCAPE){
        //TODO: etudier la possibiliter de kills les threads avant de tout fermer pour vite les stops
		myWindow.closeWindow();
	}

    if(key == GLFW_KEY_E && action == GLFW_PRESS){
        
    }

}




void input(float deltaTime) {

	if (myWindow.isPressed(GLFW_KEY_W)) {
        player.addVelocity(mav::Direction::FRONT, 75.0f, deltaTime);
        //myCam.ProcessKeyboard(mav::FORWARD, deltaTime);
        // const mav::SimpleVoxel* collisionVoxel = myWorld.getVoxel(myCam.Position.x, myCam.Position.y, myCam.Position.FRONT);
        // if (collisionVoxel) myCam.ProcessKeyboard(mav::BACKWARD, deltaTime);
    }

    if (myWindow.isPressed(GLFW_KEY_S)) {
        player.addVelocity(mav::Direction::FRONT, -75.0f, deltaTime);
        //myCam.ProcessKeyboard(mav::BACKWARD, deltaTime);
        // const mav::SimpleVoxel* collisionVoxel = myWorld.getVoxel(myCam.Position.x, myCam.Position.y, myCam.Position.z);
        // if (collisionVoxel) myCam.ProcessKeyboard(mav::FORWARD, deltaTime);
    }

    if (myWindow.isPressed(GLFW_KEY_A)) {
        player.addVelocity(mav::Direction::RIGHT, -75.0f, deltaTime);
        //myCam.ProcessKeyboard(mav::LEFT, deltaTime);
        // const mav::SimpleVoxel* collisionVoxel = myWorld.getVoxel(myCam.Position.x, myCam.Position.y, myCam.Position.z);
        // if (collisionVoxel) myCam.ProcessKeyboard(mav::RIGHT, deltaTime);
    }

    if (myWindow.isPressed(GLFW_KEY_D)) {
        player.addVelocity(mav::Direction::RIGHT, 75.0f, deltaTime);
        //myCam.ProcessKeyboard(mav::RIGHT, deltaTime);
        // const mav::SimpleVoxel* collisionVoxel = myWorld.getVoxel(myCam.Position.x, myCam.Position.y, myCam.Position.z);
        // if (collisionVoxel) myCam.ProcessKeyboard(mav::LEFT, deltaTime);
    }

}

float tempoTotalTime = 0;
void mainGraphicLoop(float elapsedTime){

    auto test = glm::vec3();

    //To modify
    tempoTotalTime += elapsedTime;
    //

    //Loading phase
    myWorld.updateReadyChunk(4);

    //Logic phase
	input(elapsedTime);
    player.update(elapsedTime);
    const mav::SimpleVoxel* foundVoxel = myWorld.CastRay(player.getCamera()->Position, player.getCamera()->Front);

    sun.setPosition(-800.f, 800.f, 0.f); // Fix position
    // sun.setPosition(0.f, 0.f, 0.0f); // Center position
    // sun.setPosition(cos(tempoTotalTime/5.0f) * 400.f + myCam.Position.x, sin(tempoTotalTime/5.0f) * 400.f + myCam.Position.y, 0.0f + myCam.Position.z); // Simulate a sun rotation
    // sun.setPosition(cos(tempoTotalTime/5.0f) * 400.f, sin(tempoTotalTime/5.0f) * 400.f, 0.0f); // Simulate a sun rotation
    // sun.setPosition(myCam.Position.x, myCam.Position.y, myCam.Position.z); // Light on yourself

    //Drawing phase
    //glClearColor(0.5, 0.5, 0.5, 1);
    glClearColor(0.5294f, 0.8078f, 0.9216f, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    

    // If we previously found a voxel 
    if ( foundVoxel != nullptr) {
        myVoxel.setPosition(foundVoxel->getPosition());
        myVoxel.updatePosition();
        myVoxel.draw();
    }
	
    //myPlane.draw();


    myWorld.drawAll();

    sun.draw();

    // std::cout << "Position = x: " << myCam.Position.x << " y: " << myCam.Position.y << " z: " << myCam.Position.z << std::endl;


}

int main(int argc, char const *argv[]){

    AABB(glm::vec3(10, 5, 3), 10);
    //myWorld.getVoxel(32, 31.4, 31.6);
    //myWorld.CastRay(glm::vec3(0, 0, 0), glm::vec3(1, 2, 0));

	srand(time(NULL));

    #ifndef NDEBUG
        mav::Global::debugShader.load("Shaders/basic_color.vs", "Shaders/sun_color.fs");
    #endif

    //Init shaders
	myChunkShader.load("Shaders/simple_voxel.vs", "Shaders/simple_voxel.fs");
    myVoxelShader.load("Shaders/basic_color.vs", "Shaders/select_color.fs");
	sunShader.load("Shaders/basic_color.vs", "Shaders/sun_color.fs");

    //Init environment
    environment.sun = &sun;
    environment.camera = player.getCamera();

    //Generators
    ClassicVoxelMapGenerator generator(0, chunkSize, voxelSize);


    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();

    //Init world
    mav::SimpleVoxel::generateGeneralFaces(voxelSize);
    

    if  (soloChunk) {
        myWorld.createChunk(1, 0, 0, &generator);
        myWorld.createChunk(1, 0, 1, &generator);
        myWorld.createChunk(1, 0, -1, &generator);
        myWorld.createChunk(2, 0, 0, &generator);
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

    glEnable( GL_BLEND );
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);

    //Setup interface
	myWindow.setMouseCallback(mouseMoving);
	myWindow.setKeyCallback(key_callback);
	myWindow.setGraphicLoop(mainGraphicLoop);

    std::cout << "Starting main loop..." << std::endl;
	myWindow.startLoop();

	return 0;
}
