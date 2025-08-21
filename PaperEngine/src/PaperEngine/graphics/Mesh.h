#pragma once

#include <vector>

#include <nvrhi/nvrhi.h>

namespace PaperEngine {

	enum class MeshType {
		/// <summary>
		/// vertex format {
		///		vec3 position;
		///		vec3 normal;
		///		vec2 texcoord;
		/// }
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
		};

		// 編輯Mesh的Submesh
		std::vector<SubMeshInfo>& getSubMeshes() { return m_subMeshes; }

		/// <summary>
		/// 載入Raw資料到Mesh中，因為不需要Format
		/// 速度是最快的
		/// 這個method會自行allocate command buffer並執行資料上傳
		/// </summary>
		/// <param name="data"></param>
		/// <param name="size"></param>
		/// <param name="indexOffset"></param>
		void loadRawData(const void* data, uint32_t size, uint32_t indexOffset);

		/// <summary>
		/// 載入Raw資料到Mesh中，因為不需要Format
		/// 速度是最快的
		/// 需要給予command list來record上傳命令
		/// </summary>
		/// <param name="cmd"></param>
		/// <param name="data"></param>
		/// <param name="size"></param>
		/// <param name="indexOffset"></param>
		void loadRawDataAsync(nvrhi::CommandListHandle cmd, const void* data, uint32_t size, uint32_t indexOffset);

		/// <summary>
		/// 設定Mesh Type
		/// 會依據該Type來決定Mesh的格式處理方式
		/// </summary>
		/// <param name="type"></param>
		void setType(MeshType type) { m_type = type; }
		MeshType getType() const { return m_type; }

		void setIndexBufferOffset(uint32_t offset) { m_indexBufferOffset = offset; }
		uint32_t getIndexBufferOffset() const { return m_indexBufferOffset; }


	private:

		/// Buffer結構
		///     "Vertices"
		///		"Indices"
		nvrhi::BufferHandle m_buffer;

		/// <summary>
		/// 主要是用來區分materials
		/// </summary>
		std::vector<SubMeshInfo> m_subMeshes;
		MeshType m_type = MeshType::Static;
		// Index 資料在buffer的偏移量
		uint32_t m_indexBufferOffset = 0;
	};

}
