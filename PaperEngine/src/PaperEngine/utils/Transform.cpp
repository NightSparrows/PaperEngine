#include "Transform.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace PaperEngine {
    PE_API void Transform::rotate(const glm::vec3& axis, float angle)
    {
        m_isDirty = true;
        this->m_rotation = glm::rotate(m_rotation, glm::radians(angle), axis);
    }
    PE_API const glm::mat4& Transform::matrix() const
    {
        update();
        return m_matrix;
    }
    PE_API Transform::operator const glm::mat4& () const
    {
        update();
        return m_matrix;
    }

    void Transform::update() const
    {
        if (!m_isDirty)
            return;


        glm::mat4 posMatrix = glm::translate(glm::mat4(1.f), this->m_position);
        glm::mat4 rotMatrix = glm::toMat4(this->m_rotation);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.f), this->m_scale);
        this->m_matrix = posMatrix * rotMatrix * scaleMatrix;
        this->m_isDirty = false;
    }

}
