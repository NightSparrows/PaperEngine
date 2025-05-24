
#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>

#include <PaperEngine/component/TagComponent.h>
#include <PaperEngine/component/TransformComponent.h>
#include <PaperEngine/component/CameraComponent.h>
#include <PaperEngine/imgui/ImGuiUtils.h>

#include "SceneHierarchyPanel.h"

namespace PaperEngine {

	void SceneHierarchyPanel::on_imgui_render()
	{
		ImGui::Begin("Scene Hierarchy");

		if (m_context) {
			// list the entities that are in root of the scene
			m_context->get_registry().view<ParentComponent>().each([&](auto entity, ParentComponent& parentCom) {
				if (parentCom.parent)	// has parent which is not in root scene
					return;
				Entity e{ entity, m_context.get() };
				draw_entity_node(e);
				});
		}

		if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
			m_selectedEntity = Entity();
		}

		ImGui::End();

		ImGui::Begin("Properties");
		if (m_selectedEntity) {
			draw_components(m_selectedEntity);
		}
		ImGui::End();

	}

	void SceneHierarchyPanel::draw_entity_node(Entity entity)
	{
		auto& tag = entity.get_component<TagComponent>().name;
		auto& children = entity.get_component<ChildComponent>().children;

		ImGuiTreeNodeFlags flags = ((m_selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity.get_handle(), flags, tag.c_str());

		if (ImGui::IsItemClicked()) {
			m_selectedEntity = entity;
		}

		if (opened) {

			for (auto child : children) {
				draw_entity_node(child);
			}
			ImGui::TreePop();
		}


	}

	void SceneHierarchyPanel::draw_components(Entity entity)
	{
		if (entity.has_component<TagComponent>()) {
			auto& tag = entity.get_component<TagComponent>();
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.name.c_str());
			if (ImGui::InputText("##EntityTag", buffer, sizeof(buffer))) {
				tag.name = std::string(buffer);
			}
		}

		if (entity.has_component<TransformComponent>()) {
			auto& transform = entity.get_component<TransformComponent>().transform;

			if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform")) {
				ImGuiUtils::DrawVec3Control("Translation", transform.get_position());
				glm::vec3 euler = glm::degrees(glm::eulerAngles(transform.get_rotation()));
				if (ImGuiUtils::DrawVec3Control("Rotation", euler)) {
					if (euler.y >= 90.f) {
						euler.y = -90.f;
					}
					else if (euler.y <= -90.f) {
						euler.y = 90.f;
					}
					transform.set_rotation(glm::quat(glm::radians(euler)));
				}
				ImGuiUtils::DrawVec3Control("Scale", transform.get_scale());
				ImGui::TreePop();
			}
		}

		if (entity.has_component<CameraComponent>()) {
			auto& camera = entity.get_component<CameraComponent>().camera;

			if (ImGui::TreeNodeEx((void*)typeid(CameraComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Camera")) {

				ImGui::DragFloat("Fov", &camera.fov(), 0.1f, 1.f, 180.f, "%.1f");

				ImGui::DragFloat("Near", &camera.zNear(), 0.01f, 0.01f, 100.f);
				ImGui::DragFloat("Far", &camera.zFar(), 0.01f, 0.01f, 10000.f);

				ImGui::InputScalar("Camera Quality", ImGuiDataType_U32, &camera.get_camera_quality());

				ImGui::TreePop();
			}
		}
 	}

}
