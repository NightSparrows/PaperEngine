#pragma once

#include <glm/glm.hpp>

namespace PaperEngine {

	struct Light {
		glm::vec3 color;
	};

	struct DirectionalLight : public Light {

	};

}
