#include "GraphicsPipeline.h"

#include <PaperEngine/core/Application.h>

namespace PaperEngine {
    GraphicsPipeline::GraphicsPipeline(nvrhi::GraphicsPipelineDesc desc, nvrhi::BindingLayoutHandle bindingLayout, size_t variableBufferSize) :
		m_graphicsPipelineDesc(desc), m_bindingLayout(bindingLayout), m_variableBufferSize(variableBufferSize)
	{

	}

    nvrhi::IGraphicsPipeline* GraphicsPipeline::getGraphicsPipeline(nvrhi::IFramebuffer* fb) const
    {
        if (m_graphicsPipeline)
            return m_graphicsPipeline;

        auto device = Application::Get()->getGraphicsContext()->getNVRhiDevice();

		m_graphicsPipeline = device->createGraphicsPipeline(
                m_graphicsPipelineDesc,
                fb);
		PE_CORE_ASSERT(m_graphicsPipeline, "Failed to create Graphics Pipeline.");

		return m_graphicsPipeline;
    }

    nvrhi::IBindingLayout* GraphicsPipeline::getBindingLayout() const
    {
        return m_bindingLayout;
    }

}
