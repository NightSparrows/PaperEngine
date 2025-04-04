#pragma once

#include <vulkan/vulkan.h>

namespace PaperEngine {

	class VulkanRenderer {
	public:

		void init();

		void cleanUp();

		void begin_frame();
		void end_frame();

		static VulkanRenderer& Get() {
			static VulkanRenderer instance;
			return instance;
		}

	private:
		void createFramebuffers();

	private:
		VkRenderPass m_renderPass{ VK_NULL_HANDLE };
		std::vector<VkFramebuffer> m_framebuffers;
	};

}
