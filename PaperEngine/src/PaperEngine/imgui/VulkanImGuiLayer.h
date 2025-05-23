#pragma once

#include "ImGuiLayer.h"

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

#include <Platform/Vulkan/VulkanContext.h>

#include <PaperEngine/renderer/Texture.h>

namespace PaperEngine {

	class VulkanImGuiLayer : public ImGuiLayer {
	public:

		bool begin_frame() override;

		void end_frame() override;

		void on_attach() override;
		void on_detach() override;

		void on_update(Timestep delta_time) override;
		void on_event(Event& e) override;

		// add a texture to present in imgui
		// call it just one
		ImTextureID addTextureImpl(TextureHandle texture) override;

		void freeTextureImpl(ImTextureID texture) override;

	protected:
		void create_framebuffers();

		static void CheckVkError(VkResult err);

	protected:
		VkDescriptorPool m_descPool{ VK_NULL_HANDLE };

		VkDescriptorSetLayout m_imageSetLayout{ VK_NULL_HANDLE };

		VkRenderPass m_renderPass{ VK_NULL_HANDLE };

		std::vector<VulkanFramebufferHandle> m_framebuffers;
		// used to check whether the view is invalid
		// not holding it
		std::vector<VkImageView> m_swapchainViews;		
		uint32_t m_width, m_height;
	};

}
