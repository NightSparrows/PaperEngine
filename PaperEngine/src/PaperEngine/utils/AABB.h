#pragma once

#include <glm/glm.hpp>

namespace PaperEngine {

	class AABB {
	public:
		AABB() = default;

		AABB(const glm::vec3& min, const glm::vec3& max) :
			m_min(min), m_max(max)
		{
		}

		AABB operator*(const glm::mat4& trans) const {
			AABB other;
			other.m_min = trans * glm::vec4(this->m_min, 1.f);
			other.m_max = trans * glm::vec4(this->m_max, 1.f);
			return other;
		}

		inline const glm::vec3& getMin() const { return m_min; }

		inline const glm::vec3& getMax() const { return m_max; }

	private:
		glm::vec3 m_min;
		glm::vec3 m_max;
	};

}
