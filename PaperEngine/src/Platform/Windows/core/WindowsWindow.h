//
// Created by NS on 2025/3/29.
//

#ifndef WINDOWSWINDOW_H
#define WINDOWSWINDOW_H

#include <string>

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>


#include <PaperEngine/core/Window.h>

namespace PaperEngine {

    class WindowsWindow : public Window {
    public:

		void init(const WindowProps& props) override;

        uint32_t get_width() const override { return m_data.width; }

		uint32_t get_height() const override { return m_data.height; }

    private:
		static uint32_t s_windowCount;

    private:
        GLFWwindow* m_handle;

        struct WindowData {
            std::string title;
            uint32_t width, height;
        };

        WindowData m_data;

    };
}



#endif //WINDOWSWINDOW_H
