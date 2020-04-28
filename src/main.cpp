


#include "Neural/Neural.hpp"


#include "Core/Global.hpp"

#include "Core/Core.hpp"
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

std::vector<float> generateHeightFromNeural();

void startInterpolate();

//Main Window
static mav::Window myWindow("Neural Network Viewer", 1200, 700);

//INTERPOLATION
static float actualDelta = 0.0;
static bool isInterpolate = false;

static std::vector<float> oldValue;
static std::vector<float> newValue;

static std::vector<glm::vec3> oldColors;
static std::vector<glm::vec3> newColors;


//OPTION
static bool linear = false;
static bool colorLinear = true;
static bool showColors = true;
static int AdditionalArgument = 0;
static float maxHeight = 100.0f;

//ANIMATION OPTION
static int mode = 0;
static bool inAnimation = false;
static float speed = 1.0f;


//Programm Variable
static Neural myNeur;

//Graphic Variable
static mav::Shader myShader;
static mav::Camera myCam(glm::vec3(0, 60, 80));

static mav::ColorPlane myPlane(&myShader, &myCam, 100);
static std::vector<mav::ColorPlane> sidePlanes;

//Mouse Variable
float lastX = mav::Global::width / 2.0f;
float lastY = mav::Global::height / 2.0f;
bool firstMouse = true;



/**
 * Callback function when the mouse move
*/
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

