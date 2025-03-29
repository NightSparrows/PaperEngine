//
// Created by NS on 2025/3/29.
//

#include "WindowsWindow.h"

namespace PaperEngine {

	uint32_t WindowsWindow::s_windowCount = 0;

	void WindowsWindow::init(const WindowProps& props)
	{
		m_data.height = props.Height;
		m_data.width = props.Width;
		m_data.title = props.Title;

		if (s_windowCount == 0)
		{
			if (!glfwInit())
			{
				//PE_CORE_ERROR("Failed to initialize GLFW!");
			}
		}
		m_handle = glfwCreateWindow(m_data.width, m_data.height, m_data.title.c_str(), nullptr, nullptr);
		s_windowCount++;

		// TODO: create context

	}

}
