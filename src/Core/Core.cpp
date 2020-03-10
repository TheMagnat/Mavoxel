
#include "Core/Core.hpp"
#include "Core/Global.hpp"

#include <iostream>

//Static Variable



namespace mav {
	
	Window::Window(std::string const& windowName /*= "default"*/, int width/* = 800*/, int height/* = 600*/) : mouseEventFunction_(nullptr), keyEventFunction_(nullptr) {

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

		auto mouse_callback = [](GLFWwindow* w, double xpos, double ypos){
			static_cast<Window*>(glfwGetWindowUserPointer(w))->mouseMovingCallback(xpos, ypos);
		};
		glfwSetCursorPosCallback(window_, mouse_callback);
		
		auto key_callback = [](GLFWwindow* w, int key, int scancode, int action, int mods){
			static_cast<Window*>(glfwGetWindowUserPointer(w))->keyCallback(key, scancode, action, mods);
		};
		glfwSetKeyCallback(window_, key_callback);


		//Desactivate the mouse
		glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		
		//FPS 60
	    glfwSwapInterval(1);

		//glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);
	    //glfwSetKeyCallback(window_, key_callback);


		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
	    	std::cout << "Failed to initialize GLAD" << std::endl;
	    	throw 2;
		}
		

	}

	void Window::closeWindow(){
		glfwSetWindowShouldClose(window_, true);
	}

	bool Window::isPressed(int key){
		return glfwGetKey(window_, key) == GLFW_PRESS;
	}

	void Window::setGraphicLoop(functionFloat newFunction){
		graphicLoopFunction_ = newFunction;
	}

	void Window::setMouseCallback(functionDoubleDouble newFunction){
		mouseEventFunction_ = newFunction;
	}

	void Window::setKeyCallback(functionIntIntIntInt newFunction){
		keyEventFunction_ = newFunction;
	}

	void Window::startLoop(){

		float deltaTime, lastFrame;

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
	void Window::mouseMovingCallback(double xpos, double ypos){
		if(mouseEventFunction_){
			mouseEventFunction_(xpos, ypos);
		}
	}


	void Window::keyCallback(int key, int scancode, int action, int mods){
		if(keyEventFunction_){
			keyEventFunction_(key, scancode, action, mods);
		}
	}



}