
#include <PaperEngine/scene/Scene.h>
#include <PaperEngine/scene/Entity.h>

namespace PaperEngine {
	Entity Scene::create_entity()
	{
		Entity entity = { m_registry.create(), this };
		return entity;
	}

	void Scene::destroy_entity(Entity entity)
	{
		m_registry.destroy(entity);
	}
}
