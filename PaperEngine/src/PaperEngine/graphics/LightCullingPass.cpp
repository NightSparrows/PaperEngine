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


#pragma region Initialize Point buffers
		{
			nvrhi::BufferDesc bufferDesc;
			bufferDesc
				.setByteSize(m_maxPointLight * sizeof(PointLightData))
				.setCpuAccess(nvrhi::CpuAccessMode::Write)
				.setIsConstantBuffer(true)
				.setKeepInitialState(true)
				.setStructStride(sizeof(PointLightData));
			m_pointLightBuffer = Application::GetNVRHIDevice()->createBuffer(bufferDesc);
			m_pointLightBufferptr = Application::GetNVRHIDevice()->mapBuffer(m_pointLightBuffer, nvrhi::CpuAccessMode::Write);

		}
#pragma endregion

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
		Application::GetNVRHIDevice()->unmapBuffer(m_pointLightBuffer);
		m_pointLightBuffer = nullptr;
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
		case LightType::Point:
		{
			const PointLight& pointLight = lightCom.light.pointLight;
			if (!BoundingSphere(transform.getPosition(), pointLight.radius)
				.isIntersect(m_currentCameraFrustum))		// 不在畫面裡
				break;

			PointLightData* dataPtr =
				reinterpret_cast<PointLightData*>(
					(uint8_t*)m_pointLightBufferptr + (m_currentPointLightCount * sizeof(PointLightData)));
			dataPtr->position = transform.getPosition();
			dataPtr->color = lightCom.light.pointLight.color;
			dataPtr->radius = lightCom.light.pointLight.radius;
		}
			break;
		default:
			break;
		}
	}

	void LightCullingPass::calculatePass()
	{
		// TODO comput light tiles

		// 重置
		m_currentDirectionalLightCount = 0;
		m_currentPointLightCount = 0;
	}

}
