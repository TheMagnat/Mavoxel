#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>

typedef void(*functionFloat)(float);
typedef void(*function)();

namespace mav{

	class Window{


		public:

			Window(std::string const& windowName, int width = 800, int height = 600);

			bool isPressed(int key);

			void setGraphicLoop(functionFloat newFunction);

			void startLoop();

			void closeWindow();



		private:

			GLFWwindow* window_;

			functionFloat graphicLoopFunction_;

	};

}