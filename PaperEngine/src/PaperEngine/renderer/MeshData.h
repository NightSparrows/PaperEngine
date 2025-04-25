#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace PaperEngine {

	/// <summary>
	/// Do not contain material
	/// </summary>
	class MeshData {
	public:
		typedef enum MeshType {
			Static,
			Animated
		}MeshType;

		struct SubMeshData {
			uint32_t offset;			// offset of index
			uint32_t count;				// index count
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
		
		MeshType type{ Static };
		std::vector<BasicVertexData> basicVertexData;
		std::vector<BoneVertexData> boneVertexData;

		std::vector<uint32_t> indexData;
		std::vector<SubMeshData> subMeshData;
		
	};

}
