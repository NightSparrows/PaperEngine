
#include <array>

#include <PaperEngine/scene/Scene.h>

#include <PaperEngine/component/CameraComponent.h>

#include "VulkanSceneRenderer.h"
#include "VulkanContext.h"
#include "VulkanUtils.h"

namespace PaperEngine {


	static VkRenderPass s_lightningPass = VK_NULL_HANDLE;
	static VulkanDescriptorSetLayoutHandle s_globalDescriptorSetLayout;

	VulkanSceneRenderer::VulkanSceneRenderer(const SceneRendererSpec& spec)
	{
		m_width = spec.width;
		m_height = spec.height;
		createFramebuffers(spec.width, spec.height);
	}

	VulkanSceneRenderer::~VulkanSceneRenderer()
	{
	}

	void VulkanSceneRenderer::resize(uint32_t width, uint32_t height)
	{
		m_width = width;
		m_height = height;
		createFramebuffers(width, height);
	}

	void VulkanSceneRenderer::renderScene(const Scene& scene)
	{
		// prepare scene
		for (auto renderer : m_renderers) {
			renderer->prepareScene(scene);
		}

		// get cameras
		std::vector<const CameraComponent*> components;
		for (auto [entity, cameraCom] : scene.get_registry().view<CameraComponent>().each()) {
			if (!cameraCom.isAcive)
				continue;

			components.push_back(&cameraCom);
		}
		std::sort(components.begin(), components.end(), [](const CameraComponent* a, const CameraComponent* b) {
			return a->cameraOrder < b->cameraOrder;
			});

		// render each camera to its target (the primary camera to last and render to final image
		for (const auto com : components) {
			renderSceneCamera(com->camera, com->target->get_texture());
		}
	}

	void VulkanSceneRenderer::addRenderer(RendererHandle renderer)
	{
		m_renderers.push_back(renderer);
	}

	void VulkanSceneRenderer::createFramebuffers(uint32_t width, uint32_t height)
	{
		m_frames.reserve(VulkanContext::GetImageCount());
		m_frames.clear();

		for (uint32_t i = 0; i < VulkanContext::GetImageCount(); i++) {
			auto& frameInfo = m_frames.emplace_back();

			// lighting pass
#pragma region Lighting pass
			{
				TextureSpecification colorSpec = {
					.width = width,
					.height = height,
					.format = TextureFormat::RGBA8,
					.isRenderTarget = true,
					.canBeTransferSrc = true
				};
				frameInfo.colorAttachment = CreateRef<VulkanTexture>(colorSpec);

				TextureSpecification depthSpec = {
					.width = width,
					.height = height,
					.format = TextureFormat::Depth,
					.isRenderTarget = true,
				};
				frameInfo.depthAttachment = CreateRef<VulkanTexture>(depthSpec);

				FramebufferSpec lightingFramebufferSpec = {
					.width = width,
					.height = height,
					.renderPass = s_lightningPass
				};

				auto& colorAttachment = lightingFramebufferSpec.attachments.emplace_back();
				colorAttachment.texture = frameInfo.colorAttachment;
				colorAttachment.clearColor = { 0.f, 0.f, 0.f, 1.f };
				auto& depthAttachment = lightingFramebufferSpec.attachments.emplace_back();
				depthAttachment.texture = frameInfo.depthAttachment;
				depthAttachment.clearDepth = 1.f;

				frameInfo.lightingFramebuffer = CreateRef<VulkanFramebuffer>(lightingFramebufferSpec);

#pragma region Global Descriptor Set Creation
				{
					BufferSpecification globalUniformBufferSpec;
					globalUniformBufferSpec
						.setSize(sizeof(GlobalUniformBufferStruct))
						.setIsUniformBuffer(true);

					frameInfo.globalUniformBuffer = CreateRef<VulkanBuffer>(globalUniformBufferSpec);

					frameInfo.globalSet = VulkanContext::GetDescriptorSetManager()->allocate(s_globalDescriptorSetLayout);

					// update global set
					VkDescriptorBufferInfo globalBufferInfo = {
						.buffer = frameInfo.globalUniformBuffer->get_handle(),
						.offset = 0,
						.range = frameInfo.globalUniformBuffer->get_size()
					};
					VkWriteDescriptorSet write = {
						.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
						.dstSet = frameInfo.globalSet->get_handle(),
						.dstBinding = 0,
						.descriptorCount = 1,
						.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
						.pBufferInfo = &globalBufferInfo
					};
					vkUpdateDescriptorSets(VulkanContext::GetDevice(), 1, &write, 0, nullptr);
				}
#pragma endregion

			}
#pragma endregion

		}

	}

