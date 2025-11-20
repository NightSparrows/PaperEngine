//
// Created by NS on 2025/3/29.
//

#include <PaperEngine/core/PlatformDetection.h>

#include <PaperEngine/core/Logger.h>

#include "GLFWWindow.h"
#include <PaperEngine/events/KeyEvent.h>
#include <PaperEngine/events/MouseEvent.h>
#include <PaperEngine/events/ApplicationEvent.h>
#include <PaperEngine/debug/Instrumentor.h>

#ifdef PE_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <ShellScalingApi.h>
#pragma comment(lib, "Shcore.lib")
#endif // PE_PLATFORM_WINDOWS

namespace PaperEngine {

	uint32_t GLFWWindow::s_windowCount = 0;

	static void GLFWErrorCallback(int error, const char* description)
	{
		PE_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	GLFWWindow::GLFWWindow(const WindowProps& props)
	{
		m_data.height = props.Height;
		m_data.width = props.Width;
		m_data.title = props.Title;
		m_disableMinimizeBox = props.disableMinimizeBox;

		if (s_windowCount == 0)
		{
			if (!glfwInit())
			{
				PE_CORE_ERROR("Failed to initialize GLFW!");
			}
			glfwSetErrorCallback(GLFWErrorCallback);
		}

	}

	GLFWWindow::~GLFWWindow()
	{
		this->cleanUp();
	}

	void GLFWWindow::init()
	{
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);	// vulkan 
		m_handle = glfwCreateWindow(m_data.width, m_data.height, m_data.title.c_str(), nullptr, nullptr);

		PE_CORE_ASSERT(m_handle, "Failed to create GLFW window!");

		// get actual size
		glfwGetWindowSize(m_handle, (int*)&m_data.width, (int*)&m_data.height);

#ifdef PE_PLATFORM_WINDOWS
		//SetWindowLongPtr(glfwGetWin32Window(m_handle), GWL_STYLE, GetWindowLongPtrA(glfwGetWin32Window(m_handle), GWL_STYLE) & ~(WS_MAXIMIZEBOX | WS_MINIMIZEBOX));
		auto style = GetWindowLongPtrA(glfwGetWin32Window(m_handle), GWL_STYLE);
		if (m_disableMinimizeBox) {
			style &= ~WS_MINIMIZEBOX;
		}
		SetWindowLongPtr(glfwGetWin32Window(m_handle), GWL_STYLE, style);
#endif // PE_PLATFORM_WINDOWS

		s_windowCount++;

		glfwSetWindowUserPointer(m_handle, &m_data);

		glfwSetWindowSizeCallback(m_handle, [](GLFWwindow* window, int width, int height) {
			WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
			data->width = static_cast<uint32_t>(width);
			data->height = static_cast<uint32_t>(height);

			WindowResizeEvent e(data->width, data->height);
			data->eventCallback(e);
			});

		glfwSetWindowPosCallback(m_handle, [](GLFWwindow* window, int xpos, int ypos) {
			WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
			// data->eventCallback(WindowMovedEvent(xpos, ypos));

#ifdef PE_PLATFORM_WINDOWS
			HWND hwnd = glfwGetWin32Window(window);
			auto monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

			uint32_t dpiX;
			uint32_t dpiY;
			GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);

			data->dpiScaleFactorX = static_cast<float>(dpiX) / 96.0f; // 96 DPI is the default DPI
			data->dpiScaleFactorY = static_cast<float>(dpiY) / 96.0f; // 96 DPI is the default DPI
#elif defined(PE_PLATFORM_LINUX)
		// Linux support for display scaling using GLFW.
		// This has limited utility due to the issue described above (NULL monitor),
		// and because GLFW doesn't support fractional scaling properly.
		// For example, on a system with 150% scaling it will report scale = 2.0
		// but the window will be either too small or too big, depending on 'resizeWindowWithDisplayScale'

			GLFWmonitor* monitor = glfwGetWindowMonitor(window);

			// Non-fullscreen windows have NULL monitor, let's use the primary monitor in this case
			if (!monitor)
				monitor = glfwGetPrimaryMonitor();

			float dpi_scale_x = (float)data->dpiScaleFactorX;
			float dpi_scale_y = (float)data->dpiScaleFactorY;
			glfwGetMonitorContentScale(monitor, &dpi_scale_x, &dpi_scale_y);
#endif // PE_PLATFORM_WINDOWS
			if (data->prevDpiScaleFactorX != data->dpiScaleFactorX ||
				data->prevDpiScaleFactorY != data->dpiScaleFactorY)
			{
				data->prevDpiScaleFactorX = data->dpiScaleFactorX;
				data->prevDpiScaleFactorY = data->dpiScaleFactorY;
				DisplayScaleChangedEvent e(data->dpiScaleFactorX, data->dpiScaleFactorY);
				data->eventCallback(e);
				PE_CORE_TRACE("Display scale changed: X = {0}, Y = {1}", data->dpiScaleFactorX, data->dpiScaleFactorY);
			}
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
				KeyPressedEvent e(key, scancode, mods, false);
				data->eventCallback(e);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent e(key, scancode, mods);
				data->eventCallback(e);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyPressedEvent e(key, scancode, mods, true);
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
			data->mouseDeltaX = xPos - data->lastMouseX;
			data->mouseDeltaY = yPos - data->lastMouseY;
			data->lastMouseX = xPos;
			data->lastMouseY = yPos;
			data->eventCallback(e);
			});

	}

	void GLFWWindow::cleanUp()
	{
		if (m_handle) {
			glfwDestroyWindow(m_handle);
			m_handle = nullptr;
			s_windowCount--;
			if (s_windowCount == 0)
			{
				glfwTerminate();
			}
		}
	}

	bool GLFWWindow::shouldClose() const
	{
		return glfwWindowShouldClose(m_handle) == GLFW_TRUE;
	}

	void GLFWWindow::onUpdate()
	{
		PE_PROFILE_FUNCTION();
		m_data.mouseDeltaX = 0;
		m_data.mouseDeltaY = 0;
		glfwPollEvents();
	}

	void GLFWWindow::setTitle(const std::string& title)
	{
		m_data.title = title;
		glfwSetWindowTitle(m_handle, m_data.title.c_str());
	}

	glm::vec2 GLFWWindow::getCursorDeltaPosition() const
	{
		return glm::vec2(static_cast<float>(m_data.mouseDeltaX), static_cast<float>(m_data.mouseDeltaY));
	}

	void GLFWWindow::setEventCallback(const EventCallbackFn& callback)
	{
		m_data.eventCallback = callback;
	}

	bool GLFWWindow::createSurface(void* instance_ptr, void* surface_ptr)
	{
		return glfwCreateWindowSurface(static_cast<VkInstance>(instance_ptr), m_handle, nullptr, static_cast<VkSurfaceKHR*>(surface_ptr)) == VK_SUCCESS;
	}

}
