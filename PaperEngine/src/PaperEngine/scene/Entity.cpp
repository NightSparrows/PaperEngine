
#include "Entity.h"
#include <PaperEngine/component/TransformComponent.h>

namespace PaperEngine {

	Entity::Entity(entt::entity handle, Scene* scene) :
	m_handle(handle), m_scene(scene)
	{

	}

	glm::mat4 Entity::getWorldPosition() const
	{
		auto& transCom = this->get_component<TransformComponent>();
		glm::mat4 result = transCom.transform.matrix();
		while (true) {
			Entity parentEntity = this->get_component<ParentComponent>().parent;

			if (parentEntity.m_handle == entt::null)
				break;

			glm::mat4 parentTransform = parentEntity.get_component<TransformComponent>().transform.matrix();
			result = parentTransform * result;
		}

		return result;
	}

}
