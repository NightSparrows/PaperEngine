#pragma once

#include <cstdint>

#include <glm/glm.hpp>
#include <PaperEngine/renderer/Texture.h>

namespace PaperEngine {

	struct FramebufferAttachment {

		typedef enum AttachmentType {
			Color,					// color attachment (RGBA8)
			DepthStencil,			// depth attachment
			Present					// present after this framebuffer
		};
		typedef enum LoadOp {
			Load,					// load the texture before framebuffer start
			Clear					// clear the texture before framebuffer start
		};
		typedef enum StoreOp {
			Store,					// store the texture after framebuffer
			DontCare
		};
		AttachmentType type;
		LoadOp loadOp;
		StoreOp storeOp;
		TextureHandle texture;
		glm::vec3 clearColor;				// if this attachment start from clear
		glm::vec2 clearDepthStencil;		// if this attachment is depth and start from clear
	};

	struct FramebufferSpec {
		uint32_t width, height;
		std::vector<FramebufferAttachment> attachments;
	};

	class Framebuffer {
	public:
		virtual ~Framebuffer() = default;
		Framebuffer(const Framebuffer&) = delete;
		Framebuffer& operator=(const Framebuffer&) = delete;



	protected:
		Framebuffer() = default;
	};

}
