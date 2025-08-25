
#include <PaperEngine/core/Base.h>
#include <PaperEngine/core/Application.h>

#include "Mesh.h"


namespace PaperEngine {

	template<typename VertexType, typename TransformFunc>
	AABB computeAABB(const std::vector<VertexType>& vertices, TransformFunc getPos)
	{
		if (vertices.empty())
			return { glm::vec3(0.0f), glm::vec3(0.0f) };

		glm::vec3 minPt(FLT_MAX);
		glm::vec3 maxPt(-FLT_MAX);

		for (const auto& v : vertices)
		{
			glm::vec3 pos = getPos(v);
			minPt = glm::min(minPt, pos);
			maxPt = glm::max(maxPt, pos);
		}

		return { minPt, maxPt };
	}

	void Mesh::loadStaticMesh(nvrhi::CommandListHandle cmdList, const std::vector<StaticVertex>& vertices)
	{
		auto device = Application::Get()->getGraphicsContext()->getNVRhiDevice();

		m_type = MeshType::Static;

		nvrhi::BufferDesc vertexBufferDesc;
		vertexBufferDesc
			.setByteSize(vertices.size() * sizeof(StaticVertex))
			.setDebugName("StaticMeshVertexBuffer")
			.setInitialState(nvrhi::ResourceStates::CopyDest)
			.setIsVertexBuffer(true);
		m_vertexBuffer = device->createBuffer(vertexBufferDesc);

		m_aabb = computeAABB<StaticVertex>(vertices, [](const StaticVertex& v) {
			return v.position;
			});

		cmdList->beginTrackingBufferState(m_vertexBuffer, nvrhi::ResourceStates::CopyDest);
		cmdList->writeBuffer(m_vertexBuffer, vertices.data(), vertexBufferDesc.byteSize);
		cmdList->setPermanentBufferState(m_vertexBuffer, nvrhi::ResourceStates::VertexBuffer);
	}

	void Mesh::loadSkeletalMesh(nvrhi::CommandListHandle cmdList, const std::vector<SkeletalVertex>& vertices)
	{
		auto device = Application::Get()->getGraphicsContext()->getNVRhiDevice();

		m_type = MeshType::Skeletal;
		
		nvrhi::BufferDesc vertexBufferDesc;
		vertexBufferDesc
			.setByteSize(vertices.size() * sizeof(SkeletalVertex))
			.setDebugName("SkeletalMeshVertexBuffer")
			.setInitialState(nvrhi::ResourceStates::CopyDest)
			.setIsVertexBuffer(true);
		m_vertexBuffer = device->createBuffer(vertexBufferDesc);

		m_aabb = computeAABB<SkeletalVertex>(vertices, [](const SkeletalVertex& v) {
			return v.position;
			});

		cmdList->beginTrackingBufferState(m_vertexBuffer, nvrhi::ResourceStates::CopyDest);
		cmdList->writeBuffer(m_vertexBuffer, vertices.data(), vertexBufferDesc.byteSize);
		cmdList->setPermanentBufferState(m_vertexBuffer, nvrhi::ResourceStates::VertexBuffer);
	}

	void Mesh::loadIndexBuffer(nvrhi::CommandListHandle cmdList, const void* indicesData, size_t indicesCount, nvrhi::Format type)
	{
		auto device = Application::Get()->getGraphicsContext()->getNVRhiDevice();

		m_indexFormat = type;
		
		nvrhi::BufferDesc indexBufferDesc;
		indexBufferDesc
			.setByteSize(indicesCount * (type == nvrhi::Format::R16_UINT ? sizeof(uint16_t) : sizeof(uint32_t)))
			.setDebugName("MeshIndexBuffer")
			.setInitialState(nvrhi::ResourceStates::CopyDest)
			.setIsIndexBuffer(true);
		m_indexBuffer = device->createBuffer(indexBufferDesc);
		cmdList->beginTrackingBufferState(m_indexBuffer, nvrhi::ResourceStates::CopyDest);
		cmdList->writeBuffer(m_indexBuffer, indicesData, indexBufferDesc.byteSize);
		cmdList->setPermanentBufferState(m_indexBuffer, nvrhi::ResourceStates::IndexBuffer);
	}

	void Mesh::bindMesh(nvrhi::GraphicsState& state) const
	{
		state.indexBuffer.buffer = m_indexBuffer;
		state.indexBuffer.format = m_indexFormat;
		state.indexBuffer.offset = 0;
		switch (m_type)
		{
		case PaperEngine::MeshType::Static:
		{
			state.vertexBuffers = {
				{ m_vertexBuffer, 0, offsetof(StaticVertex, position) },
				{ m_vertexBuffer, 1, offsetof(StaticVertex, normal) },
				{ m_vertexBuffer, 2, offsetof(StaticVertex, texcoord) }
			};
		}
			break;
		case PaperEngine::MeshType::Skeletal:
		{
			state.vertexBuffers = {
				{ m_vertexBuffer, 0, offsetof(SkeletalVertex, position) },
				{ m_vertexBuffer, 1, offsetof(SkeletalVertex, normal) },
				{ m_vertexBuffer, 2, offsetof(SkeletalVertex, texcoord) },
				{ m_vertexBuffer, 3, offsetof(SkeletalVertex, boneIndices) },
				{ m_vertexBuffer, 4, offsetof(SkeletalVertex, boneWeights) }
			};
		}
			break;
		default:
			break;
		}
	}

	void Mesh::bindSubMesh(nvrhi::DrawArguments& drawArgs, uint32_t subMeshIndex) const
	{
		if (subMeshIndex >= m_subMeshes.size())
		{
			PE_CORE_ERROR("SubMesh index out of range: {}", subMeshIndex);
			return;
		}
		const SubMeshInfo& subMesh = m_subMeshes[subMeshIndex];
		drawArgs.vertexCount = subMesh.indicesCount;
		drawArgs.startIndexLocation = subMesh.indicesOffset;
		drawArgs.startVertexLocation = 0;
		drawArgs.startInstanceLocation = 0;
	}

}
