#pragma once

#include <PaperEngine/core/Base.h>

namespace PaperEngine {

	enum class ImageFormat {
		Present,			// get the swapchain image for framebuffer
		SwapchainFormat,
		DepthBuffer,		// this will use for depth buffer
		DepthFormat,
		RGBA32				// 16 bytes per pixel
	};

	enum class LoadOp {
		DontCare,
		Load,
		Clear
	};

	enum class StoreOp {
		DontCare,
		Store
	};

	enum class ImageLayout {
		Undefined,
		ColorAttachment,
		PresentSrc,
		DepthAttachment,
		ShaderReadOnly
	};

	/// <summary>
	/// a pass of a stage to render
	/// use the primary command buffer to render
	/// also contain framebuffer
	/// and we do use the subpass feature (it just not necessary)
	/// </summary>
	class PE_API RenderPass {
	public:
		
		struct AttachmentDescription {
			ImageFormat format;
			LoadOp loadOp; 
			StoreOp storeOp; 
			ImageLayout initLayout;
			ImageLayout finalLayout;
		};

		struct RenderPassCreateInfo {
			std::vector<AttachmentDescription> attachments;
			std::vector<uint32_t> input_attachments;			// which attachments is use for input
			std::vector<uint32_t> output_attachments;			// which attachments is use for output
		};

		/// <summary>
		/// Recreating the framebuffer with new size
		/// </summary>
		/// <param name="width"></param>
		/// <param name="height"></param>
		virtual void recreate_framebuffers(uint32_t width, uint32_t height) = 0;

		static Ref<RenderPass> Create(const RenderPassCreateInfo& createInfo, uint32_t width, uint32_t height);

	protected:
		RenderPass() = default;

	private:

	};

}
