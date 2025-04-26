
#include <PaperEngine/scene/Scene.h>
#include <PaperEngine/scene/Entity.h>
#include <PaperEngine/component/TransformComponent.h>

namespace PaperEngine {
	Entity Scene::create_entity()
	{
		Entity entity = { m_registry.create(), this };
		// every entity need a transform component
		entity.add_component<TransformComponent>();
		return entity;
	}

	void Scene::destroy_entity(Entity entity)
	{
		m_registry.destroy(entity);
	}
}
