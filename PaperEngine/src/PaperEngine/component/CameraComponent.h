#pragma once

#include <PaperEngine/renderer/Camera.h>

#include <PaperEngine/renderer/RenderTexture.h>

namespace PaperEngine {

	/// <summary>
	/// There's no primary camera for it
	/// just set by the editor (maybe)
	/// that render target to swapchain or other RenderTexture
	/// later can be use as the final scene
	/// </summary>
	struct CameraComponent {
		Camera camera;

		// whether this camera is active or not
		bool isAcive{ true };				

		/// <summary>
		/// If that scene can be render to the whole scene
		/// maybe just set the order to UINT32_MAX
		/// </summary>
		uint32_t cameraOrder{ 0 };		// small number render first
		// what texture render to
		RenderTextureHandle target;
	};

}
