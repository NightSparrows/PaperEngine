
#include <imgui_internal.h>


#include "ImGuiUtils.h"



#include <PaperEngine/core/Application.h>
#include <backends/imgui_impl_vulkan.h>

// vulkan
#include <Platform/Vulkan/VulkanTexture.h>

#include "ImGuiLayer.h"

namespace PaperEngine {

    PE_API ImTextureID PaperEngine::ImGuiUtils::GetImGuiTexture(TextureHandle texture)
    {
        auto layerInstance = ImGuiLayer::GetInstance();
        if (!layerInstance)
            return 0;

		return layerInstance->addTextureImpl(texture);
    }

    PE_API void ImGuiUtils::FreeImGuiTexture(ImTextureID texture)
    {
        auto layerInstance = ImGuiLayer::GetInstance();
        if (!layerInstance)
            return;

        return layerInstance->freeTextureImpl(texture);
    }

    PE_API bool ImGuiUtils::DrawVec3Control(const std::string_view label, glm::vec3& values, float resetValue, float columnWidth, float dragSpeed, const char* format)
    {
        ImGui::PushID(label.data());
        bool changed = false;

        ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.data());
		ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
        if (ImGui::Button("X", buttonSize))
            values.x = resetValue;
		ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if (ImGui::DragFloat("##X", &values.x, 0.1f)) {
            changed = true;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
        if (ImGui::Button("Y", buttonSize))
            values.y = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if (ImGui::DragFloat("##Y", &values.y, 0.1f)) {
            changed = true;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.35f, 0.9f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
        if (ImGui::Button("Z", buttonSize))
            values.z = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if (ImGui::DragFloat("##Z", &values.z, 0.1f)) {
            changed = true;
        }
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();

        ImGui::Columns(1);

        ImGui::PopID();
        return changed;
    }

}
