
#include "Core/Core.hpp"
#include "World/World.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

static mav::Window myWindow("Mavoxel");

static mav::World myWorld(32);

void input(){

	if(myWindow.isPressed(GLFW_KEY_ESCAPE)){
		myWindow.closeWindow();
	}

}

void mainGraphicLoop(float elapsedTime){
	input();
	
	std::cout << "Je ne fais rien : " << elapsedTime << std::endl;

	myWorld.drawAll();

}

int main(int argc, char const *argv[]){

	myWorld.createChunk(0, 0);
	myWorld.createChunk(1, 0);
	myWorld.createChunk(0, 1);
	myWorld.createChunk(1, 1);

	myWindow.setGraphicLoop(mainGraphicLoop);

	myWindow.startLoop();

	return 0;
}