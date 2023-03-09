
/*

This main file is only for testing the game engine,
you can use a totally different main file.

*/


//TESTTT

#include "PerlinNoise.hpp"

//FIN TESTTT

#include "Core/Global.hpp"

#include "Core/Core.hpp"
//#include "World/World.hpp"
#include "Mesh/Plane.hpp"
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

std::vector<float> generateHeight(size_t len, size_t row, size_t seed);
std::vector<float> cubeGenerateHeight(size_t len, size_t row, size_t seed);
std::vector<float> coneGenerateHeight(size_t len, size_t row, size_t seed);


static size_t actualSeed = 1;
static double noiseSize = 3;

//


static mav::Window myWindow("Mavoxel", 1920, 1080);

//INTERPOLATE

static bool water = false;
static int mode = 0;

static float actualDelta = 0.0;
static bool isInterpolate = false;
static std::vector<float> oldValue;
static std::vector<float> newValue;


///


//static mav::World myWorld(32);

static mav::Shader myShader;
static mav::Camera myCam(glm::vec3(0, 20, 0));

static mav::Plane myPlane(&myShader, &myCam, 100);

static int octaveNumber = 0;

static float speed = 1.0f;

static float maxHeight = 30.0f;


std::vector<float> interpolateVector(std::vector<float> const& first, std::vector<float> const& second, float delta){

	std::vector<float> result;


	for(size_t i(0); i < first.size(); ++i){
		result.emplace_back(first[i] * (1.0f - delta) + second[i] * delta);
	}


	return result;

}

std::vector<float> cosInterpolateVector(std::vector<float> const& first, std::vector<float> const& second, float delta){

	std::vector<float> result;

	float cosDelta;

	cosDelta = (1.0f - cos(delta * 3.1415926f)) / 2.0f;


	for(size_t i(0); i < first.size(); ++i){
		result.emplace_back(first[i] * (1.0f - cosDelta) + second[i] * cosDelta);
	}


	return result;

}



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

void startInterpolate(){

	isInterpolate = true;
	actualDelta = 0.0f;

	oldValue = newValue;

	if(mode == 0){
		newValue = generateHeight(VERT_LEN, VERT_ROW, actualSeed);
		
	}
	else if(mode == 1){
		newValue = cubeGenerateHeight(VERT_LEN, VERT_ROW, actualSeed);
	}
	else if(mode == 2){
		newValue = coneGenerateHeight(VERT_LEN, VERT_ROW, actualSeed);
	}
	

}

void key_callback(int key, int scancode, int action, int mods){

	if (key == GLFW_KEY_UP && action == GLFW_PRESS){

		++octaveNumber;

    	startInterpolate();

    }

    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS){

		if(--octaveNumber < 0){
			octaveNumber = 0;
		}
		else{
			startInterpolate();	
		}

    	

    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS){
    	
    	actualSeed = rand()%1000000;

    	startInterpolate();

    }

	if (key == GLFW_KEY_0 && action == GLFW_PRESS){
    	
    	water = 1 - water;

    	if(water){

    		myShader.setFloat("water",  1.0f);

    		speed = 0.5f;
    		maxHeight = 10.0f;

    		actualSeed = rand()%1000000;
    		startInterpolate();
    	}
    	else{
    		myShader.setFloat("water",  0.0f);
    		speed = 1.0f;
    		maxHeight = 30.0f;

    		startInterpolate();
    	}	

    }

    if (key == GLFW_KEY_1 && action == GLFW_PRESS){

		mode = 0;

    	startInterpolate();

    }

	if (key == GLFW_KEY_2 && action == GLFW_PRESS){

		mode = 1;

    	startInterpolate();

    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS){

		mode = 2;

    	startInterpolate();

    }

}


std::vector<float> cubeGenerateHeight(size_t len, size_t row, size_t seed){



	siv::PerlinNoise test(seed);

	std::vector<float> heightResult;

	for (size_t i = 0; i < row; ++i) {
		for (size_t j = 0; j < len; ++j) {

			double x((j/(double)len)*noiseSize);
			double y((i/(double)row)*noiseSize);

			x -= fmod(x, 0.05);
			y -= fmod(y, 0.05);


			double tempoHeight(test.octaveNoise(x, y, octaveNumber));

			tempoHeight = tempoHeight*maxHeight;

			tempoHeight -= fmod(tempoHeight, 1.25);


			heightResult.emplace_back(tempoHeight);


		}
	}

	return heightResult;
}

std::vector<float> coneGenerateHeight(size_t len, size_t row, size_t seed){



	siv::PerlinNoise test(seed);

	std::vector<float> heightResult;

	for (size_t i = 0; i < row; ++i) {
		for (size_t j = 0; j < len; ++j) {

			double x((j/(double)len)*noiseSize);
			double y((i/(double)row)*noiseSize);

			double tempoHeight(test.octaveNoise(x, y, octaveNumber));

			tempoHeight = tempoHeight*maxHeight;

			tempoHeight -= fmod(tempoHeight, 1.0);


			heightResult.emplace_back(tempoHeight);


		}
	}

	return heightResult;
}


std::vector<float> generateHeight(size_t len, size_t row, size_t seed){



	siv::PerlinNoise test(seed);

	std::vector<float> heightResult;

	for (size_t i = 0; i < row; ++i) {
		for (size_t j = 0; j < len; ++j) {


			float tempoHeight(test.octaveNoise((j/(double)len)*noiseSize, (i/(double)row)*noiseSize, octaveNumber));

			heightResult.emplace_back(tempoHeight*maxHeight);


		}
	}

	return heightResult;
}



int main(int argc, char const *argv[]){

	srand(time(NULL));

	myShader.load("Shaders/basic.vs", "Shaders/basic.fs", "Shaders/basic.gs");

	myShader.setFloat("water",  0.0f);

	myPlane.init();

	//myWorld.createChunk(0, 0);
	// myWorld.createChunk(1, 0);
	// myWorld.createChunk(0, 1);
	// myWorld.createChunk(1, 1);

	std::vector<float> height(generateHeight(VERT_LEN, VERT_ROW, actualSeed));

	newValue = height;

	myPlane.set(VERT_LEN*VERT_ROW, VERT_LEN, height);
	myPlane.update();

	myWindow.setMouseCallback(mouseMoving);
	myWindow.setKeyCallback(key_callback);
	myWindow.setGraphicLoop(mainGraphicLoop);

	myWindow.startLoop();

	return 0;
}

void mainGraphicLoop(float elapsedTime){
	input(elapsedTime);

	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	

	if(isInterpolate){

		actualDelta += elapsedTime * speed;

		if(actualDelta > 1.0){

			if(water){
				actualSeed = rand()%1000000;
				startInterpolate();
			}
			else{
				actualDelta = 1.0;
				isInterpolate = false;
			}

		}

		std::vector<float> height(cosInterpolateVector(oldValue, newValue, actualDelta));

		myPlane.set(VERT_LEN*VERT_ROW, VERT_LEN, height);
		myPlane.update();

	}




	myPlane.draw();
	//std::cout << "Je ne fais rien : " << elapsedTime << std::endl;

	//myWorld.drawAll();

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