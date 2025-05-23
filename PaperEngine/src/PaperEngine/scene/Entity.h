#pragma once

#include <vector>

#include <PaperEngine/core/Logger.h>

#include "Scene.h"

namespace PaperEngine {

	class Entity {
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity&) = default;
		Entity& operator=(const Entity& other) = default;

		template<typename T, typename... Args>
		T& add_component(Args&&... args) {
			PE_CORE_ASSERT(!has_component<T>(), "Entity already has component");
			T& component = m_scene->m_registry.emplace<T>(m_handle, std::forward<Args>(args)...);
			// on component added
			return component;
		}

		template<typename T, typename... Args>
		T& add_or_replace_component(Args&&... args) {
			T& component = m_scene->m_registry.emplace_or_replace<T>(m_handle, std::forward<Args>(args)...);
			// on component added
			return component;
		}

		template<typename T>
		T& get_component() {
			PE_CORE_ASSERT(has_component<T>(), "Entity doesnt have component");
			return m_scene->m_registry.get<T>(m_handle);
		}

		template<typename T>
		T& get_component() const {
			PE_CORE_ASSERT(has_component<T>(), "Entity doesnt have component");
			return m_scene->m_registry.get<T>(m_handle);
		}

		template<typename T>
		T* try_get_component() {
			return m_scene->m_registry.try_get<T>(m_handle);
		}

		template<typename T>
		bool has_component() const {
			return m_scene->m_registry.try_get<T>(m_handle);
		}

		template<typename T>
		void remove_component()
		{
			PE_CORE_ASSERT(has_component<T>(), "Entity does not have component!");
			m_scene->m_registry.remove<T>(m_handle);
		}

		operator bool() const { return m_handle != entt::null; }

		bool operator==(const Entity& other) const {
			return (this->m_handle == other.m_handle) && (this->m_scene == other.m_scene);
		}

		bool operator!=(const Entity& other) const {
			return !(*this == other);
		}

		entt::entity get_handle() const { return m_handle; }

		glm::mat4 getWorldPosition() const;

	private:
		entt::entity m_handle{ entt::null };

		Scene* m_scene{ nullptr };
	};

	struct ChildComponent {
		std::vector<Entity> children;
	};

	struct ParentComponent {
		Entity parent{ entt::null, nullptr };
	};

}
