
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

#include <Helper/ThreadPool.hpp>

// Parameters
#define VERT_LEN 700
#define VERT_ROW 700

float voxelSize = 10.0f;
size_t chunkSize = 64;

//TEMPO:
int nbChunkPerAxis = 10;


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

    if(key == GLFW_KEY_ESCAPE){
        //TODO: etudier la possibiliter de kills les threads avant de tout fermer pour vite les stops
		myWindow.closeWindow();
	}

}

auto perlinGenerator = FastNoise::New<FastNoise::Perlin>();
// siv::PerlinNoise test(seed);

size_t seed = 0;
inline bool testIfVoxelIn(float x, float y, float z) {
    
    // auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
    // float testValue = fnFractal->GenSingle2D(1.f, 2.f, 10);

    // auto gen = FastNoise::New<FastNoise::Perlin>();

    // float testValue = gen->GenSingle2D(x, z, seed);
    // float testValue_2 = gen->GenSingle2D(x/100.0, z/100.0, seed);
    // float testValue_3 = gen->GenSingle2D(x/100.0, z/100.0, seed) * 32;

    // auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
    auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();
    fnFractal->SetSource(perlinGenerator);
    fnFractal->SetOctaveCount(9);

    float factor = 400.0f;

    return fnFractal->GenSingle3D(x/factor, y/factor, z/factor, seed) >= 0.0f;
}

std::vector<std::vector<std::vector<int>>> generateVoxelMap(float xGlobal, float yGlobal, float zGlobal) {

    std::vector<std::vector<std::vector<int>>> output;


    float positionOffsets = - ((chunkSize) / 2.0f) * voxelSize;

    output.resize(chunkSize);
    for (size_t x = 0; x < chunkSize; ++x) {

        output[x].resize(chunkSize);
        for (int y = chunkSize - 1; y >= 0; --y) {

            output[x][y].resize(chunkSize);
            for (size_t z = 0; z < chunkSize; ++z) {

                float xPos = (x * voxelSize) + positionOffsets + (xGlobal * chunkSize * voxelSize);
                float yPos = (y * voxelSize) + positionOffsets + (yGlobal * chunkSize * voxelSize);
                float zPos = (z * voxelSize) + positionOffsets + (zGlobal * chunkSize * voxelSize);

                //float testValue = perlinGenerator->GenSingle2D(xPos/factor, zPos/factor, seed);
                bool isIn = testIfVoxelIn(xPos, yPos, zPos);

                //if (yPos < testValue * 32.0f)
                if (isIn) {
                    
                    //TODO: Gérer le cas du début
                    if (y == chunkSize - 1) {
                        output[x][y][z] = 1;
                    }
                    else {
                        int upperVoxelId = output[x][y+1][z];
                        output[x][y][z] = upperVoxelId + 1;

                        if(upperVoxelId > 2) output[x][y+1][z] = 2;
                    }

                }
                else {
                    output[x][y][z] = 0;

                    //TODO: Améliorer la boucle entière pour la rendre plus propre/opti
                    if (y != chunkSize - 1) {
                        if(output[x][y+1][z] > 2) output[x][y+1][z] = 2;
                    }

                }

            }
        }
    }

    //Corect last y level
    for (size_t x = 0; x < chunkSize; ++x) {
        for (size_t z = 0; z < chunkSize; ++z) {
            if (output[x][0][z] > 2)
                output[x][0][z] = 2;
        }
    }

    return output;

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
    myWorld.updateReadyChunk(3);

    //Logic phase
	input(elapsedTime);
    
    // sun.setPosition(300.f, 400.f, 100.0f); // Simulate a sun rotation
    // sun.setPosition(0.f, 0.f, 0.0f); // Simulate a sun rotation
    // sun.setPosition(cos(tempoTotalTime/5.0f) * 400.f + myCam.Position.x, sin(tempoTotalTime/5.0f) * 400.f + myCam.Position.y, 0.0f + myCam.Position.z); // Simulate a sun rotation
    // sun.setPosition(cos(tempoTotalTime/5.0f) * 400.f, sin(tempoTotalTime/5.0f) * 400.f, 0.0f); // Simulate a sun rotation
    sun.setPosition(myCam.Position.x, myCam.Position.y, myCam.Position.z); // Light on yourself

    //Drawing phase
    //glClearColor(0.5, 0.5, 0.5, 1);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	

	//myVoxel.draw();
    //myPlane.draw();

    myWorld.drawAll();

    sun.draw();

    //std::cout << "Position = x: " << myCam.Position.x << " y: " << myCam.Position.y << " z: " << myCam.Position.z << std::endl;

}

int main(int argc, char const *argv[]){

	srand(time(NULL));

    //Init shaders
	myShader.load("Shaders/simple_voxel.vs", "Shaders/simple_voxel.fs");
	sunShader.load("Shaders/basic_color.vs", "Shaders/sun_color.fs");

    //Init environment
    environment.sun = &sun;
    environment.camera = &myCam;

    //Generators


    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();

    //Init world
    mav::SimpleVoxel::generateGeneralFaces(voxelSize);

	//myWorld.createChunk(0, 0, 0, generateVoxelMap, testIfVoxelIn);

    for (int x = 0; x <= nbChunkPerAxis; ++x) {
        for (int y = 0; y <= nbChunkPerAxis; ++y) {
            for (int z = 0; z <= nbChunkPerAxis; ++z) {
                myWorld.createChunk(x, y, z, generateVoxelMap, testIfVoxelIn);
            }
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

    //Setup interface
	myWindow.setMouseCallback(mouseMoving);
	myWindow.setKeyCallback(key_callback);
	myWindow.setGraphicLoop(mainGraphicLoop);

    std::cout << "Starting main loop..." << std::endl;
	myWindow.startLoop();

	return 0;
}
