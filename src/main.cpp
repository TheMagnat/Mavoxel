
/*

This main file is only for testing the game engine,
you can use a totally different main file.

*/


//TESTTT

#include "PerlinNoise.hpp"
#include "Neural/Neural.hpp"

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



#define VERT_LEN 300
#define VERT_ROW 300




//DECLARE FUNC

void mainGraphicLoop(float elapsedTime);

void input(float deltaTime);

std::vector<float> generateHeight(size_t len, size_t row, size_t seed);
std::vector<float> cubeGenerateHeight(size_t len, size_t row, size_t seed);
std::vector<float> coneGenerateHeight(size_t len, size_t row, size_t seed);
std::vector<float> generateHeightFromNeural();

//static size_t actualSeed = 1;

//


static mav::Window myWindow("Neural Network Viewer", 1200, 700);

//INTERPOLATE

static bool linear = true;
static bool inAnimation = false;
static unsigned int counter = 0;

static bool water = false;
static int mode = 0;

static float actualDelta = 0.0;
static bool isInterpolate = false;

static std::vector<float> oldValue;
static std::vector<float> newValue;

static std::vector<glm::vec3> allColors;


///


//static mav::World myWorld(32);

static Neural myNeur;

static mav::Shader myShader;
static mav::Camera myCam(glm::vec3(0, 20, 0));

static mav::ColorPlane myPlane(&myShader, &myCam, 100);
static mav::ColorPlane leftPlane(&myShader, &myCam, 100);

static int AdditionalArgument = 0;

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

	cosDelta = (1.0f - cos(delta * M_PI)) / 2.0f;


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

	newValue = generateHeightFromNeural();

	// if(mode == 0){
	// 	newValue = generateHeight(VERT_LEN, VERT_ROW, actualSeed);
		
	// }
	// else if(mode == 1){
	// 	newValue = cubeGenerateHeight(VERT_LEN, VERT_ROW, actualSeed);
	// }
	// else if(mode == 2){
	// 	newValue = coneGenerateHeight(VERT_LEN, VERT_ROW, actualSeed);
	// }
	

}

void key_callback(int key, int scancode, int action, int mods){

	if (key == GLFW_KEY_UP && action == GLFW_PRESS){

		++AdditionalArgument;

    	startInterpolate();

    }

    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS){

		if(--AdditionalArgument < 0){
			AdditionalArgument = 0;
		}
		else{
			startInterpolate();	
		}

    	

    }

	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS){

		if(myWindow.isPressed(GLFW_KEY_LEFT_ALT)){
			speed += 1.0;
		}
		else{

			speed += 0.1;
		}

    }

    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS){
		
		if(myWindow.isPressed(GLFW_KEY_LEFT_ALT)){
			if((speed -= 1.0) < 0.0){
				speed = 0.0;
			}
		}
		else{
			if((speed -= 0.1) < 0.0){
				speed = 0.0;
			}
		}

    }

	if (key == GLFW_KEY_O && action == GLFW_PRESS){

		myNeur.learnOneByOne();

		allColors = myNeur.getColorVector(VERT_LEN, VERT_ROW);

    	startInterpolate();

		inAnimation = true;
		
    }

	if (key == GLFW_KEY_P && action == GLFW_PRESS){

		myNeur.dataOneByOne();

		allColors = myNeur.getColorVector(VERT_LEN, VERT_ROW);

    	startInterpolate();
		
    }

	if (key == GLFW_KEY_M && action == GLFW_PRESS){

		myNeur.learnOneByOne();

		allColors = myNeur.getColorVector(VERT_LEN, VERT_ROW);

    	startInterpolate();
		
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
	if (key == GLFW_KEY_L && action == GLFW_PRESS){

		linear = 1 - linear;

    	startInterpolate();

    }

}


std::vector<float> generateHeightFromNeural(){
	
	return myNeur.generateHeight(VERT_LEN, VERT_ROW, linear, 2, 30, maxHeight);

}

void updateAnimation(float elapsedTime){

	if(isInterpolate){

		actualDelta += elapsedTime * speed;

		if(actualDelta > 1.0){

			actualDelta = 1.0;
			isInterpolate = false;



		}

		std::vector<float> height(cosInterpolateVector(oldValue, newValue, actualDelta));

		myPlane.set(VERT_LEN*VERT_ROW, VERT_LEN, height, allColors);
		myPlane.update();



	}


			//The animation just finished
	if(inAnimation){
		if(!isInterpolate){

			++counter;

			myNeur.learnOneByOne();

			allColors = myNeur.getColorVector(VERT_LEN, VERT_ROW);

			startInterpolate();

			if(counter == 2000){
				inAnimation = false;
			}

		}
	}
	

}

int main(int argc, char const *argv[]){

	srand(time(NULL));
	srand(rand()%100000);

	//Neural part
	myNeur.init({20, 5}, {-0.05, 0.025});
	myNeur.generate("iris.data", 150, 4, {"Iris-setosa", "Iris-versicolor", "Iris-virginica"});
	
	std::cout << "1\n";
	myNeur.initOneByOne();
	std::cout << "2\n";
	//myNeur.learn();

	//myNeur.print();



	//Graphic Part
	myShader.load("Shaders/basic.vs", "Shaders/basic.fs", "Shaders/basic.gs");
	myShader.setFloat("water",  0.0f);


	myPlane.init();

	// myPlane.setPosition(0, -100);


	leftPlane.init();
	leftPlane.setPosition(-50, 0);
	leftPlane.setRotationMatrice(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	leftPlane.set(2*2, 2, std::vector<float>{0, 0, 0, 0}, std::vector<glm::vec3>(4, glm::vec3(0.0f, 0.0f, 1.0f)));
	leftPlane.update();

	//myWorld.createChunk(0, 0);
	// myWorld.createChunk(1, 0);
	// myWorld.createChunk(0, 1);
	// myWorld.createChunk(1, 1);



	// std::vector<float> height(generateHeight(VERT_LEN, VERT_ROW, actualSeed));
	std::vector<float> height(generateHeightFromNeural());
	newValue = height;

	std::cout << "get color\n";
	allColors = myNeur.getColorVector(VERT_LEN, VERT_ROW);
	std::cout << "fin get color\n";
	myPlane.set(VERT_LEN*VERT_ROW, VERT_LEN, height, allColors);
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
	
	updateAnimation(elapsedTime);

	myPlane.draw();
	leftPlane.draw();
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