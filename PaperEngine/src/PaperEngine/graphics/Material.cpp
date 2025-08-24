#include "Material.h"

#include <PaperEngine/core/Application.h>

namespace PaperEngine {
	
	Material::Material(Ref<GraphicsPipeline> graphicsPipeline, const std::unordered_map<std::string, Variable>& parameterSlots) :
		m_graphicsPipeline(graphicsPipeline), m_parameterSlots(parameterSlots)
	{
		m_cmd = Application::GetNVRHIDevice()->createCommandList();

		if (m_graphicsPipeline->getVariableBufferSize() > 0) {
			nvrhi::BufferDesc bufferDesc;
			bufferDesc
				.setByteSize(m_graphicsPipeline->getVariableBufferSize())
				.setIsConstantBuffer(true);
			m_variableBuffer = Application::GetNVRHIDevice()->createBuffer(bufferDesc);
			m_bindingSetDesc.addItem(nvrhi::BindingSetItem::ConstantBuffer(0, m_variableBuffer));
			m_cpuVariableBuffer.resize(m_graphicsPipeline->getVariableBufferSize());
		}
	}

	void Material::setFloat(const std::string& name, float value)
	{
		if (m_parameterSlots.find(name) == m_parameterSlots.end())
		{
			PE_CORE_ERROR("Material: Float parameter '{}' not found.", name);
			return;
		}

		auto& variable = m_parameterSlots[name]; 
		*reinterpret_cast<float*>(m_cpuVariableBuffer.data() + variable.offset) = value;
		m_variableBufferModified = true;
	}

	void Material::setTexture(const std::string& name, nvrhi::TextureHandle texture)
	{
		if (m_parameterSlots.find(name) == m_parameterSlots.end())
		{
			PE_CORE_ERROR("Material: Texture parameter '{}' not found.", name);
			return;
		}

		auto& variable = m_parameterSlots[name];
		
		for (auto& bindingItem : m_bindingSetDesc.bindings)
		{
			if (bindingItem.type != nvrhi::ResourceType::Texture_SRV)
				continue;

			if (bindingItem.slot != variable.slot)
				continue;

			// 同type同slot

			if (variable.type != VariableType::Texture)
			{
				PE_CORE_ERROR("Material: Parameter '{}' is not a texture type.", name);
				return;
			}

			// already bind it
			if (std::get<nvrhi::TextureHandle>(variable.value) == texture)
				return;

			bindingItem = nvrhi::BindingSetItem::Texture_SRV(variable.slot, texture);
			variable.value = texture;
			m_bindingSet = nullptr;
			return;
		}

		// 沒找到，新增他
		m_bindingSetDesc.addItem(nvrhi::BindingSetItem::Texture_SRV(variable.slot, texture));
		variable.value = texture;
		m_bindingSet = nullptr;
	}

	nvrhi::IBindingSet* Material::getBindingSet()
	{
		if (m_variableBufferModified) {
			m_cmd->open();
			m_cmd->writeBuffer(m_variableBuffer, m_cpuVariableBuffer.data(), m_cpuVariableBuffer.size());
			m_cmd->close();
			Application::GetNVRHIDevice()->executeCommandList(m_cmd);
			m_variableBufferModified = false;
		}
		if (m_bindingSet)
			return m_bindingSet;

		auto device = Application::Get()->getGraphicsContext()->getNVRhiDevice();
		m_bindingSet = device->createBindingSet(
			m_bindingSetDesc,
			m_graphicsPipeline->getBindingLayout());

		return m_bindingSet;
	}

} // namespace PaperEngine
