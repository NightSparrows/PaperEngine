#pragma once

#include <PaperEngine/renderer/Camera.h>

#include <PaperEngine/renderer/RenderTexture.h>

namespace PaperEngine {

	enum class CameraType {
		/// <summary>
		/// its will change the aspect ratio when the viewport is change
		/// </summary>
		MainCamera,				// the main camera, render to the swapchain
		/// <summary>
		/// You must control size manually
		/// </summary>
		RenderTexture			// render to a render texture
	};

	/// <summary>
	/// There's no primary camera for it
	/// just set by the editor (maybe)
	/// that render target to swapchain or other RenderTexture
	/// later can be use as the final scene
	/// </summary>
	struct CameraComponent {
		Camera camera;

		void setAsMainCamera() {
			cameraType = CameraType::MainCamera;
			cameraOrder = UINT32_MAX;
		}

		// whether this camera is active or not
		bool isAcive{ true };				

		CameraType cameraType{ CameraType::MainCamera };

		/// <summary>
		/// If that scene can be render to the whole scene
		/// maybe just set the order to UINT32_MAX
		/// </summary>
		uint32_t cameraOrder{ 0 };		// small number render first
		// what texture render to
		// I think no swapchain buffer render directly to anymore
		// because the editor and runtime just very complex to seperate
		RenderTextureHandle target;
	};

}
