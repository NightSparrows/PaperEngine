#pragma once

#include <PaperEngine/scene/Scene.h>
#include <PaperEngine/scene/Entity.h>

namespace PaperEngine {

	class SceneHierarchyPanel {
	public:

		void set_context(Ref<Scene> scene) { m_context = scene; }

		void on_imgui_render();

	protected:
		void draw_entity_node(Entity entity);

		void draw_components(Entity entity);

	protected:

		Ref<Scene> m_context;

		Entity m_selectedEntity;

	};

}
