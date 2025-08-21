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

    class GLFWWindow : public Window {
    public:
        GLFWWindow(const WindowProps& props);
        ~GLFWWindow();

        void init() override;

        void cleanUp() override;

        bool shouldClose() const override;

        void onUpdate() override;

        void setTitle(const std::string& title) override;

        glm::vec2 getCursorDeltaPosition() const override;

        void setEventCallback(const EventCallbackFn& callback) override;

        uint32_t getWidth() const override { return m_data.width; }

		uint32_t getHeight() const override { return m_data.height; }

		void* getNativeWindow() override { return static_cast<void*>(m_handle); }

    private:
		static uint32_t s_windowCount;

    private:
        GLFWwindow* m_handle{ nullptr };

		bool m_disableMinimizeBox{ false };

        struct WindowData {
            std::string title;
            uint32_t width = 0, height = 0;
			EventCallbackFn eventCallback;

			double lastMouseX = 0, lastMouseY = 0;

            double mouseDeltaX, mouseDeltaY;
        };

        WindowData m_data;

    };
}



#endif //WINDOWSWINDOW_H
