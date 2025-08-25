#include "Material.h"

#include <PaperEngine/core/Application.h>

namespace PaperEngine {
	
	Material::Material(Ref<GraphicsPipeline> graphicsPipeline) :
		m_graphicsPipeline(graphicsPipeline)
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

	PE_API void Material::setOffset(const std::string& name, uint32_t offset)
	{
		auto& variable = m_parameterSlots[name];

		variable.offset = offset;
	}

	PE_API void Material::setSlot(const std::string& name, uint32_t slotIndex)
	{
		auto& variable = m_parameterSlots[name];

		variable.slot = slotIndex;
	}

	void Material::setFloat(const std::string& name, float value)
	{
		//if (m_parameterSlots.find(name) == m_parameterSlots.end())
		//{

		//	PE_CORE_ERROR("Material: Float parameter '{}' not found.", name);
		//	return;
		//}

		auto& variable = m_parameterSlots[name]; 
		*reinterpret_cast<float*>(m_cpuVariableBuffer.data() + variable.offset) = value;
		m_variableBufferModified = true;
	}

	void Material::setTexture(const std::string& name, TextureHandle texture)
	{
		//if (m_parameterSlots.find(name) == m_parameterSlots.end())
		//{
		//	PE_CORE_ERROR("Material: Texture parameter '{}' not found.", name);
		//	return;
		//}

		auto& variable = m_parameterSlots[name];
		
		for (auto& bindingItem : m_bindingSetDesc.bindings)
		{
			if (bindingItem.type != nvrhi::ResourceType::Texture_SRV)
				continue;

			if (bindingItem.slot != variable.slot)
				continue;

			// already bind it
			if (std::get<TextureHandle>(variable.value) == texture)
				return;

			bindingItem = nvrhi::BindingSetItem::Texture_SRV(variable.slot, texture->getTexture());
			variable.value = texture;
			variable.type = VariableType::Texture;
			m_bindingSet = nullptr;
			return;
		}

		// 沒找到，新增他
		m_bindingSetDesc.addItem(nvrhi::BindingSetItem::Texture_SRV(variable.slot, texture->getTexture()));
		variable.value = texture;
		m_bindingSet = nullptr;
	}

	void Material::setSampler(const std::string& name, nvrhi::SamplerHandle sampler)
	{

		auto& variable = m_parameterSlots[name];

		for (auto& bindingItem : m_bindingSetDesc.bindings)
		{
			if (bindingItem.type != nvrhi::ResourceType::Sampler)
				continue;

			if (bindingItem.slot != variable.slot)
				continue;

			// 同type同slot

			// already bind it
			if (std::get<nvrhi::SamplerHandle>(variable.value) == sampler)
				return;

			bindingItem = nvrhi::BindingSetItem::Sampler(variable.slot, sampler);
			variable.value = sampler;
			variable.type = VariableType::Sampler;
			m_bindingSet = nullptr;
			return;
		}
		// 沒找到，新增他
		m_bindingSetDesc.addItem(nvrhi::BindingSetItem::Sampler(variable.slot, sampler));
		variable.value = sampler;
		variable.type = VariableType::Sampler;
		m_bindingSet = nullptr;
	}

	nvrhi::IBindingSet* Material::getBindingSet()
	{
		if (m_variableBufferModified && m_cpuVariableBuffer.size() > 0) {
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
