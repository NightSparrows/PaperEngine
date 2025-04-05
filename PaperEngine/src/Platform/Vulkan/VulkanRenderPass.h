#pragma once

#include <vulkan/vulkan.h>

#include <PaperEngine/renderer/RenderPass.h>

#include "VulkanTexture2D.h"

namespace PaperEngine {

	class VulkanRenderPass : public RenderPass {
	public:
		VulkanRenderPass(const RenderPassCreateInfo& createInfo, uint32_t width, uint32_t height);
		~VulkanRenderPass();

		void recreate_framebuffers(uint32_t width, uint32_t height) override;

	private:

		VkRenderPass m_renderPass{ VK_NULL_HANDLE };
		
		RenderPassCreateInfo m_createInfo;

		struct FramebufferInfo {
			std::vector<VulkanTexture2D> attachments;
			VkFramebuffer handle{ VK_NULL_HANDLE };
		};
		std::vector<FramebufferInfo> m_framebuffers;

	};

}
