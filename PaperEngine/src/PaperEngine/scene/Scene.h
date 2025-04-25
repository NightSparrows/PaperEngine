#pragma once

#include <PaperEngine/core/Base.h>

#include <entt/entt.hpp>

#include <PaperEngine/core/Timestep.h>

#include <PaperEngine/renderer/Camera.h>

namespace PaperEngine {

	class Entity;

	class Scene {
	public:
		PE_API Scene() = default;

		PE_API Entity create_entity();

		PE_API void destroy_entity(Entity entity);

		void on_update(Timestep delta_time) {}

		entt::registry& get_registry() { return m_registry; }

		const entt::registry& get_registry() const { return m_registry; }

	private:
		entt::registry m_registry;

		Ref<Camera> m_activeCamera;

		friend class Entity;
	};

}
