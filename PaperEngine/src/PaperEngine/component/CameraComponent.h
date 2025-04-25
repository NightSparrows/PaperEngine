#pragma once

#include <PaperEngine/renderer/Camera.h>

#include <PaperEngine/renderer/RenderTexture.h>

namespace PaperEngine {

	struct CameraComponent {
		Camera camera;

		bool isAcive{ true };				// whether this camera is active

		uint32_t cameraOrder{ 0 };		// small number render first
		// what texture render to
		RenderTextureHandle target;
	};

}