	glm::ivec2 VulkanSceneRenderer::getSize() const
	{
		return { m_width, m_height };
	}

	void VulkanSceneRenderer::renderSceneCamera(const Camera& camera, TextureHandle targetImage)
	{
		auto& currentFrameInfo = m_frames[VulkanContext::GetCurrentImageIndex()];

		VulkanCommandBufferHandle cmd = CreateRef<VulkanCommandBuffer>();

		// 
		GlobalUniformBufferStruct globalStruct{};
		globalStruct.projectionMatrix = camera.get_projection_matrix();
		globalStruct.viewMatrix = camera.get_view_matrix();
		cmd->open();
		cmd->writeBuffer(currentFrameInfo.globalUniformBuffer, &globalStruct, sizeof(GlobalUniformBufferStruct));
		cmd->close();
		VulkanContext::GetCommandBufferManager()->executeCommandBuffer(cmd);

		cmd->open();

		// TODO: pre depth rendering
		// TODO: shadowmap rendering

		cmd->beginFramebuffer(currentFrameInfo.lightingFramebuffer);

		// you must bind the global set after graphics pipeline is bind
		//cmd->bindDescriptorSet(0, currentFrameInfo.globalSet);

		for (auto renderer : m_renderers) {
			renderer->render(cmd, currentFrameInfo.globalSet, m_width, m_height);
		}

		cmd->endFramebuffer();

		// last process image, used as the copy source
		VulkanTextureHandle lastImage = currentFrameInfo.colorAttachment;

		// TODO: post processing passes

		// TODO:
		// copy lastImage to camera target
#pragma region Copy lastImage to Camera Target
		cmd->setTextureState(lastImage, TextureState::TransferSrc);
		cmd->setTextureState(targetImage, TextureState::TransferDst);

		cmd->copyTexture(lastImage, targetImage, { 0, 0 }, { 0, 0 }, { lastImage->get_width(), lastImage->get_height(), 1 });

#pragma endregion


		cmd->close();
		VulkanContext::GetCommandBufferManager()->executeCommandBuffer(cmd);
	}

	void PaperEngine::VulkanSceneRenderer::Init()
	{
		{
			std::array<VkAttachmentDescription, 2> attachments = {
				VkAttachmentDescription{
					.format = VK_FORMAT_R8G8B8A8_SRGB,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				},
				VkAttachmentDescription{
					.format = VulkanContext::GetDepthFormat(),
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				}
			};

			VkAttachmentReference colorAttachment = {
				.attachment = 0,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			};
			VkAttachmentReference depthAttachment = {
				.attachment = 1,
				.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			};
			VkSubpassDescription subpass = {
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = 1,
				.pColorAttachments = &colorAttachment,
				.pDepthStencilAttachment = &depthAttachment
			};

			VkRenderPassCreateInfo createInfo = {
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
				.attachmentCount = static_cast<uint32_t>(attachments.size()),
				.pAttachments = attachments.data(),
				.subpassCount = 1,
				.pSubpasses = &subpass
			};

			CHECK_VK_RESULT(vkCreateRenderPass(VulkanContext::GetDevice(), &createInfo, nullptr, &s_lightningPass));
		}


#pragma region Global Descriptor Set Layout Creation
		DescriptorSetLayoutSpec globalDscLayoutSpec;
		globalDscLayoutSpec
			.addUniformBuffer(0, ShaderStage::Vertex);

		s_globalDescriptorSetLayout = std::static_pointer_cast<VulkanDescriptorSetLayout>(DescriptorSetLayout::Create(globalDscLayoutSpec));

#pragma endregion


		PE_CORE_TRACE("[SceneRenderer] initialized.");
	}

	void PaperEngine::VulkanSceneRenderer::CleanUp()
	{
		s_globalDescriptorSetLayout.reset();

		vkDestroyRenderPass(VulkanContext::GetDevice(), s_lightningPass, nullptr);
		s_lightningPass = VK_NULL_HANDLE;
		PE_CORE_TRACE("[SceneRenderer] Clean up.");
	}

	VkRenderPass VulkanSceneRenderer::GetLightningPass()
	{
		return s_lightningPass;
	}

	VulkanDescriptorSetLayoutHandle VulkanSceneRenderer::GetGlobalDescriptorSetLayout()
	{
		return s_globalDescriptorSetLayout;
	}

}
