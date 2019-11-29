#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>

typedef void(*functionFloat)(float);
typedef void(*function)();
typedef void(*functionDoubleDouble)(double, double);

namespace mav{

	class Window{


		public:

			Window(std::string const& windowName, int width = 800, int height = 600);

			bool isPressed(int key);

			void setGraphicLoop(functionFloat newFunction);

			void startLoop();

			void closeWindow();



			///Callback
			void setMouseCallback(functionDoubleDouble newFunction);
			void mouseMovingCallback(double xpos, double ypos);


		private:

			GLFWwindow* window_;

			functionFloat graphicLoopFunction_;

			//Input
			functionDoubleDouble mouseEventFunction_;

	};

}