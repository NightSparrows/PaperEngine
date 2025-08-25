#include "Mouse.h"

#include "Application.h"
#include "Window.h"

#include "GLFWWindow.h"

namespace PaperEngine {

    bool Mouse::IsMouseButtonDown(MouseCode button)
    {
        GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->getWindow()->getNativeWindow());

		return glfwGetMouseButton(window, button) == GLFW_PRESS;
    }

    PE_API void Mouse::GrabMouseCursor(bool grab)
    {
        GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->getWindow()->getNativeWindow());

		glfwSetInputMode(window, GLFW_CURSOR, grab ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    PE_API glm::vec2 Mouse::GetDeltaPosition()
    {
		return Application::Get()->getWindow()->getCursorDeltaPosition();
    }

}
