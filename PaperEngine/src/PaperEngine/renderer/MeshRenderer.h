#pragma once

#include "Renderer.h"
#include "Material.h"
#include "Mesh.h"

namespace PaperEngine {

	class MeshRenderer : public Renderer {
	public:

		PE_API static Ref<MeshRenderer> Create();

	};

}
