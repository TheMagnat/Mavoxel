
#pragma once

#include <GLFW/glfw3.h>

#include <string>
#include <functional>

using function = std::function<void()>;
using functionFloat = std::function<void(float x)>;
using functionDoubleDouble = std::function<void(double x, double y)>;
using functionIntIntIntInt = std::function<void(int x, int y, int z, int w)>;
using functionIntIntInt = std::function<void(int x, int y, int z)>;
using functionIntInt = std::function<void(int x, int y)>;

namespace vuw {

	class Window {

		public:

			Window(std::string const& windowName, int width, int height, bool disableCursor = true);

			~Window();

			//Verify if a key is pressed
			bool isPressed(int key) const;

			//Close the window
			void closeWindow() const;

			//Set callbacks
			void startLoop() const;
			void setGraphicLoop(functionFloat newFunction);

			void setMouseCallback(functionDoubleDouble newFunction);
			void setMouseClickCallback(functionIntIntInt newFunction);
			void setKeyCallback(functionIntIntIntInt newFunction);
			void setResizeCallback(functionIntInt newFunction);

			//Callbacks
			void mouseMovingCallback(double xPos, double yPos);
			void mouseClickCallback(int button, int action, int mods);
			void keyCallback(int key, int scancode, int action, int mods);
			void resizeCallback(int width, int height);

			inline GLFWwindow* get() const {
				return window_;
			}

		private:
			GLFWwindow* window_;

			functionFloat graphicLoopFunction_;

			//Input
			functionDoubleDouble mouseEventFunction_;
			functionIntIntInt mouseClickEventFunction_;
			functionIntIntIntInt keyEventFunction_;
			functionIntInt resizeEventFunction_;

	};

}
