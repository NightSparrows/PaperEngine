
#include "BoundingVolume.h"

namespace PaperEngine
{
    static glm::vec4 row(const glm::mat4& m, int i)
    {
        return glm::vec4(m[0][i], m[1][i], m[2][i], m[3][i]);
    }

    Frustum Frustum::Extract(const glm::mat4& vp)
    {
        Frustum f;
        glm::vec4 row0 = row(vp, 0);
        glm::vec4 row1 = row(vp, 1);
        glm::vec4 row2 = row(vp, 2);
        glm::vec4 row3 = row(vp, 3);

        f.planes[0] = row3 + row0; // Left
        f.planes[1] = row3 - row0; // Right
        f.planes[2] = row3 + row1; // Bottom
        f.planes[3] = row3 - row1; // Top
        f.planes[4] = row3 + row2; // Near
        f.planes[5] = row3 - row2; // Far

        for (auto& p : f.planes)
            p /= glm::length(glm::vec3(p));

        return f;
    }

    static bool Intersect(const Frustum& frustum, const AABB& aabb)
    {
        for (int i = 0; i < 6; i++)
        {
            const glm::vec3 normal(frustum.planes[i]);

            glm::vec3 positiveVertex = aabb.min;
            if (normal.x >= 0) positiveVertex.x = aabb.max.x;
            if (normal.y >= 0) positiveVertex.y = aabb.max.y;
            if (normal.z >= 0) positiveVertex.z = aabb.max.z;

            float distance = glm::dot(normal, positiveVertex) + frustum.planes[i].w;

            if (distance < -0.001f)
                return false;
        }

        return true;
    }

    static bool Intersect(const Frustum& f, const BoundingSphere& s)
    {
        for (auto& p : f.planes)
        {
            float distance = p.x * s.position.x + p.y * s.position.y + p.z * s.position.z + p.w;
            if (distance < -s.radius)
                return false;       // outside
        }
        return true;
    }

    bool Frustum::isIntersect(const BoundingVolume& other)
    {
        if (auto o = dynamic_cast<const AABB*>(&other))
            return Intersect(*this, *o);

        return false;
    }


    bool AABB::isIntersect(const BoundingVolume&)
    {
        return false;
    }

    bool BoundingSphere::isIntersect(const BoundingVolume& other)
    {
        if (auto o = dynamic_cast<const Frustum*>(&other))
            return Intersect(*o, *this);

        return false;
    }

}
