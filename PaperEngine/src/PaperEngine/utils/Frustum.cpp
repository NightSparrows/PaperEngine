#include "Frustum.h"

namespace PaperEngine {

    static glm::vec4 row(const glm::mat4& m, int i)
    {
        return glm::vec4(m[0][i], m[1][i], m[2][i], m[3][i]);
    }

    Frustum Frustum::Extract(const glm::mat4& vp)
    {
        Frustum f;

        f.planes[0] = row(vp, 3) + row(vp, 0); // Left
        f.planes[1] = row(vp, 3) - row(vp, 0); // Right
        f.planes[2] = row(vp, 3) + row(vp, 1); // Bottom
        f.planes[3] = row(vp, 3) - row(vp, 1); // Top
        f.planes[4] = row(vp, 3) + row(vp, 2); // Near
        f.planes[5] = row(vp, 3) - row(vp, 2); // Far

        // normalize planes
        for (auto& p : f.planes)
            p /= glm::length(glm::vec3(p));

        return f;
    }

    bool Frustum::isAABBInFrustum(const AABB& aabb)
    {
        for (int i = 0; i < 6; i++)
        {
            const glm::vec3 normal(this->planes[i]);

            glm::vec3 positiveVertex = aabb.getMin();
            if (normal.x >= 0) positiveVertex.x = aabb.getMax().x;
            if (normal.y >= 0) positiveVertex.y = aabb.getMax().y;
            if (normal.z >= 0) positiveVertex.z = aabb.getMax().z;

            float distance = glm::dot(normal, positiveVertex) + this->planes[i].w;

            if (distance < 0)
                return false;
        }

        return true;
    }

}
