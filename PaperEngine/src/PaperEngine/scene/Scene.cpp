
#include <PaperEngine/scene/Scene.h>
#include <PaperEngine/scene/Entity.h>
#include <PaperEngine/component/TransformComponent.h>
#include <PaperEngine/component/CameraComponent.h>
#include <PaperEngine/component/TagComponent.h>

namespace PaperEngine {
	Entity Scene::create_entity()
	{
		Entity entity = { m_registry.create(), this };
		entity.add_component<TagComponent>();
		// every entity need a transform component
		entity.add_component<TransformComponent>();
		entity.add_component<ChildComponent>();
		entity.add_component<ParentComponent>();
		return entity;
	}

	void Scene::destroy_entity(Entity entity)
	{
		PE_CORE_ASSERT(entity.get_handle() != entt::null, "Entity is null!");

		// remove this entity from its parent
		auto parentEntity = entity.get_component<ParentComponent>().parent;
		if (parentEntity.get_handle() != entt::null) {
			auto& children = parentEntity.get_component<ChildComponent>().children;
			children.erase(std::remove(children.begin(), children.end(), entity), children.end());
		}

		// remove children entities
		remove_children(entity);


		m_registry.destroy(entity.get_handle());
	}

	void Scene::on_resize(uint32_t width, uint32_t height)
	{
		auto view = m_registry.view<CameraComponent>();
		for (auto entity : view) {
			auto& camera = view.get<CameraComponent>(entity);
			if (camera.cameraType == CameraType::MainCamera) {
				camera.camera.set_viewport((float)width, (float)height);
			}
		}
	}

	void Scene::remove_children(Entity entity)
	{
		auto& children = entity.get_component<ChildComponent>().children;
		for (auto child : children) {
			remove_children(child);
			m_registry.destroy(child.get_handle());
		}
		children.clear();
	}
}
