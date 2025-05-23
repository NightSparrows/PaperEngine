#include "VulkanTexture.h"

#include "VulkanContext.h"
#include "VulkanUtils.h"

namespace PaperEngine {

	static VkImageType convertImageType(TextureType type) {
		switch (type)
		{
		case PaperEngine::Texture2D:
			return VK_IMAGE_TYPE_2D;
		default:
			PE_CORE_ASSERT(false, "Unsupported image type");
			return VK_IMAGE_TYPE_2D;
		}
	}

	static VkImageViewType convertImageViewType(VkImageType type) {
		switch (type)
		{
		case VK_IMAGE_TYPE_2D:
			return VK_IMAGE_VIEW_TYPE_2D;
		default:
			PE_CORE_ASSERT(false, "Unsupported image type");
			return VK_IMAGE_VIEW_TYPE_2D;
		}
	}

	static VkFormat convertFormat(TextureFormat format) {
		switch (format)
		{
		case TextureFormat::RGBA8:
			return VK_FORMAT_R8G8B8A8_SRGB;
		case TextureFormat::RGBA32F:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case TextureFormat::Depth:
			return VK_FORMAT_D32_SFLOAT;	// TODO: get from graphics context
		case TextureFormat::Present:
			return VulkanContext::GetSwapchain().image_format;
		default:
			PE_CORE_ASSERT(false, "Unsupported image format");
			return VK_FORMAT_UNDEFINED;
		}
	}

	VulkanTexture::VulkanTexture(const TextureSpecification& spec) :
		m_width(spec.width), m_height(spec.height), m_format(convertFormat(spec.format)),
		m_type(convertImageType(spec.type))
	{
		VkImageCreateInfo image_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = m_type,
			.format = m_format,
			.extent = { spec.width, spec.height, 1 },
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_SAMPLED_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};
		if (image_info.format != VulkanContext::GetDepthFormat()) {
			image_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			image_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		else {
			image_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		if (spec.canBeTransferSrc) {
			image_info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}
		VmaAllocationCreateInfo alloc_info = {
			.usage = VMA_MEMORY_USAGE_GPU_ONLY
		};
		CHECK_VK_RESULT(vmaCreateImage(VulkanContext::GetAllocator(), &image_info, &alloc_info, &m_image, &m_allocation, nullptr));
		PE_CORE_TRACE("image created. {}", (void*)m_image);

		// TODO create default image view
		this->create_image_view();
		this->create_sampler();
	}

	VulkanTexture::~VulkanTexture()
	{
		vkDestroyImageView(VulkanContext::GetDevice(), m_view, nullptr);
		PE_CORE_TRACE("image view destroyed. {}", (void*)m_view);
		if (!m_isNative) {
			vmaDestroyImage(VulkanContext::GetAllocator(), m_image, m_allocation);
			PE_CORE_TRACE("image destroyed. {}", (void*)m_image);
		}
		if (m_sampler != VK_NULL_HANDLE) {
			vkDestroySampler(VulkanContext::GetDevice(), m_sampler, nullptr);
		}
	}

	VulkanTexture::VulkanTexture(VkImage image, const TextureSpecification& spec) :
		m_width(spec.width), m_height(spec.height), m_format(convertFormat(spec.format)),
		m_type(convertImageType(spec.type)), m_isNative(true), m_image(image)
	{

		this->create_image_view();
		this->create_sampler();
	}

	void VulkanTexture::create_image_view()
	{
		VkImageViewCreateInfo viewCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = m_image, // <-- replace this with your VkImage handle
			.viewType = convertImageViewType(m_type),
			.format = m_format, // <-- match your image format
			.components = {
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY
			},
			.subresourceRange = {
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};
		if (m_format == VulkanContext::GetDepthFormat()) {
			viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		else {
			viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		CHECK_VK_RESULT(vkCreateImageView(VulkanContext::GetDevice(), &viewCreateInfo, nullptr, &m_view));
		PE_CORE_TRACE("image view created. {}", (void*)m_view);
	}

	void VulkanTexture::create_sampler()
	{
		VkSamplerCreateInfo samplerCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,  // Not used since no mips
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.mipLodBias = 0.0f,
			.anisotropyEnable = VK_FALSE,
			.maxAnisotropy = 1.0f,
			.compareEnable = VK_FALSE,
			.compareOp = VK_COMPARE_OP_ALWAYS,
			.minLod = 0.0f,
			.maxLod = 0.0f,
			.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
			.unnormalizedCoordinates = VK_FALSE,
		};
		CHECK_VK_RESULT(vkCreateSampler(VulkanContext::GetDevice(), &samplerCreateInfo, nullptr, &m_sampler));
	}

}
