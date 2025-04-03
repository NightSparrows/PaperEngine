#pragma once

#include <PaperEngine/core/Base.h>

namespace PaperEngine {

	class Mesh {
	public:
		enum MeshType {
			Static,				// only position, normal, texcoord
			Animated,			// with bones information
			Dynamic				// will be update in runtime (use static architecture)
		};

		struct SubMeshInfo {
			uint32_t offset;		// the offset of this submesh (per vertex)
			uint32_t size;			// the size of this submesh (per vertex)
		};

		Mesh(MeshType type) : m_type(type) {}
		virtual ~Mesh() = default;

		inline PE_API MeshType getType() const { return m_type; }

	protected:
		MeshType m_type;

		std::vector<SubMeshInfo> m_subMeshes;

	};

}
