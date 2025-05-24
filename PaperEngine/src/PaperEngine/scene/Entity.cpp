
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

	void Entity::setParent(const Entity& newParent)
	{
		if (newParent.get_scene() != this->get_scene()) {
			PE_CORE_ERROR("the parent is in different scene!");
			return;
		}

		auto& parentCom = this->get_component<ParentComponent>();

		if (newParent == parentCom.parent) {
			// already the same parent
			return;
		}

		if (parentCom.parent) {
			parentCom.parent.remove_child(*this);
		}

		if (newParent) {
			newParent.get_component<ChildComponent>().children.push_back(*this);
		}
		parentCom.parent = newParent;
	}

	void Entity::remove_child(const Entity& child)
	{
		auto& childrenList = this->get_component<ChildComponent>().children;
		auto it = std::find(childrenList.begin(), childrenList.end(), child);
		if (it != childrenList.end()) {
			childrenList.erase(it);
		}
		else {
			PE_CORE_ERROR("Child not found in parent entity!");
		}
	}

}
