
#include "Core.hpp"

#include <iostream>

//Static Variable



namespace mav {
	
	Window::Window(std::string const& windowName /*= "default"*/, int width/* = 800*/, int height/* = 600*/){

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

		glfwMakeContextCurrent(window_);

		
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






}