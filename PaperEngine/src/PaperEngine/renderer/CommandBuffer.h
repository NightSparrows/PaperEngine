#pragma once


#include <PaperEngine/core/Base.h>
#include <PaperEngine/renderer/Buffer.h>
#include <PaperEngine/renderer/GraphicsPipeline.h>
#include <PaperEngine/renderer/DescriptorSet.h>

namespace PaperEngine {

	typedef enum BindPoint {
		Graphics,
		Compute
	}BindPoint;

	struct CommandBufferSpec {
		// is primary command buffer
		bool isPrimary = true;
	};

	class CommandBuffer {
	public:
		virtual ~CommandBuffer() = default;

		/// <summary>
		/// Open that this buffer can start recording commands
		/// when open it
		/// you CAN NOT recording cross thread.
		/// </summary>
		virtual void open() = 0;
		virtual void close() = 0;

		/// <summary>
		/// Don't care about buffer staging
		/// </summary>
		/// <param name="buffer"></param>
		/// <param name="data"></param>
		/// <param name="size"></param>
		/// <param name="offset"></param>
		virtual void writeBuffer(BufferHandle buffer, const void* data, size_t size, size_t offset = 0) = 0;

		/// <summary>
		/// Future support subresource
		/// </summary>
		/// <param name="texture"></param>
		/// <param name="state"></param>
		virtual void setTextureState(TextureHandle texture, TextureState state) = 0;

		virtual void bindGraphicsPipeline(GraphicsPipelineHandle graphicsPipeline) = 0;

		virtual void bindIndexBuffer(BufferHandle buffer, uint32_t offset = 0) = 0;

		// bindDescriptorSet
		virtual void bindDescriptorSet(uint32_t setSlot, DescriptorSetHandle set, BindPoint bindPoint = Graphics) = 0;

		virtual void bindDescriptorSets(uint32_t firstSet, uint32_t setCount, DescriptorSetHandle* set, BindPoint bindPoint = Graphics) = 0;

		/// <summary>
		/// Draw vertices depend on index buffer
		/// </summary>
		/// <param name="indexCount"></param>
		/// <param name="firstIndex"></param>
		/// <param name="instanceCount"></param>
		/// <param name="vertexOffset"></param>
		/// <param name="firstInstance"></param>
		virtual void drawIndexed(uint32_t indexCount, uint32_t firstIndex = 0, uint32_t instanceCount = 1, uint32_t vertexOffset = 0, uint32_t firstInstance = 0) = 0;

	public:
		PE_API static Ref<CommandBuffer> Create(const CommandBufferSpec& spec);

	protected:
		CommandBuffer() = default;

	};

	typedef Ref<CommandBuffer> CommandBufferHandle;
}