/**
 * The callback function called when a key is pushed
*/
void key_callback(int key, int scancode, int action, int mods){

	if (key == GLFW_KEY_UP && action == GLFW_PRESS){

		++AdditionalArgument;

    	startInterpolate();

    }

    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS){

		--AdditionalArgument;

		startInterpolate();	    	

    }

	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS){

		if(myWindow.isPressed(GLFW_KEY_LEFT_ALT)){
			speed += 1.0;
		}
		else{

			speed += 0.1;
		}

		std::cout << "Speed : " << speed << std::endl;

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

		std::cout << "Speed : " << speed << std::endl;

    }

	if (key == GLFW_KEY_O && action == GLFW_PRESS){
		
		inAnimation = 1 - inAnimation;

		if(inAnimation){

			if(mode == 0)
				myNeur.dataOneByOne();
			else if(mode == 1)
				myNeur.learnOneByOne();
			else{
				myNeur.finishAll();
			}

			myNeur.reLabelize();

			startInterpolate();

		}
		
    }

	if (key == GLFW_KEY_P && action == GLFW_PRESS){

		if(mode == 0)
			myNeur.dataOneByOne();
		else if(mode == 1)
			myNeur.learnOneByOne();
		else{
			myNeur.finishAll();
		}

		myNeur.reLabelize();

    	startInterpolate();
		
    }

	//Reset
	if (key == GLFW_KEY_R && action == GLFW_PRESS){

		myNeur.reset();

		startInterpolate();

	}

	if (key == GLFW_KEY_C && action == GLFW_PRESS){
		
		showColors = 1 - showColors;

		startInterpolate();

	}

	if (key == GLFW_KEY_V && action == GLFW_PRESS){
		
		colorLinear = 1 - colorLinear;

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


// std::vector<float> interpolateVector(std::vector<float> const& first, std::vector<float> const& second, float delta){

// 	std::vector<float> result;


// 	for(size_t i(0); i < first.size(); ++i){
// 		result.emplace_back(first[i] * (1.0f - delta) + second[i] * delta);
// 	}


// 	return result;

// }

/**
 * This function interpolate two vector of float
*/
std::vector<float> cosInterpolateVector(std::vector<float> const& first, std::vector<float> const& second, float delta){

	std::vector<float> result;

	float cosDelta;

	cosDelta = (1.0f - cos(delta * M_PI)) / 2.0f;


	for(size_t i(0); i < first.size(); ++i){
		result.emplace_back(first[i] * (1.0f - cosDelta) + second[i] * cosDelta);
	}


	return result;

}

/**
 * This function interpolate two vector of glm::vec
*/
std::vector<glm::vec3> cosInterpolateVec3Vector(std::vector<glm::vec3> const& first, std::vector<glm::vec3> const& second, float delta){

	std::vector<glm::vec3> result;

	float cosDelta;

	cosDelta = (1.0f - cos(delta * M_PI)) / 2.0f;


	for(size_t i(0); i < first.size(); ++i){

		glm::vec3 tempo;

		for(uint8_t j(0); j < 3; ++j){
			tempo[j] = first[i][j] * (1.0f - cosDelta) + second[i][j] * cosDelta;
		}

		result.emplace_back(std::move(tempo));

	}


	return result;

}

/**
 * This function prepare the animated interpolation between the old
 * height of the plane and the new one generated by generateHeightFromNeural.
*/
void startInterpolate(){

	isInterpolate = true;
	actualDelta = 0.0f;


	oldValue = newValue;
	newValue = generateHeightFromNeural();

	oldColors = newColors;

	if(showColors){
		newColors = myNeur.getColorVector(VERT_LEN, VERT_ROW, colorLinear);
	}
	else{
		newColors = std::vector<glm::vec3>(newColors.size(), glm::vec3(219.0f/255.0f, 206.0f/255.0f, 211.0f/255.0f));
	}

}


/**
 * return the new height generated
*/
std::vector<float> generateHeightFromNeural(){
	
	return myNeur.generateHeight(VERT_LEN, VERT_ROW, linear, 2, 50, maxHeight+5*AdditionalArgument, -150.0f);

}

/**
 * Live input check function
*/
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


void updateAnimation(float elapsedTime){

	if(isInterpolate){

		actualDelta += elapsedTime * speed;

		if(actualDelta > 1.0){

			actualDelta = 1.0;
			isInterpolate = false;



		}

		std::vector<float> height(cosInterpolateVector(oldValue, newValue, actualDelta));
		std::vector<glm::vec3> colors;
		
		colors = cosInterpolateVec3Vector(oldColors, newColors, actualDelta);

		myPlane.set(VERT_LEN*VERT_ROW, VERT_LEN, height, colors);
		myPlane.update();

	}


	if(inAnimation){
		if(!isInterpolate){

			int end;

			if(mode == 0)
				end = myNeur.dataOneByOne();
			else if(mode == 1)
				end = myNeur.learnOneByOne();
			else{
				myNeur.finishAll();
				end = 1;
			}

			myNeur.reLabelize();

			startInterpolate();

			if(end){
				inAnimation = false;
			}

		}
	}

}


/**
 * Main graphic loop
*/
void mainGraphicLoop(float elapsedTime){
	
	input(elapsedTime);

	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	updateAnimation(elapsedTime);

	

	myPlane.draw();

	glEnable(GL_CULL_FACE);
	for(size_t i(0); i < 4; ++i){
		sidePlanes[i].draw();
	}
	glDisable(GL_CULL_FACE);

}

int main(int argc, char const *argv[]){

	srand(time(NULL));
	srand(rand()%100000);

	//Neural part
	myNeur.init({10, 6}, {-0.05, 0.025});
	myNeur.generate("iris.data", 150, 4, {"Iris-setosa", "Iris-versicolor", "Iris-virginica"});
	myNeur.initAfterGenerate();

	//Graphic Part
	myShader.load("Shaders/basic.vs", "Shaders/basic.fs", "Shaders/basic.gs");
	myShader.setFloat("water",  0.0f);
	myPlane.init();

	for(size_t i = 0; i < 4; ++i){
		
		sidePlanes.emplace_back(&myShader, &myCam, 100);
		sidePlanes[i].init();
		sidePlanes[i].setSizeVec(glm::vec3(500.0f, 1.0f, 50.0f)),
		sidePlanes[i].set(2*2, 2, std::vector<float>{0, 0, 0, 0}, std::vector<glm::vec3>(4, glm::vec3(0.0f, 0.0f, 1.0f)));
		sidePlanes[i].update();

	}	

	sidePlanes[0].setPosition(-50, 0);
	sidePlanes[0].setRotationMatrice(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	
	sidePlanes[1].setPosition(0, -50);
	sidePlanes[1].setSizeVec(glm::vec3(50.0f, 1.0f, 500.0f)),
	sidePlanes[1].setRotationMatrice(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));

	sidePlanes[2].setPosition(50, 0);
	sidePlanes[2].setRotationMatrice(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)));

	sidePlanes[3].setPosition(0, 50);
	sidePlanes[3].setSizeVec(glm::vec3(50.0f, 1.0f, 500.0f)),
	sidePlanes[3].setRotationMatrice(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));


	newValue = generateHeightFromNeural();
	newColors = myNeur.getColorVector(VERT_LEN, VERT_ROW, colorLinear);

	myPlane.set(VERT_LEN*VERT_ROW, VERT_LEN, newValue, newColors);
	myPlane.update();

	myWindow.setMouseCallback(mouseMoving);
	myWindow.setKeyCallback(key_callback);
	myWindow.setGraphicLoop(mainGraphicLoop);

	myWindow.startLoop();

	return 0;
}