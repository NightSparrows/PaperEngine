#include "LightCullingPass.h"
#include <PaperEngine/core/Application.h>
#include <PaperEngine/utils/File.h>

#include <PaperEngine/debug/Instrumentor.h>

namespace PaperEngine {

	void LightCullingPass::init() {
		nvrhi::CommandListParameters cmdParams;
		cmdParams.setQueueType(nvrhi::CommandQueue::Compute);
		m_cmd = Application::GetNVRHIDevice()->createCommandList(cmdParams);

		m_computeQuery = Application::GetNVRHIDevice()->createEventQuery();

		{
			nvrhi::BufferDesc bufferDesc;
			bufferDesc
				.setDebugName("Directional Light Buffer")
				.setByteSize(m_maxDirectionalLight * sizeof(DirectionalLightData))
				.setCpuAccess(nvrhi::CpuAccessMode::Write)
				.setIsConstantBuffer(true)
				.setKeepInitialState(true)
				.setStructStride(sizeof(DirectionalLightData));
			m_directionalLightBuffer = CreateRef<GPUBuffer>(IResource::Usage::FrameStreaming, bufferDesc);
		}

#pragma region Initialize Point buffers
		{
			nvrhi::BufferDesc bufferDesc;
			bufferDesc
				.setDebugName("Point Light Buffer")
				.setByteSize(m_maxPointLight * sizeof(PointLightData))
				.setCpuAccess(nvrhi::CpuAccessMode::Write)
				.setIsConstantBuffer(true)
				.setKeepInitialState(true)
				.setStructStride(sizeof(PointLightData));
			m_pointLightBuffer = CreateRef<GPUBuffer>(IResource::Usage::FrameStreaming, bufferDesc);

		}
#pragma endregion

#pragma region Light Cull Binding Layout Creation

		nvrhi::BindingLayoutDesc lightCullBindingLayoutDesc;
		lightCullBindingLayoutDesc
			.setVisibility(nvrhi::ShaderType::Compute)
			.setRegisterSpace(0)
			.setRegisterSpaceIsDescriptorSet(true)
			.addItem(nvrhi::BindingLayoutItem::ConstantBuffer(0))	// Global Data buffer, Constant buffer
			.addItem(nvrhi::BindingLayoutItem::StructuredBuffer_SRV(0))		// Shader Resource
			.addItem(nvrhi::BindingLayoutItem::StructuredBuffer_UAV(0))		// global light indices
			.addItem(nvrhi::BindingLayoutItem::StructuredBuffer_UAV(1))		// cluster range
			.addItem(nvrhi::BindingLayoutItem::RawBuffer_UAV(2));			// global counter

		m_lightCullBindingLayout = Application::GetNVRHIDevice()->createBindingLayout(lightCullBindingLayoutDesc);
#pragma endregion

#pragma region Global data GPU Buffer
		{
			nvrhi::BufferDesc bufferDesc;
			bufferDesc
				.setDebugName("LightCullGlobalDataBuffer")
				.setByteSize(sizeof(GlobalData))
				.setCpuAccess(nvrhi::CpuAccessMode::Write)
				.setIsConstantBuffer(true)
				.setKeepInitialState(true);
			m_pointLightCullData.globalDataBuffer = CreateRef<GPUBuffer>(IResource::Usage::FrameStreaming, bufferDesc);
		}
#pragma endregion

#pragma region Global Light Indices Buffer Creation
		{
			nvrhi::BufferDesc bufferDesc;
			bufferDesc
				.setDebugName("Global Light Indices Buffer")
				.setKeepInitialState(true)
				.setByteSize(
					m_numberOfXSlices *
					m_numberOfYSlices *
					m_numberOfZSlices *
					m_maxPointLightPerCluster * sizeof(uint32_t))
				.setStructStride(sizeof(uint32_t))
				.setCanHaveUAVs(true)
				.setCpuAccess(nvrhi::CpuAccessMode::None);
			m_pointLightCullData.globalLightIndicesBuffer = CreateRef<GPUBuffer>(IResource::Usage::FrameStatic, bufferDesc);
		}
#pragma endregion

#pragma region Cluster Ranges Buffer Creation
		{
			nvrhi::BufferDesc bufferDesc;
			bufferDesc
				.setDebugName("Cluster Ranges Buffer")
				.setKeepInitialState(true)
				.setByteSize(
					m_numberOfXSlices *
					m_numberOfYSlices *
					m_numberOfZSlices *
					sizeof(ClusterRange))
				.setCanHaveUAVs(true)
				.setStructStride(sizeof(ClusterRange));
			m_pointLightCullData.clusterRangesBuffer = CreateRef<GPUBuffer>(IResource::Usage::FrameStatic, bufferDesc);
		}
#pragma endregion

#pragma region Global Counter Buffer
		{
			nvrhi::BufferDesc bufferDesc;
			bufferDesc
				.setDebugName("Global Counter Buffer")
				.setKeepInitialState(true)
				.setCanHaveUAVs(true)
				.setCanHaveRawViews(true)
				.setByteSize(sizeof(uint32_t));
			m_pointLightCullData.globalCounterBuffer = CreateRef<GPUBuffer>(IResource::Usage::FrameStatic, bufferDesc);
		}
#pragma endregion

#pragma region Binding Set Creation
		{
			std::vector<nvrhi::BindingSetDesc> bindingSetDescs;
			for (uint32_t i = 0; i < Application::Get()->getGraphicsContext()->getMaxFrameInFlight(); i++)
			{
				nvrhi::BindingSetDesc bindingSetDesc;
				bindingSetDesc
					.addItem(nvrhi::BindingSetItem::ConstantBuffer(0, m_pointLightCullData.globalDataBuffer->getStorages()[i].buffer))
					.addItem(nvrhi::BindingSetItem::StructuredBuffer_SRV(0, m_pointLightBuffer->getStorages()[i].buffer))
					.addItem(nvrhi::BindingSetItem::StructuredBuffer_UAV(0, m_pointLightCullData.globalLightIndicesBuffer->getStorages()[i].buffer))
					.addItem(nvrhi::BindingSetItem::StructuredBuffer_UAV(1, m_pointLightCullData.clusterRangesBuffer->getStorages()[i].buffer))
					.addItem(nvrhi::BindingSetItem::RawBuffer_UAV(2, m_pointLightCullData.globalCounterBuffer->getStorages()[i].buffer));
				bindingSetDescs.push_back(bindingSetDesc);
			}
			m_pointLightCullData.lightCullBindingSet = std::make_shared<BindingSet>(
				IResource::Usage::FrameStreaming,
				m_lightCullBindingLayout,
				bindingSetDescs);
		}
#pragma endregion

#pragma region Light Culling Compute pipeline Initialization
		{
			nvrhi::ComputePipelineDesc pipelineDesc;

			nvrhi::ShaderDesc lightCullShaderDesc;
			lightCullShaderDesc
				.setDebugName("LightCullComputeShader")
				.setEntryName("main_cs")
				.setShaderType(nvrhi::ShaderType::Compute);
			File file("assets/PaperEngine/shader/LightCull/lightCull.comp.spv");

			auto shaderBinary = file.readBinaryFully();
			pipelineDesc.CS = Application::GetNVRHIDevice()->createShader(
				lightCullShaderDesc,
				shaderBinary->data,
				shaderBinary->size);

			pipelineDesc.bindingLayouts = {
				m_lightCullBindingLayout
			};

			m_lightCullPipeline = Application::GetNVRHIDevice()->createComputePipeline(pipelineDesc);

		}
#pragma endregion


	}

