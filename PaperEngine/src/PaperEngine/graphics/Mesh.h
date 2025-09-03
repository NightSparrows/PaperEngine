#pragma once

#include <vector>

#include <PaperEngine/core/Base.h>
#include <PaperEngine/utils/BoundingVolume.h>


#include <nvrhi/nvrhi.h>
#include <glm/glm.hpp>

namespace PaperEngine {

	struct StaticVertex {
		glm::vec3 position; // vec3 position
		glm::vec3 normal;   // vec3 normal
		glm::vec2 texcoord; // vec2 texcoord
	};

	struct SkeletalVertexInfo {
		glm::ivec4 boneIndices = glm::ivec4(0); // ivec4 boneIndices
		glm::vec4 boneWeights = glm::vec4(0); // vec4 boneWeights
	};

	enum class MeshType {
		/// <summary>
		/// vertex format {
		///		vec3 position;
		///		vec3 normal;
		///		vec2 texcoord;
		/// }
		/// 並且如果使用更新就會重新allocate
		/// </summary>
		Static,
		/// <summary>
		/// vertex format {
		///		vec3 position;
		///		vec3 normal;
		///		vec2 texcoord;
		///		ivec4 boneIndices; // 骨骼索引
		///		vec4 boneWeights; // 骨骼權重
		/// }
		/// </summary>
		Skeletal
	};

	/// <summary>
	/// </summary>
	class Mesh {
	public:
		struct SubMeshInfo {
			uint32_t indicesOffset = 0;
			uint32_t indicesCount = 0;
			uint32_t materialIndex = 0;			// 這個subMesh使用什麼material
		};

		// 編輯Mesh的Submesh
		PE_API std::vector<SubMeshInfo>& getSubMeshes() { return m_subMeshes; }

		PE_API void loadStaticMesh(
			nvrhi::CommandListHandle cmdList, 
			const std::vector<StaticVertex>& vertices);

		PE_API void loadSkeletalMesh(
			nvrhi::CommandListHandle cmdList, 
			const std::vector<StaticVertex>& vertices,
			const std::vector<SkeletalVertexInfo>& boneInfos);

		PE_API void loadIndexBuffer(nvrhi::CommandListHandle cmdList, const void* indicesData, size_t indicesCount, nvrhi::Format type = nvrhi::Format::R32_UINT);

		/// <summary>
		/// Bind this Mesh
		/// no submesh information
		/// 使用mesh畫的順序為
		/// bindMesh
		///  for each submesh
		///		bindSubMesh
		///		for each instances
		///			drawIndexed
		/// </summary>
		/// <param name="state"></param>
		/// <param name="drawArgs"></param>
		void bindMesh(nvrhi::GraphicsState& state) const;

		void bindSubMesh(nvrhi::DrawArguments& drawArgs, uint32_t subMeshIndex) const;

		/// <summary>
		/// 設定Mesh Type
		/// 會依據該Type來決定Mesh的格式處理方式
		/// </summary>
		/// <param name="type"></param>
		PE_API MeshType getType() const { return m_type; }

		inline PE_API const AABB& getAABB() const { return m_aabb; }

	private:

		AABB m_aabb;

		nvrhi::BufferHandle m_vertexBuffer;

		nvrhi::Format m_indexFormat = nvrhi::Format::R32_UINT; // 預設為32位元整數索引格式
		nvrhi::BufferHandle m_indexBuffer;

		nvrhi::BufferHandle m_boneBuffer;

		/// <summary>
		/// 主要是用來區分materials
		/// </summary>
		std::vector<SubMeshInfo> m_subMeshes;
		MeshType m_type = MeshType::Static;
	};

	typedef Ref<Mesh> MeshHandle;

}
