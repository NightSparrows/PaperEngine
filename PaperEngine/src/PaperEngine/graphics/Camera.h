#pragma once

#include <glm/glm.hpp>

namespace PaperEngine {

	/// <summary>
	/// 只包含ProjectionMatrix
	/// ViewMatrix由外部（如Transform component）
	/// </summary>
	class Camera {
	public:
		
		void setFov(float fov) {
			m_fov = fov;
			m_isDirty = true;
		}
		
		void setWidth(float width) {
			m_width = width;
			m_isDirty = true;
		}
		
		void setHeight(float height) {
			m_height = height;
			m_isDirty = true;
		}
		
		void setNearPlane(float nearPlane) {
			m_nearPlane = nearPlane;
			m_isDirty = true;
		}

		void setFarPlane(float farPlane) {
			m_farPlane = farPlane;
			m_isDirty = true;
		}


		const glm::mat4& getProjectionMatrix() const;

		float getFov() const {
			return m_fov;
		}

		float getNearPlane() const {
			return m_nearPlane;
		}

		float getFarPlane() const {
			return m_farPlane;
		}

		float getWidth() const {
			return m_width;
		}

		float getHeight() const {
			return m_height;
		}

	private:
		mutable bool m_isDirty{ true };

		float m_fov{ 70.f };
		float m_width{ 1280.f };
		float m_height{ 720.f };
		float m_nearPlane{ 0.1f };
		float m_farPlane{ 1000.f };

		mutable glm::mat4 m_projectionMatrix{ 1.f };

	};

}
