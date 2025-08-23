#include "Entity.h"

namespace PaperEngine {

	Entity::Entity(entt::entity handle, Scene* scene) :
		m_handle(handle),
		m_scene(scene)
	{
	}

}
