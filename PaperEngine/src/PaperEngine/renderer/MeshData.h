#pragma once

#include <vector>

#include <glm/glm.hpp>

#include <PaperEngine/renderer/Mesh.h>

namespace PaperEngine {

	/// <summary>
	/// Do not contain material
	/// </summary>
	class MeshData {
	public:

		struct SubMeshData {
			uint32_t offset;			// offset of index
			uint32_t count;				// index count

			SubMeshData(uint32_t off, uint32_t cou) {
				offset = off;
				count = cou;
			}
		};

		struct BasicVertexData {
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 uv;
		};

		struct BoneVertexData
		{
			glm::ivec4 boneIndices;
			glm::vec4 weights;
		};
		
		MeshType type{ MeshType::Static };
		std::vector<BasicVertexData> basicVertexData;
		std::vector<BoneVertexData> boneVertexData;

		std::vector<SubMeshData> subMeshData;

		std::vector<uint32_t> indexData;
		
	};

}
