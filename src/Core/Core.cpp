
#include "Core/Core.hpp"
#include "Core/Global.hpp"

#include <iostream>

//Static Variable



namespace mav {
	
	Window::Window(std::string const& windowName, int width, int height) : mouseEventFunction_(nullptr), keyEventFunction_(nullptr) {

		glfwInit();
	    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	    
	    #ifdef __APPLE__
	    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		#endif
	  

	    window_ = glfwCreateWindow(width, height, windowName.data(), NULL, NULL);
		
		if (window_ == NULL){
		    std::cout << "Failed to create GLFW window" << std::endl;
		    glfwTerminate();
		    throw 1;
		}

		glfwGetWindowSize(window_, &width, &height);

		Global::width	= width;
		Global::height	= height;

		glfwMakeContextCurrent(window_);


		glfwSetWindowUserPointer(window_, this);

		auto mouseMoveCallback = [](GLFWwindow* w, double xPos, double yPos){
			static_cast<Window*>(glfwGetWindowUserPointer(w))->mouseMovingCallback(xPos, yPos);
		};
		glfwSetCursorPosCallback(window_, mouseMoveCallback);

		auto mouseClickCallback = [](GLFWwindow* w, int button, int action, int mods){
			static_cast<Window*>(glfwGetWindowUserPointer(w))->mouseClickCallback(button, action, mods);
		};
		glfwSetMouseButtonCallback(window_, mouseClickCallback);
		
		auto keyCallback = [](GLFWwindow* w, int key, int scancode, int action, int mods){
			static_cast<Window*>(glfwGetWindowUserPointer(w))->keyCallback(key, scancode, action, mods);
		};
		glfwSetKeyCallback(window_, keyCallback);


		//Desactivate the mouse
		glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		
		//FPS 60
	    glfwSwapInterval(1);

		//glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);


		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
	    	std::cout << "Failed to initialize GLAD" << std::endl;
	    	throw 2;
		}
		

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