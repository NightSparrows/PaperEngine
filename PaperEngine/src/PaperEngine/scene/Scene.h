#pragma once

#include <entt/entt.hpp>

#include <PaperEngine/core/Base.h>
#include <PaperEngine/core/UUID.h>

namespace PaperEngine {

	class Entity;

	class Scene {
	public:
		PE_API Scene() = default;

		PE_API Entity createEntity(const std::string& name);

		PE_API Entity createEntity(const std::string& name, UUID uuid);

		entt::registry& getRegistry() { return m_registry; }

	private:
		entt::registry m_registry; // 使用entt的registry來管理實體和組件
	};

}
