#include "Scene.h"
#include "Entity.h"

#include <PaperEngine/components/TransformComponent.h>
#include <PaperEngine/components/TagComponent.h>

namespace PaperEngine {

    Entity Scene::createEntity(const std::string& name)
    {
		Entity entity = { m_registry.create(), this };
        entity.addComponent<TransformComponent>();
        entity.addComponent<TagComponent>().name = name.empty() ? "Entity" : name;
        return entity;
    }

}
