
#pragma once

#include <VulkanWrapper/Window.hpp>

namespace vuw {

	Window::Window(std::string const& windowName, int width, int height, bool disableCursor) {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window_ = glfwCreateWindow(width, height, windowName.data(), nullptr, nullptr);
		glfwSetWindowUserPointer(window_, this);

		//glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);

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

		auto resizeCallback = [](GLFWwindow* w, int width, int height){
			static_cast<Window*>(glfwGetWindowUserPointer(w))->resizeCallback(width, height);
		};
		glfwSetFramebufferSizeCallback(window_, resizeCallback);


		//Desactivate the mouse
		if (disableCursor)
			glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		
		//FPS 60
		// glfwSwapInterval(1);
	}

	Window::~Window() {
		glfwDestroyWindow(window_);
		glfwTerminate();
	}

	void Window::closeWindow() const {
		glfwSetWindowShouldClose(window_, true);
	}

	bool Window::isPressed(int key) const {
		return glfwGetKey(window_, key) == GLFW_PRESS;
	}

	void Window::setGraphicLoop(functionFloat newFunction) {
		graphicLoopFunction_ = newFunction;
	}

	void Window::setMouseCallback(functionDoubleDouble newFunction) {
		mouseEventFunction_ = newFunction;
	}

	void Window::setMouseClickCallback(functionIntIntInt newFunction) {
		mouseClickEventFunction_ = newFunction;
	}

	void Window::setKeyCallback(functionIntIntIntInt newFunction) {
		keyEventFunction_ = newFunction;
	}

	void Window::setResizeCallback(functionIntInt newFunction) {
		resizeEventFunction_ = newFunction;
	}

	// Callbacks
	void Window::mouseMovingCallback(double xPos, double yPos) {
		if(mouseEventFunction_){
			mouseEventFunction_(xPos, yPos);
		}
	}

	void Window::mouseClickCallback(int button, int action, int mods) {
		if(mouseClickEventFunction_){
			mouseClickEventFunction_(button, action, mods);
		}
	}

	void Window::keyCallback(int key, int scancode, int action, int mods) {
		if(keyEventFunction_){
			keyEventFunction_(key, scancode, action, mods);
		}
	}

	void Window::resizeCallback(int width, int height) {
		if (resizeEventFunction_) {
			resizeEventFunction_(width, height);
		}
	}

	void Window::startLoop() const {

		float deltaTime, lastFrame = glfwGetTime();

		while(!glfwWindowShouldClose(window_)){

			float time = glfwGetTime();

			//Time Calculation
			deltaTime = time - lastFrame;
			lastFrame = time;

			graphicLoopFunction_(deltaTime);

			glfwPollEvents();
		
		}
		
	}

}
