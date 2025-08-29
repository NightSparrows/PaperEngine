#include "LightCullingPass.h"

#include <PaperEngine/core/Application.h>

namespace PaperEngine {

	void LightCullingPass::init() {

		nvrhi::BufferDesc bufferDesc;
		bufferDesc
			.setByteSize(m_maxDirectionalLight * sizeof(DirectionalLightData))
			.setCpuAccess(nvrhi::CpuAccessMode::Write)
			.setIsConstantBuffer(true)
			.setKeepInitialState(true)
			.setStructStride(sizeof(DirectionalLightData));
		m_directionalLightBuffer = Application::GetNVRHIDevice()->createBuffer(bufferDesc);
		m_directionalLightBufferPtr = Application::GetNVRHIDevice()->mapBuffer(m_directionalLightBuffer, nvrhi::CpuAccessMode::Write);
	}

	void LightCullingPass::setCameraFrustum(const Frustum& frustum)
	{
		m_currentCameraFrustum = frustum;
	}

	LightCullingPass::LightCullingPass()
	{
	}

	LightCullingPass::~LightCullingPass()
	{
		Application::GetNVRHIDevice()->unmapBuffer(m_directionalLightBuffer);
		m_directionalLightBufferPtr = nullptr;
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
					(uint8_t*)m_directionalLightBufferPtr + (m_currentDirectionalLightCount * sizeof(DirectionalLightData)));
			dataPtr->direction = lightCom.light.directionalLight.direction;
			dataPtr->color = lightCom.light.directionalLight.color;
			m_currentDirectionalLightCount++;
		}
			break;
		default:
			break;
		}
	}

	void LightCullingPass::calculatePass()
	{
		// 重置
		m_currentDirectionalLightCount = 0;
	}

}
