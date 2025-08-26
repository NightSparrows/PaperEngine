#include "Scene.h"
#include "Entity.h"

#include <PaperEngine/components/TransformComponent.h>
#include <PaperEngine/components/TagComponent.h>
#include <PaperEngine/components/IDComponent.h>

namespace PaperEngine {

    Entity Scene::createEntity(const std::string& name)
    {
		Entity entity = { m_registry.create(), this };
        entity.addComponent<IDComponent>();
        entity.addComponent<TransformComponent>();
        entity.addComponent<TagComponent>().name = name.empty() ? "Entity" : name;
        return entity;
    }

}