	void LightCullingPass::setCamera(const Camera& camera, const glm::mat4& viewMatrix, const Frustum& frustum)
	{
		GlobalData* globalData = static_cast<GlobalData*>(m_pointLightCullData.globalDataBuffer->mapBufferPtr());

		globalData->projViewMatrix = camera.getProjectionMatrix() * viewMatrix;
		globalData->viewMatrix = viewMatrix;
		globalData->inverseProjMatrix = glm::inverse(camera.getProjectionMatrix());
		globalData->screenWidth = camera.getWidth();
		globalData->screenHeight = camera.getHeight();
		m_currentCameraFrustum = frustum;
		globalData->nearPlane = camera.getNearPlane();
		globalData->farPlane = camera.getFarPlane();
	}

	void LightCullingPass::beginPass()
	{
		// 重置
		m_currentDirectionalLightCount = 0;
		m_currentPointLightCount = 0;
	}

	LightCullingPass::LightCullingPass()
	{
	}

	LightCullingPass::~LightCullingPass()
	{
	}

	void LightCullingPass::processLight(const Transform& transform, const LightComponent& lightCom)
	{
		switch (lightCom.type)
		{
		case LightType::Directional:
		{
			if (m_currentDirectionalLightCount >= m_maxDirectionalLight)
				break;
			DirectionalLightData* dataPtr =
				reinterpret_cast<DirectionalLightData*>(
					(uint8_t*)m_directionalLightBuffer->mapBufferPtr() + (m_currentDirectionalLightCount * sizeof(DirectionalLightData)));
			dataPtr->direction = lightCom.light.directionalLight.direction;
			dataPtr->color = lightCom.light.directionalLight.color;
			m_currentDirectionalLightCount++;
		}
			break;
		case LightType::Point:
		{
			if (m_currentPointLightCount >= m_maxPointLight)
				break;
			const PointLight& pointLight = lightCom.light.pointLight;
			if (!BoundingSphere(transform.getPosition(), pointLight.radius)
				.isIntersect(m_currentCameraFrustum))		// 不在畫面裡
				break;

			PointLightData* dataPtr =
				reinterpret_cast<PointLightData*>(
					(uint8_t*)m_pointLightBuffer->mapBufferPtr() + (m_currentPointLightCount * sizeof(PointLightData)));
			dataPtr->position = transform.getPosition();
			dataPtr->color = lightCom.light.pointLight.color;
			dataPtr->radius = lightCom.light.pointLight.radius;
			m_currentPointLightCount++;
		}
			break;
		default:
			break;
		}
	}

