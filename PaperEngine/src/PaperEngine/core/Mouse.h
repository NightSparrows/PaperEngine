#pragma once

#include "Base.h"
#include "MouseCodes.h"

#include <glm/glm.hpp>

namespace PaperEngine {

	namespace Mouse {

		PE_API bool IsMouseButtonDown(MouseCode button);

		PE_API void GrabMouseCursor(bool grab);

		PE_API glm::vec2 GetDeltaPosition();
	};

}
