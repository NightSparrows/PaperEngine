#pragma once

#include "Renderer.h"

namespace PaperEngine {

	class MeshRenderer : public Renderer {
	public:

		void prepareScene(const Scene& scene) override;

		void render() override;

	};

}
