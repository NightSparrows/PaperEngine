#include "Keyboard.h"

#include "Application.h"
#include "Window.h"

#include "GLFWWindow.h"

namespace PaperEngine {

	bool Keyboard::IsKeyDown(KeyCode keyCode)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->getWindow()->getNativeWindow());

		return glfwGetKey(window, keyCode) == GLFW_PRESS;
	}

}
