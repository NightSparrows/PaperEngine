
#include <PaperEngine/scene/Scene.h>
#include <PaperEngine/scene/Entity.h>
#include <PaperEngine/component/TransformComponent.h>

namespace PaperEngine {
	Entity Scene::create_entity()
	{
		Entity entity = { m_registry.create(), this };
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
