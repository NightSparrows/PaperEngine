#pragma once

#include <PaperEngine/components/LightComponent.h>
#include <PaperEngine/utils/Transform.h>
#include <PaperEngine/utils/BoundingVolume.h>
#include <PaperEngine/graphics/Camera.h>

#include <nvrhi/nvrhi.h>

namespace PaperEngine {

	struct DirectionalLightData
	{
		glm::vec3 direction;
		glm::vec3 color;
		// 2025/08/30我發現不用padding
		// 你如果在shader寫 float3才會被pad
		//float _pad0, _pad1;// pad to 32 bytes
	};

	struct PointLightData
	{
		glm::vec3 position;
		glm::vec3 color;
		float radius;
	};

	class LightCullingPass
	{
	public:
		struct PointLightCullData
		{
			nvrhi::BufferHandle globalDataBuffer;
			void* globalDataBufferPtr = nullptr;
			nvrhi::BufferHandle globalLightIndicesBuffer;
			nvrhi::BufferHandle clusterRangesBuffer;
			nvrhi::BufferHandle globalCounterBuffer;
			nvrhi::BindingSetHandle lightCullBindingSet;
		};

		struct GlobalData
		{
			glm::mat4 projViewMatrix;
			glm::mat4 viewMatrix;
			glm::mat4 inverseProjMatrix;

			uint32_t numXSlices;
			uint32_t numYSlices;
			uint32_t numZSlices;

			uint32_t pointLightCount;

			float nearPlane;
			float farPlane;

			uint32_t screenWidth;
			uint32_t screenHeight;
		};

		struct ClusterRange
		{
			uint32_t offset;
			uint32_t count;
		};

	public:
		LightCullingPass();
		~LightCullingPass();

		void init();

		void setCamera(const Camera& camera, const glm::mat4& viewMatrix, const Frustum& frustum);

		/// <summary>
		/// 需要先Call這個才能process
		/// </summary>
		void beginPass();

		/// <summary>
		/// 必須要先設定frustum
		/// </summary>
		/// <param name="transform"></param>
		/// <param name="lightCom"></param>
		void processLight(const Transform& transform, const LightComponent& lightCom);

		void calculatePass(nvrhi::ICommandList* cmd);

		nvrhi::IBuffer* getDirectionalLightBuffer() { return m_directionalLightBuffer; }
		uint32_t getDirectionalLightCount() const { return m_currentDirectionalLightCount; }

		nvrhi::IBuffer* getPointLightBuffer() { return m_pointLightBuffer; }
		uint32_t getPointLightCount() const { return m_currentPointLightCount; }

		PointLightCullData& getPointLightCullData();

		uint32_t getNumberOfProcessPointLights() const { return m_numberOfProcessPointLights; }

		uint32_t getNumberOfXSlices() const { return m_numberOfXSlices; }
		uint32_t getNumberOfYSlices() const { return m_numberOfYSlices; }
		uint32_t getNumberOfZSlices() const { return m_numberOfZSlices; }

	private:
		Frustum m_currentCameraFrustum{};

		//light culling compute shader
		nvrhi::ComputePipelineHandle m_lightCullPipeline;
		nvrhi::BindingLayoutHandle m_lightCullBindingLayout;

		uint32_t m_numberOfXSlices = 32;
		uint32_t m_numberOfYSlices = 32;
		uint32_t m_numberOfZSlices = 32;
		uint32_t m_maxPointLightPerCluster = 2048;

		uint32_t m_numberOfProcessPointLights = 0;
		// data
		PointLightCullData m_pointLightCullData;

		// Directional Light Data
		uint32_t m_maxDirectionalLight = 8;
		// Current to input (in bytes)
		uint32_t m_currentDirectionalLightCount = 0;
		nvrhi::BufferHandle m_directionalLightBuffer;
		void* m_directionalLightBufferPtr = nullptr;

		// Point Light Data
		uint32_t m_maxPointLight = 10000;
		uint32_t m_currentPointLightCount = 0;
		nvrhi::BufferHandle m_pointLightBuffer;
		void* m_pointLightBufferptr = nullptr;

		std::mutex m_mutex;
	};

}
