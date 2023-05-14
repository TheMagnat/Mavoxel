
#include "Core/Core.hpp"
#include "Core/Global.hpp"

#include <iostream>

//Static Variable



namespace mav {
	
	Window::Window(std::string const& windowName, int width, int height) : mouseEventFunction_(nullptr), keyEventFunction_(nullptr) {

		std::cout << "Deprecated Window" << std::endl;
		throw 1;

		

	}

	void Window::closeWindow() const {
		glfwSetWindowShouldClose(window_, true);
	}

	bool Window::isPressed(int key) const {
		return glfwGetKey(window_, key) == GLFW_PRESS;
	}

	void Window::setGraphicLoop(functionFloat newFunction){
		graphicLoopFunction_ = newFunction;
	}

	void Window::setMouseCallback(functionDoubleDouble newFunction){
		mouseEventFunction_ = newFunction;
	}

	void Window::setMouseClickCallback(functionIntIntInt newFunction) {
		mouseClickEventFunction_ = newFunction;
	}

	void Window::setKeyCallback(functionIntIntIntInt newFunction){
		keyEventFunction_ = newFunction;
	}

	void Window::startLoop() const {

		float deltaTime, lastFrame = glfwGetTime();

		while(!glfwWindowShouldClose(window_)){

			float time = glfwGetTime();

			//Time Calculation
			deltaTime = time - lastFrame;
        	lastFrame = time;

			graphicLoopFunction_(deltaTime);

			glfwSwapBuffers(window_);
	    	glfwPollEvents();
		
		}
		
	}

	//CALLBACK
	void Window::mouseMovingCallback(double xPos, double yPos){
		if(mouseEventFunction_){
			mouseEventFunction_(xPos, yPos);
		}
	}

	void Window::mouseClickCallback(int button, int action, int mods){
		if(mouseClickEventFunction_){
			mouseClickEventFunction_(button, action, mods);
		}
	}

	void Window::keyCallback(int key, int scancode, int action, int mods){
		if(keyEventFunction_){
			keyEventFunction_(key, scancode, action, mods);
		}
	}

}