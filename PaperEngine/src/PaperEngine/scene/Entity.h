#pragma once

#include <glm/glm.hpp>

#include <entt/entt.hpp>

#include <PaperEngine/core/Base.h>

#include <PaperEngine/scene/Scene.h>

#include <PaperEngine/utils/Transform.h>

namespace PaperEngine {

	class Entity {
		friend class Scene; // 讓Scene可以訪問Entity的私有成員
	public:
		PE_API Entity() = default;
		PE_API Entity(entt::entity handle, Scene* scene);

		PE_API void setPosition(const glm::vec3& position);

		PE_API void setTransform(const Transform& transform);

		template<typename T, typename... Args>
		T& addComponent(Args&&... args) {
			return m_scene->getRegistry().emplace<T>(m_handle, std::forward<Args>(args)...);
		}

		template<typename T>
		void removeComponent() {
			m_scene->getRegistry().remove<T>(m_handle);
		}

		template<typename T>
		T& getComponent() const {
			return m_scene->getRegistry().get<T>(m_handle);
		}

		template<typename T>
		T* tryGetComponent() const {
			return m_scene->getRegistry().try_get<T>(m_handle);
		}

		template<typename T>
		bool hasComponent() const {
			return m_scene->getRegistry().try_get<T>(m_handle);
		}

	private:
		entt::entity m_handle{ entt::null }; // entt的實體句柄
		Scene* m_scene{ nullptr }; // 所屬場景


	};

}
