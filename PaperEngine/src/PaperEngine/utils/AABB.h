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
        AABB transformed(const glm::mat4& transform) const
        {
            // AABB center / half extents
            glm::vec3 center = 0.5f * (m_min + m_max);
            glm::vec3 halfExtents = 0.5f * (m_max - m_min);

            // 轉換中心
            glm::vec3 newCenter = glm::vec3(transform * glm::vec4(center, 1.0f));

            // 取 3x3 上三角旋轉縮放矩陣
            glm::mat3 rotScale(
                glm::vec3(transform[0][0], transform[0][1], transform[0][2]),
                glm::vec3(transform[1][0], transform[1][1], transform[1][2]),
                glm::vec3(transform[2][0], transform[2][1], transform[2][2])
            );

            // 絕對值矩陣
            glm::mat3 absMat(
                glm::abs(rotScale[0]),
                glm::abs(rotScale[1]),
                glm::abs(rotScale[2])
            );

            // new half extents
            glm::vec3 newHalfExtents;
            newHalfExtents.x = glm::dot(absMat[0], halfExtents);
            newHalfExtents.y = glm::dot(absMat[1], halfExtents);
            newHalfExtents.z = glm::dot(absMat[2], halfExtents);

            AABB result;
            result.m_min = newCenter - newHalfExtents;
            result.m_max = newCenter + newHalfExtents;
            return result;
        }

		inline const glm::vec3& getMin() const { return m_min; }

		inline const glm::vec3& getMax() const { return m_max; }

	private:
		glm::vec3 m_min;
		glm::vec3 m_max;
	};

}
