#include "Camera.h"

#include <PaperEngine/core/Application.h>
namespace PaperEngine {

	void Camera::update() const
	{
		if (m_isDirty) {
			m_projectionMatrix = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_zNear, m_zFar);

			GraphicsAPI api = Application::Get().get_window().get_context().getGraphicsAPI();
			switch (api)
			{
			case PaperEngine::GraphicsAPI::None:
				break;
			case PaperEngine::GraphicsAPI::Vulkan:
				m_projectionMatrix[1][1] *= -1.0f; // ← This would invert the Y axis! (for vulkan)
				break;
			default:
				break;
			}

			m_isDirty = false;
		}
	}

}
