#pragma once

#include <vulkan/vulkan.h>
#include <PaperEngine/renderer/CommandBuffer.h>

#include "VulkanStagingBuffer.h"

#include "VulkanDescriptorSet.h"
#include "VulkanFramebuffer.h"
#include "VulkanTexture.h"
#include "VulkanGraphicsPipeline.h"

namespace PaperEngine {

	class VulkanMaterial;

	class VulkanCommandBuffer : public CommandBuffer {
	public:
		VulkanCommandBuffer(const CommandBufferSpec& spec = CommandBufferSpec());
		~VulkanCommandBuffer();

		void open() override;
		void close() override;

		void setViewports(uint32_t viewportCount, const Viewport* viewports, uint32_t firstViewport) override;

		void setViewport(const Viewport& viewport, uint32_t viewportIndex = 0) override;

		void setTextureState(TextureHandle texture, TextureState state) override;

		void writeBuffer(BufferHandle buffer, const void* data, size_t size, size_t offset = 0) override;

		void copyBuffer(BufferHandle srcBuffer, BufferHandle dstBuffer, size_t size, size_t srcOffset, size_t dstOffset) override;

		void writeTexture(TextureHandle texture, const void* data, const ImageOffset& offset, const ImageExtent& extent) override;

		void bindDescriptorSet(uint32_t setSlot, DescriptorSetHandle set, BindPoint bindPoint = Graphics) override;

		void bindDescriptorSets(uint32_t firstSet, uint32_t setCount, DescriptorSetHandle* set, BindPoint bindPoint = Graphics) override;

		void bindGraphicsPipeline(GraphicsPipelineHandle graphicsPipeline) override;

		void bindIndexBuffer(BufferHandle buffer, uint32_t offset) override;

		void drawIndexed(uint32_t indexCount, uint32_t firstIndex = 0, uint32_t instanceCount = 1, uint32_t vertexOffset = 0, uint32_t firstInstance = 0) override;

		void beginFramebuffer(VulkanFramebufferHandle framebuffer);
		void endFramebuffer();

		VkCommandPool get_pool() const { return m_pool; }

		VkCommandBuffer get_handle() const { return m_handle; }

		static VkPipelineBindPoint ConvertBindPoint(BindPoint bindPoint);

	protected:
		friend class VulkanCommandBufferManager;
		/// <summary>
		/// Release the object that is hold by this command buffer 
		/// use in CommandBufferManager that wait after the execution
		/// </summary>
		void releaseObject();

	protected:
		VkCommandPool m_pool{ VK_NULL_HANDLE };			// the pool this command buffer is using
		VkCommandBuffer m_handle{ VK_NULL_HANDLE };

		bool m_isPrimary;

		VulkanStagingBufferHandle m_stagingBuffer;

		// the framebuffer that hold by this command buffer in runtime
		std::vector<VulkanFramebufferHandle> m_framebuffers;
		std::vector<BufferHandle> m_buffers;
		std::vector<TextureHandle> m_textures;
		std::vector<Ref<DescriptorSet>> m_descriptorSets;
		std::vector<Ref<VulkanGraphicsPipeline>> m_graphicsPipeline;
		Ref<VulkanGraphicsPipeline> m_currentGraphicsPipeline;
		VulkanFramebufferHandle m_currentFramebuffer;
	};

	typedef Ref< VulkanCommandBuffer> VulkanCommandBufferHandle;
}
