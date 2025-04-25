
#include <array>
#include <fstream>

#include <PaperEngine/core/Base.h>
#include <PaperEngine/core/Logger.h>

#include "VulkanGraphicsPipeline.h"
#include "VulkanContext.h"
#include "VulkanUtils.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanSceneRenderer.h"
#include "VulkanMeshRenderer.h"
#include "VulkanShader.h"

namespace PaperEngine {

	VulkanGraphicsPipeline::VulkanGraphicsPipeline(const GraphicsPipelineSpecification& spec)
	{
		VkGraphicsPipelineCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        createInfo.flags = 0;

        // TODO: shaders
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        PE_CORE_ASSERT(spec.VS, "Vertex shader not set!");

        Ref<VulkanShader> vertexShader = std::static_pointer_cast<VulkanShader>(spec.VS);
        shaderStages.push_back(VkPipelineShaderStageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertexShader->get_handle(),
            .pName = "main"
            });

        Ref<VulkanShader> fragmentShader = std::static_pointer_cast<VulkanShader>(spec.FS);
        shaderStages.push_back(VkPipelineShaderStageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragmentShader->get_handle(),
            .pName = "main"
            });

        createInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        createInfo.pStages = shaderStages.data();

#pragma region Vertex Input State

        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        // not using vertex input for vertex data, using pvp
        createInfo.pVertexInputState = &vertexInput;

#pragma endregion

#pragma region Input Assembly State

        VkPipelineInputAssemblyStateCreateInfo inputAsm{};
        inputAsm.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAsm.primitiveRestartEnable = spec.primitiveRestartEnable ? VK_TRUE : VK_FALSE;
        switch (spec.topology)
        {
        case GraphicsPipelineSpecification::Topology::TriangleList:
            inputAsm.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        default:
            PE_CORE_ASSERT(false, "Unknown topology");
            break;
        }
        createInfo.pInputAssemblyState = &inputAsm;

#pragma endregion

        // TODO: tessellation state

#pragma region Viewport state
        // I'm using dynamic state
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;
        createInfo.pViewportState = &viewportState;
#pragma endregion

#pragma region Rasterization State
        VkPipelineRasterizationStateCreateInfo rasterizationState{};
        rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationState.depthClampEnable = VK_FALSE;
        rasterizationState.rasterizerDiscardEnable = VK_FALSE;
        rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
        switch (spec.cullMode)
        {
        case GraphicsPipelineSpecification::CullMode::Back:
            rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
            break;
        case GraphicsPipelineSpecification::CullMode::None:
            rasterizationState.cullMode = VK_CULL_MODE_NONE;
            break;
        default:
            PE_CORE_ASSERT(false, "Unknown cull mode");
            break;
        }
        rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationState.depthBiasEnable = VK_FALSE;
        rasterizationState.lineWidth = 1.f;
        createInfo.pRasterizationState = &rasterizationState;
#pragma endregion

#pragma region Multisample State
        VkPipelineMultisampleStateCreateInfo multisampleState{};
        multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;      // TODO: use render pass defition ?
        multisampleState.sampleShadingEnable = VK_FALSE;
        multisampleState.minSampleShading = 1.f;
        multisampleState.pSampleMask = nullptr;
        multisampleState.alphaToCoverageEnable = VK_FALSE;
        multisampleState.alphaToOneEnable = VK_FALSE;
        createInfo.pMultisampleState = &multisampleState;
#pragma endregion
        
#pragma region Depth Stencil State
        VkPipelineDepthStencilStateCreateInfo depthStencilState{};
        depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilState.depthTestEnable = spec.depthTest ? VK_TRUE : VK_FALSE;
        depthStencilState.depthWriteEnable = spec.depthWrite ? VK_TRUE : VK_FALSE;
        depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilState.depthBoundsTestEnable = VK_FALSE;
        depthStencilState.stencilTestEnable = VK_FALSE;
        createInfo.pDepthStencilState = &depthStencilState;
#pragma endregion

#pragma region Color Blend State

        VkPipelineColorBlendAttachmentState blendAttechState = {
            .blendEnable = VK_FALSE,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };

        VkPipelineColorBlendStateCreateInfo colorBlendState{};
        colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendState.logicOpEnable = VK_FALSE;
        colorBlendState.logicOp = VK_LOGIC_OP_COPY;
        colorBlendState.attachmentCount = 1;
        colorBlendState.pAttachments = &blendAttechState;
        createInfo.pColorBlendState = &colorBlendState;

#pragma endregion

#pragma region Dynamic State

        VkDynamicState dynamicStates[2] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(VkDynamicState);
        dynamicState.pDynamicStates = dynamicStates;

        createInfo.pDynamicState = &dynamicState;
#pragma endregion

