#pragma once

#include <glm/glm.hpp>

#include "AABB.h"

namespace PaperEngine {

	struct Frustum {
		glm::vec4 planes[6];

		static Frustum Extract(const glm::mat4& viewProj);

		bool isAABBInFrustum(const AABB& aabb);

	};

}
