#pragma once

#include <glm/glm.hpp>

namespace PaperEngine {

	typedef enum class LightType {
		PointLight,
		SpotLight,
		DirectionLight
	}LightType;

	struct LightComponent {
		LightType type;
		glm::vec3 color;
		float range;
	};

}

