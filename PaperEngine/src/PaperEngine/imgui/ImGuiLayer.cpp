#include "ImGuiLayer.h"

#include <PaperEngine/core/Application.h>

#include <Platform/Vulkan/VulkanImGuiLayer.h>

namespace PaperEngine {
	
	Ref<ImGuiLayer> ImGuiLayer::Create()
	{
		switch (Application::Get()->getRenderAPI())
		{
		case RenderAPI::Vulkan:
			return CreateRef<VulkanImGuiLayer>();
		default:
			break;
		}
		PE_CORE_ASSERT(false, "Unsupported RenderAPI for ImGuiLayer!");
		return NULL;
	}

}
