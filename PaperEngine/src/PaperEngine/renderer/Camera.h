#pragma once

#include <PaperEngine/core/Base.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace PaperEngine {

	class Camera {
	public:

		Camera() = default;
		Camera(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
			: m_position(position), m_rotation(rotation), m_scale(scale) {
		}

		void update() {
			if (m_isDirty) {
				glm::mat4 transform = glm::translate(glm::mat4(1.f), m_position) *
					glm::toMat4(m_rotation) *
					glm::scale(glm::mat4(1.f), m_scale);

				m_viewMatrix = glm::inverse(transform);
				m_projectionMatrix = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_zNear, m_zFar);
				m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
				m_isDirty = false;
			}
		}

		void set_position(const glm::vec3& position) { m_position = position; m_isDirty = true; }
		void set_rotation(const glm::quat& rotation) { m_rotation = rotation; m_isDirty = true; }
		void set_scale(const glm::vec3& scale) { m_scale = scale; m_isDirty = true; }

		const glm::vec3& get_position() const { return m_position; }
		const glm::quat& get_rotation() const { return m_rotation; }
		const glm::vec3& get_scale() const { return m_scale; }

		const glm::mat4& get_view_matrix() const { return m_viewMatrix; }
		const glm::mat4& get_projection_matrix() const { return m_projectionMatrix; }
		const glm::mat4& get_view_projection_matrix() const { return m_viewProjectionMatrix; }

	private:
		bool m_isDirty{ true };

		float m_fov{ 70.f };
		float m_aspectRatio{ 16.f / 9.f };
		float m_zNear{ 0.1f };
		float m_zFar{ 1000.f };

		glm::vec3 m_position;
		glm::quat m_rotation;
		glm::vec3 m_scale{ 1.f, 1.f, 1.f };

		glm::mat4 m_viewMatrix{ 1.f };
		glm::mat4 m_projectionMatrix{ 1.f };
		glm::mat4 m_viewProjectionMatrix{ 1.f };

		// TODO: target image 2d (where the scene to render)
		//Ref<RenderTarget> m_renderTarget;
		// render workflow (how to render)
		//Ref<RenderWorkflow> m_renderWorkflow;
	};

	typedef Ref<Camera> CameraHandle;
}
