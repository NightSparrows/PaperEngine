#include "Entity.h"

#include <PaperEngine/components/TransformComponent.h>
#include <PaperEngine/components/MeshComponent.h>

namespace PaperEngine {

	Entity::Entity(entt::entity handle, Scene* scene) :
		m_handle(handle),
		m_scene(scene)
	{
	}

	void Entity::setPosition(const glm::vec3& position) {
		auto transCom = tryGetComponent<TransformComponent>();
		if (!transCom) {
			PE_CORE_ERROR("Try moving entity that has no transform component!");
			return;
		}
		transCom->transform.setPosition(position);

		auto meshCom = tryGetComponent<MeshComponent>();
		if (!meshCom) {
			return;			// just return
		}
		meshCom->worldAABB = meshCom->mesh->getAABB().transformed(transCom->transform);
	}

	void Entity::setTransform(const Transform& transform)
	{
		auto transCom = tryGetComponent<TransformComponent>();
		if (!transCom) {
			PE_CORE_ERROR("Try moving entity that has no transform component!");
			return;
		}
		transCom->transform = transform;

		auto meshCom = tryGetComponent<MeshComponent>();
		if (!meshCom) {
			return;			// just return
		}
		meshCom->worldAABB = meshCom->mesh->getAABB().transformed(transCom->transform);
	}

}
