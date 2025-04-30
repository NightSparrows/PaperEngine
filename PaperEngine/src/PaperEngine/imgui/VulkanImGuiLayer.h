#pragma once

#include "ImGuiLayer.h"

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

#include <Platform/Vulkan/VulkanContext.h>

namespace PaperEngine {

	class VulkanImGuiLayer : public ImGuiLayer {
	public:

		bool begin_frame() override;

		void end_frame() override;

		void on_attach() override;
		void on_detach() override;

		void on_update(Timestep delta_time) override;
		void on_event(Event& e) override;

	protected:
		void create_framebuffers();

	protected:
		VkDescriptorPool m_descPool{ VK_NULL_HANDLE };

		VkRenderPass m_renderPass{ VK_NULL_HANDLE };

		std::vector<VkFramebuffer> m_framebuffers;
		uint32_t m_width, m_height;
	};

}
