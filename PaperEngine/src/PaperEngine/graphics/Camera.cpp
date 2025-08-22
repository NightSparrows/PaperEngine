#include "Camera.h"

#include <PaperEngine/core/Application.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace PaperEngine {

    const glm::mat4& Camera::getProjectionMatrix() const
    {
		if (m_isDirty)
		{
			m_projectionMatrix = glm::perspective(glm::radians(m_fov), m_width / m_height, m_nearPlane, m_farPlane);
			if (Application::Get()->getRenderAPI() == RenderAPI::Vulkan) {
				m_projectionMatrix[1][1] = -m_projectionMatrix[1][1]; // Vulkan uses a different Y-axis convention
			}
			m_isDirty = false;
		}
		return m_projectionMatrix;
    }

}
