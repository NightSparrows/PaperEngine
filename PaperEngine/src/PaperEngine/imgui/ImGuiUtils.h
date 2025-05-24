#pragma once

#include <imgui.h>
#include <PaperEngine/renderer/Texture.h>

#include <glm/glm.hpp>

namespace PaperEngine {

	class ImGuiUtils {
	public:

		PE_API static ImTextureID GetImGuiTexture(TextureHandle texture);

		PE_API static void FreeImGuiTexture(ImTextureID texture);

		PE_API static bool DrawVec3Control(const std::string_view label, glm::vec3& values, float resetValue = 0.0f,
			float columnWidth = 100.0f,
			float dragSpeed = 0.1f,
			const char* format = "%.3f");

	};

}
