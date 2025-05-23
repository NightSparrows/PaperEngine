#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <PaperEngine/renderer/SceneRenderer.h>
#include <PaperEngine/renderer/Camera.h>

#include "VulkanDescriptorSetLayout.h"
#include "VulkanTexture.h"
#include "VulkanFramebuffer.h"
#include "VulkanBuffer.h"
#include "VulkanDescriptorSet.h"

namespace PaperEngine {

	class VulkanSceneRenderer : public SceneRenderer {

	public:

		VulkanSceneRenderer(const SceneRendererSpec& spec);
		~VulkanSceneRenderer();

		void resize(uint32_t width, uint32_t height) override;

		void renderScene(const Scene& scene) override;

		void addRenderer(RendererHandle renderer) override;

		void createFramebuffers(uint32_t width, uint32_t height);

		glm::ivec2 getSize() const override;

	protected:

		void renderSceneCamera(const Camera& camera, TextureHandle targetImage);

	public:

		// intiialize in vulkan context
		static void Init();
		static void CleanUp();

		/// <summary>
		/// Get the lightning pass 
		/// </summary>
		/// <returns></returns>
		static VkRenderPass GetLightningPass();

		static VulkanDescriptorSetLayoutHandle GetGlobalDescriptorSetLayout();

	protected:
		struct SceneRenderFrame {

			// lighting pass
			VulkanTextureHandle colorAttachment;
			VulkanTextureHandle depthAttachment;

			VulkanFramebufferHandle lightingFramebuffer;

			Ref<VulkanBuffer> globalUniformBuffer;
			Ref<VulkanDescriptorSet> globalSet;
		};

		// the frame struct in flight
		std::vector<SceneRenderFrame> m_frames;
		std::vector<RendererHandle> m_renderers;

		uint32_t m_width, m_height;

	};

}
