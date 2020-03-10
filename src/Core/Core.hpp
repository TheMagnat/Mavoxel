#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>

typedef void(*functionFloat)(float);
typedef void(*function)();
typedef void(*functionDoubleDouble)(double, double);
typedef void(*functionIntIntIntInt)(int, int, int, int);

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

			void setKeyCallback(functionIntIntIntInt newFunction);
			void keyCallback(int key, int scancode, int action, int mods);


		private:

			GLFWwindow* window_;

			functionFloat graphicLoopFunction_;

			//Input
			functionDoubleDouble mouseEventFunction_;

			functionIntIntIntInt keyEventFunction_;

	};

}