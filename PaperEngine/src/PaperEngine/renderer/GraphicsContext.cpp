#include <PaperEngine/renderer/GraphicsContext.h>

#include <Platform/Vulkan/VulkanContext.h>

namespace PaperEngine {
	Scope<GraphicsContext> GraphicsContext::Create(void* window)
	{
		return CreateScope<VulkanContext>(static_cast<GLFWwindow*>(window));
	}
}