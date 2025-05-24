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

		PE_API void on_resize(uint32_t width, uint32_t height);

		entt::registry& get_registry() { return m_registry; }

		const entt::registry& get_registry() const { return m_registry; }

	protected:
		void remove_children(Entity entity);

	private:
		entt::registry m_registry;

		entt::entity m_mainCameraEntity{ entt::null };

		friend class Entity;
	};

}
