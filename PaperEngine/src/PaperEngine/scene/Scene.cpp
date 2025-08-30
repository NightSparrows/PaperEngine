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

    PE_API Entity Scene::createEntity(const std::string& name, UUID uuid)
    {
        Entity entity = { m_registry.create(), this };
        auto& idCom = entity.addComponent<IDComponent>();
        idCom.id = uuid;
        entity.addComponent<TransformComponent>();
        entity.addComponent<TagComponent>().name = name.empty() ? "Entity" : name;
        return entity;
    }

}
