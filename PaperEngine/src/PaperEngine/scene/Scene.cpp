#include "Scene.h"
#include "Entity.h"

#include <PaperEngine/components/TransformComponent.h>

namespace PaperEngine {

    Entity Scene::createEntity(const std::string& name)
    {
		Entity entity = { m_registry.create(), this };
        entity.addComponent<TransformComponent>();
        return entity;
    }

}
