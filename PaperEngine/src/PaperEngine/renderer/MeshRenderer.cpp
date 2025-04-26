#include "MeshRenderer.h"

#include <PaperEngine/core/Application.h>
#include <PaperEngine/component/MeshComponent.h>
#include <Platform/Vulkan/VulkanMeshRenderer.h>

namespace PaperEngine {

	PE_API Ref<MeshRenderer> MeshRenderer::Create()
	{
		return CreateRef<VulkanMeshRenderer>();
	}

}
