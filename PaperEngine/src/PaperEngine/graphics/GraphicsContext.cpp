#include "GraphicsContext.h"

#include <Platform/Vulkan/VulkanGraphicsContext.h>

#include <PaperEngine/core/Application.h>

namespace PaperEngine {

    Ref<GraphicsContext> GraphicsContext::Create(Window* window)
    {
		switch (Application::Get()->getRenderAPI())
		{
		case RenderAPI::Vulkan:
			return CreateRef<VulkanGraphicsContext>(window);
		default:
			break;
		}
		return nullptr;
    }

}
