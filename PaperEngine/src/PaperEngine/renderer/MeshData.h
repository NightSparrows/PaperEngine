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
			uint32_t materialIndex;		// material index

			SubMeshData(uint32_t off, uint32_t cou, uint32_t materialIdx) {
				offset = off;
				count = cou;
				materialIndex = materialIdx;
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
		uint32_t boneCount{ 0 };								// if the mesh having bone

		std::vector<SubMeshData> subMeshData;

		std::vector<uint32_t> indexData;
		
	};

}
