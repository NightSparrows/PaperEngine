#include "Scene.h"
#include "Entity.h"
namespace PaperEngine {

    Entity Scene::createEntity(const std::string& name)
    {
		Entity entity = { m_registry.create(), this };
        return entity;
    }

}
