#pragma once

#include <glm/glm.hpp>

namespace PaperEngine {

	enum class LightType {
		Directional,
		Point,
		Spot
	};

	struct Light {
		glm::vec3 color;
	};

	struct DirectionalLight : public Light {
		glm::vec3 direction;
	};

	struct PointLight : public Light {
		float radius;
	};

}