#pragma region Material variables

        DescriptorSetLayoutSpec setLayoutSpec;
        for (uint32_t i = 0; i < spec.materialVariables.size(); i++) {
            const auto& varInfo = spec.materialVariables[i];

            // assert materialBinding[binding.binding] is null before assign
            auto& materialBindingInfo = m_materialBindings[varInfo.binding];
            materialBindingInfo.type = VulkanDescriptorSetLayout::ConvertType(varInfo.type);
            switch (varInfo.type)
            {
            case DescriptorType::UniformBuffer:
                setLayoutSpec.addUniformBuffer(varInfo.binding, varInfo.stages, varInfo.count);
                break;
            case DescriptorType::CombinedImageSampler:
                setLayoutSpec.addImageSampler(varInfo.binding, varInfo.stages, varInfo.count);
                break;
            default:
                break;
            }
        }

        m_materialLayout = std::static_pointer_cast<VulkanDescriptorSetLayout>(DescriptorSetLayout::Create(setLayoutSpec));
#pragma endregion

#pragma region Pipeline Layout Createion

        VkPipelineLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        std::array<VkDescriptorSetLayout, 3> setLayouts{};
        // TODO: insert global set layout, material set layout, and instance set layout
        setLayouts[0] = VulkanSceneRenderer::GetGlobalDescriptorSetLayout()->get_handle();
        setLayouts[1] = m_materialLayout->get_handle();
        setLayouts[2] = VulkanMeshRenderer::GetMeshInstanceDescriptorSetLayout()->get_handle();
        // the instnace set layout mean the data per mesh instnace
        layoutCreateInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        layoutCreateInfo.pSetLayouts = setLayouts.data();

        CHECK_VK_RESULT(vkCreatePipelineLayout(VulkanContext::GetDevice(), &layoutCreateInfo, nullptr, &m_layout));
        createInfo.layout = m_layout;
#pragma endregion


#pragma region RenderPass


        // Use the scene renderer static main renderpass (Lighting and Transparency pass)
        // I think I use that passes as same pass with order
        createInfo.renderPass = VulkanSceneRenderer::GetLightningPass();
        createInfo.subpass = 0;         // Not using subpass for my engine

#pragma endregion


#pragma region Pipeline Cache

        VkPipelineCache pipelineCache{ VK_NULL_HANDLE };
        if (!spec.cacheFilePath.empty()) {
            VkPipelineCacheCreateInfo cacheInfo{};
            cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

            std::vector<char> cacheData;

            if (std::filesystem::exists(spec.cacheFilePath)) {
                std::ifstream file(spec.cacheFilePath, std::ios::binary);
                if (file.is_open()) {
                    size_t fileSize = static_cast<size_t>(file.tellg());
                    cacheData.resize(fileSize);

                    file.seekg(0);
                    file.read(cacheData.data(), fileSize);

                    cacheInfo.initialDataSize = fileSize;
                    cacheInfo.pInitialData = cacheData.data();
                }
            }
            CHECK_VK_RESULT(vkCreatePipelineCache(VulkanContext::GetDevice(), &cacheInfo, nullptr, &pipelineCache));
            m_cacheFilePath = spec.cacheFilePath;
            m_pipelineCache = pipelineCache;
        }

#pragma endregion



        CHECK_VK_RESULT(vkCreateGraphicsPipelines(VulkanContext::GetDevice(), pipelineCache, 1, &createInfo, nullptr, &m_handle));

        PE_CORE_TRACE("Graphics pipeline created. {}", (void*)m_handle);

	}

    VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
    {
        m_materialLayout.reset();

        if (!m_cacheFilePath.empty()) {
            size_t cacheSize = 0;
            vkGetPipelineCacheData(VulkanContext::GetDevice(), m_pipelineCache, &cacheSize, nullptr);
            std::vector<char> cacheData(cacheSize);
            vkGetPipelineCacheData(VulkanContext::GetDevice(), m_pipelineCache, &cacheSize, cacheData.data());

            std::ofstream file(m_cacheFilePath, std::ios::binary);
            if (file.is_open()) {
                file.write(cacheData.data(), cacheData.size());
                PE_CORE_TRACE("graphics pipeline cache save to {}, size: {}", m_cacheFilePath, cacheData.size());
            }
            else {
                PE_CORE_WARN("Failed to save graphics pipeline cache file: {}", m_cacheFilePath);
            }
            vkDestroyPipelineCache(VulkanContext::GetDevice(), m_pipelineCache, nullptr);
        }

        vkDestroyPipelineLayout(VulkanContext::GetDevice(), m_layout, nullptr);
        vkDestroyPipeline(VulkanContext::GetDevice(), m_handle, nullptr);
        PE_CORE_TRACE("Graphics pipeline destroyed. {}", (void*)m_handle);
    }

}
