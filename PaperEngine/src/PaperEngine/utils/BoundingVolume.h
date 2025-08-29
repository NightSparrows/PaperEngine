#pragma once

#include <PaperEngine/core/Application.h>

#include <glm/glm.hpp>

namespace PaperEngine {

	struct BoundingVolume
	{
		virtual bool isIntersect(const BoundingVolume&) = 0;
	};

    struct Frustum : public BoundingVolume {
        glm::vec4 planes[6];

        static Frustum Extract(const glm::mat4& viewProj);

        PE_API bool isIntersect(const BoundingVolume&);
    };

    struct BoundingSphere : public BoundingVolume
    {
        BoundingSphere(const glm::vec3& pos, float r) :
            position(pos), radius(r) { }

        PE_API bool isIntersect(const BoundingVolume&) override;

        glm::vec3 position;
        float radius;
    };

    struct AABB : public BoundingVolume {
        AABB() = default;

        AABB(const glm::vec3& min, const glm::vec3& max) :
            min(min), max(max)
        {
        }


        PE_API bool isIntersect(const BoundingVolume&) override;

        AABB transformed(const glm::mat4& transform) const
        {
            // AABB center / half extents
            glm::vec3 center = 0.5f * (min + max);
            glm::vec3 halfExtents = 0.5f * (max - min);

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

            AABB result(newCenter - newHalfExtents, newCenter + newHalfExtents);
            return result;
        }

        glm::vec3 min;
        glm::vec3 max;
    };

}
