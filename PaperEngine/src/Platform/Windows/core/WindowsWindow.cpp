//
// Created by NS on 2025/3/29.
//
#include <PaperEngine/core/Logger.h>

#include "WindowsWindow.h"
#include <PaperEngine/events/KeyEvent.h>
#include <PaperEngine/events/MouseEvent.h>
#include <PaperEngine/events/ApplicationEvent.h>

namespace PaperEngine {

	uint32_t WindowsWindow::s_windowCount = 0;

	static void GLFWErrorCallback(int error, const char* description)
	{
		PE_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		m_data.height = props.Height;
		m_data.width = props.Width;
		m_data.title = props.Title;

		if (s_windowCount == 0)
		{
			if (!glfwInit())
			{
				PE_CORE_ERROR("Failed to initialize GLFW!");
			}
			glfwSetErrorCallback(GLFWErrorCallback);
		}

	}

	WindowsWindow::~WindowsWindow()
	{
		if (m_handle) {
			glfwDestroyWindow(m_handle);
			s_windowCount--;
			if (s_windowCount == 0)
			{
				glfwTerminate();
			}
		}
	}

	void WindowsWindow::init()
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);	// vulkan 
		m_handle = glfwCreateWindow(m_data.width, m_data.height, m_data.title.c_str(), nullptr, nullptr);
		s_windowCount++;

		m_context = GraphicsContext::Create(m_handle);
		m_context->init();

		glfwSetWindowUserPointer(m_handle, &m_data);

		glfwSetWindowSizeCallback(m_handle, [](GLFWwindow* window, int width, int height) {
			WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
			data->width = static_cast<uint32_t>(width);
			data->height = static_cast<uint32_t>(height);

			WindowResizeEvent e(data->width, data->height);
			data->eventCallback(e);
			});

		glfwSetWindowCloseCallback(m_handle, [](GLFWwindow* window) {
			WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);

			WindowCloseEvent e;
			data->eventCallback(e);
			});

		glfwSetKeyCallback(m_handle, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressedEvent e(key, 0);
				data->eventCallback(e);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent e(key);
				data->eventCallback(e);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyPressedEvent e(key, true);
				data->eventCallback(e);
				break;
			}
			default:
				break;
			}
			});

		glfwSetCharCallback(m_handle, [](GLFWwindow* window, unsigned int keycode) {
			WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);

			KeyTypedEvent e(keycode);
			data->eventCallback(e);
			});

		glfwSetMouseButtonCallback(m_handle, [](GLFWwindow* window, int button, int action, int mods) {
			WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);

			switch (action) {
			case GLFW_PRESS:
			{
				MouseButtonPressedEvent e(button);
				data->eventCallback(e);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent e(button);
				data->eventCallback(e);
				break;
			}
			default:
				break;
			}
			});

		glfwSetScrollCallback(m_handle, [](GLFWwindow* window, double xOffset, double yOffset) {
			WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent e(static_cast<float>(xOffset), static_cast<float>(yOffset));
			data->eventCallback(e);
			});

		glfwSetCursorPosCallback(m_handle, [](GLFWwindow* window, double xPos, double yPos) {
			WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent e(static_cast<float>(xPos), static_cast<float>(yPos));
			data->eventCallback(e);
			});

	}

	void WindowsWindow::on_update()
	{
		glfwPollEvents();
		m_context->swapBuffers();
	}

	void WindowsWindow::set_event_callback(const EventCallbackFn& callback)
	{
		m_data.eventCallback = callback;
	}

}
