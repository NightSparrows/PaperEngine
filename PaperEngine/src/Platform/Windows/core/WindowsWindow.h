//
// Created by NS on 2025/3/29.
//

#ifndef WINDOWSWINDOW_H
#define WINDOWSWINDOW_H

#include <string>

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>


#include <PaperEngine/core/Window.h>
#include <PaperEngine/renderer/GraphicsContext.h>

namespace PaperEngine {

    class WindowsWindow : public Window {
    public:
        WindowsWindow(const WindowProps& props);
        ~WindowsWindow();

        void init() override;

        void cleanUp() override;

        void on_update() override;

        void set_event_callback(const EventCallbackFn& callback) override;

        uint32_t get_width() const override { return m_data.width; }

		uint32_t get_height() const override { return m_data.height; }

		void* get_native_window() override { return static_cast<void*>(m_handle); }

		GraphicsContext& get_context() override { return *m_context.get(); }

    private:
		static uint32_t s_windowCount;

    private:
        GLFWwindow* m_handle{ nullptr };

		Scope<GraphicsContext> m_context;

        struct WindowData {
            std::string title;
            uint32_t width, height;
			EventCallbackFn eventCallback;
        };

        WindowData m_data;

    };
}



#endif //WINDOWSWINDOW_H
