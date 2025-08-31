#include "Transform.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace PaperEngine {
    PE_API void Transform::rotate(const glm::vec3& axis, float angle)
    {
        m_isDirty = true;
        this->m_rotation = glm::rotate(m_rotation, glm::radians(angle), axis);
    }

    Transform& Transform::operator=(const glm::mat4& matrix)
    {
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
        glm::vec3 skew;
        glm::vec4 perspective;
        bool success = glm::decompose(matrix, scale, rotation, position, skew, perspective);
        if (success) {
            m_isDirty = true;
            m_position = position;
            m_rotation = rotation;
            m_scale = scale;
        }

        return *this;
    }

    PE_API glm::vec3 Transform::getForward() const
    {
        update();
        return glm::normalize(m_rotation * glm::vec3(0, 0, -1));
    }

    PE_API glm::vec3 Transform::getRight() const
    {
        update();
        return glm::normalize(m_rotation * glm::vec3(1, 0, 0));
    }

    PE_API glm::vec3 Transform::getUp() const
    {
        update();
        return glm::normalize(m_rotation * glm::vec3(0, 1, 0));
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
