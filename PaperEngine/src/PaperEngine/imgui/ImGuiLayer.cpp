#include "ImGuiLayer.h"

#include <PaperEngine/imgui/VulkanImGuiLayer.h>

namespace PaperEngine {

	Ref<ImGuiLayer> ImGuiLayer::Create() {
		return CreateRef<VulkanImGuiLayer>();
	}

}
