#pragma once

#include <PaperEngine/renderer/Camera.h>

#include <PaperEngine/renderer/Texture.h>

namespace PaperEngine {

	struct CameraComponent {

		CameraHandle camera;

		bool isAcive;				// whether this camera is active

		// it can be multiple primary camera in the scene
		// but only one camera can be activated at a time
		bool isPrimary;				// whether this is a primary camera there is only one primary camera activated in the scene

		TextureHandle targetImage;
	};

}
