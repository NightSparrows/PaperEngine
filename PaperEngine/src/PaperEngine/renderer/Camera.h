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

		PE_API void update() const;

		void set_viewport(float width, float height) { m_aspectRatio = width / height; m_isDirty = true; }

		void set_camera_quality(uint32_t quality) { m_cameraQuality = quality; }

		const glm::mat4& get_projection_matrix() const { update(); return m_projectionMatrix; }

		uint32_t get_camera_quality() const { return m_cameraQuality; }

		uint32_t& get_camera_quality() { return m_cameraQuality; }

		float& fov() { m_isDirty = true; return m_fov; }

		float& zNear() { m_isDirty = true; return m_zNear; }

		float& zFar() { m_isDirty = true; return m_zFar; }

	private:
		mutable bool m_isDirty{ true };

		float m_fov{ 70.f };
		float m_aspectRatio{ 16.f / 9.f };
		float m_zNear{ 0.1f };
		float m_zFar{ 1000.f };

		mutable glm::mat4 m_projectionMatrix{ 1.f };

		// the quality value for renderer to decide the quality it will draw for this camera
		// 0 for the max quality
		uint32_t m_cameraQuality{ 0 };


		// TODO: target image 2d (where the scene to render)
		//Ref<RenderTarget> m_renderTarget;
		// render workflow (how to render)
		//Ref<RenderWorkflow> m_renderWorkflow;
	};

	typedef Ref<Camera> CameraHandle;
}
