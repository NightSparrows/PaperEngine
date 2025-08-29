#pragma once

#include "PaperEngine/core/Base.h"
#include "PaperEngine/graphics/Light.h"

namespace PaperEngine {


	struct LightComponent
	{
		LightType type;
		bool castShadow;
		// TODO: Culling geometry class instance
		union
		{
			DirectionalLight directionalLight;
			PointLight pointLight;
		}light;
	};

}


