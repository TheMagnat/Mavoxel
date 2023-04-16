#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <functional>
#include <string>

using function = std::function<void()>;
using functionFloat = std::function<void(float x)>;
using functionDoubleDouble = std::function<void(double x, double y)>;
using functionIntIntIntInt = std::function<void(int x, int y, int z, int w)>;
using functionIntIntInt = std::function<void(int x, int y, int z)>;


namespace mav{

	class Window{


		public:

			Window(std::string const& windowName, int width = 800, int height = 600);

			bool isPressed(int key) const;

			void setGraphicLoop(functionFloat newFunction);

			void startLoop() const;

			void closeWindow() const;



			///Callback
			void setMouseCallback(functionDoubleDouble newFunction);
			void mouseMovingCallback(double xPos, double yPos);

			void setMouseClickCallback(functionIntIntInt newFunction);
			void mouseClickCallback(int button, int action, int mods);

			void setKeyCallback(functionIntIntIntInt newFunction);
			void keyCallback(int key, int scancode, int action, int mods);


		private:

			GLFWwindow* window_;

			functionFloat graphicLoopFunction_;

			//Input
			functionDoubleDouble mouseEventFunction_;
			functionIntIntInt mouseClickEventFunction_;
			functionIntIntIntInt keyEventFunction_;

	};

}