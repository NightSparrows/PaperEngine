#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <PaperEngine/renderer/GraphicsContext.h>

namespace PaperEngine {

	class VulkanContext : public GraphicsContext {
	public:

		VulkanContext(GLFWwindow* handle);
		~VulkanContext();

		void init() override;
		void swapBuffers() override;

	private:
		GLFWwindow* m_windowHandle{ nullptr };

		VkInstance m_instance{ VK_NULL_HANDLE };

	};

}