	void LightCullingPass::calculatePass(nvrhi::ICommandList* cmd)
	{
		PE_PROFILE_FUNCTION();
		// 紀錄一下會compute多少個Point Light
		m_numberOfProcessPointLights = m_currentPointLightCount;

		auto& pointLightCullData = m_pointLightCullData;

		GlobalData* globalData = static_cast<GlobalData*>(pointLightCullData.globalDataBuffer->mapBufferPtr());
		globalData->numXSlices = m_numberOfXSlices;
		globalData->numYSlices = m_numberOfYSlices;
		globalData->numZSlices = m_numberOfZSlices;
		globalData->pointLightCount = m_currentPointLightCount;

		nvrhi::ComputeState computeState;

		// TODO: calculate Depth min max in each tiles

		// Compute Light Clusters
#pragma region Compute Light Clusters
		computeState.bindings = { pointLightCullData.lightCullBindingSet->getCurrentHandle()};
		computeState.pipeline = m_lightCullPipeline;
		cmd->setComputeState(computeState);

		// 重置Buffers
		cmd->clearBufferUInt(pointLightCullData.globalCounterBuffer->getCurrentHandle(), 0);
		cmd->clearBufferUInt(pointLightCullData.globalLightIndicesBuffer->getCurrentHandle(), 0);

		cmd->dispatch(m_numberOfXSlices, m_numberOfYSlices, m_numberOfZSlices);

		//cmd->setBufferState(
		//	pointLightCullData.globalDataBuffer,
		//	nvrhi::ResourceStates::ShaderResource);
		//cmd->setBufferState(
		//	pointLightCullData.globalCounterBuffer,
		//	nvrhi::ResourceStates::ShaderResource);
		//cmd->setBufferState(
		//	pointLightCullData.globalLightIndicesBuffer,
		//	nvrhi::ResourceStates::ShaderResource);
		//cmd->setBufferState(
		//	pointLightCullData.clusterRangesBuffer,
		//	nvrhi::ResourceStates::ShaderResource);
		//cmd->commitBarriers();

#pragma endregion

	}

	LightCullingPass::PointLightCullData& LightCullingPass::getPointLightCullData()
	{
		return m_pointLightCullData;
	}

}
