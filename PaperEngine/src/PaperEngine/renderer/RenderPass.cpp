
#include <PaperEngine/renderer/RenderPass.h>

#include <Platform/Vulkan/VulkanRenderPass.h>

namespace PaperEngine {



	Ref<RenderPass> RenderPass::Create(const RenderPassCreateInfo& createInfo, uint32_t width, uint32_t height)
	{
		return CreateRef<VulkanRenderPass>(createInfo, width, height);
	}

}
