#include "ImGuiLayer.h"

#include <PaperEngine/core/Application.h>

#include <PaperEngine/imgui/VulkanImGuiLayer.h>

namespace PaperEngine {

	std::weak_ptr<ImGuiLayer> ImGuiLayer::s_instance;

	Ref<ImGuiLayer> ImGuiLayer::Create() {
		GraphicsAPI api = Application::Get().get_window().get_context().getGraphicsAPI();
		switch (api)
		{
		case PaperEngine::GraphicsAPI::None:
			break;
		case PaperEngine::GraphicsAPI::Vulkan:
		{
			auto layer = CreateRef<VulkanImGuiLayer>();
			s_instance = layer;
			return layer;
		}
		default:
			break;
		}

		return nullptr;
	}

	std::shared_ptr<ImGuiLayer> ImGuiLayer::GetInstance()
	{
		auto instance = s_instance.lock();
		if (!instance) {
			PE_CORE_ASSERT(false, "ImGuiLayer not created");
			return nullptr;
		}
		return instance;
	}

}
