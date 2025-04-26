#include "Keyboard.h"

#include "Application.h"
#include "Window.h"
#include "Platform/Windows/core/WindowsWindow.h"

namespace PaperEngine {

	bool Keyboard::IsKeyDown(KeyCode keyCode)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().get_window().get_native_window());

		return glfwGetKey(window, keyCode) == GLFW_PRESS;
	}

}
