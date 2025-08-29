#pragma once

#include <PaperEngine/components/LightComponent.h>
#include <PaperEngine/utils/Transform.h>

#include <nvrhi/nvrhi.h>

namespace PaperEngine {

	struct DirectionalLightData
	{
		glm::vec3 direction;
		glm::vec3 color;
		float _pad0, _pad1;// pad to 32 bytes
	};

	struct PointLightData
	{
		glm::vec3 position;
		glm::vec3 color;
		float radius;
		float _pad0;// pad to 32 bytes
	};

	class LightCullingPass
	{
	public:
		LightCullingPass();
		~LightCullingPass();

		void init();

		void processLight(const Transform& transform, const LightComponent& lightCom);

		void calculatePass();

		nvrhi::IBuffer* getDirectionalLightBuffer() { return m_directionalLightBuffer; }

		uint32_t getDirectionalLightCount() const { return m_currentDirectionalLightCount; }

	private:


		// Directional Light Data
		std::vector<DirectionalLightData> m_directionalLightCpuBuffer;

		uint32_t m_maxDirectionalLight = 8;
		// Current to input (in bytes)
		uint32_t m_currentDirectionalLightCount = 0;
		nvrhi::BufferHandle m_directionalLightBuffer;
		void* m_directionalLightBufferPtr = nullptr;


		nvrhi::BufferHandle m_pointLightBuffer;

		nvrhi::CommandListHandle m_cmd;

	};

}
