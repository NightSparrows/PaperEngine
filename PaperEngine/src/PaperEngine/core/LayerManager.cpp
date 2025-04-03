
#include <PaperEngine/core/Logger.h>
#include <PaperEngine/core/LayerManager.h>

namespace PaperEngine {

	void LayerManager::cleanUp()
	{
		for (Layer* layer : *m_layers) {
			layer->on_detach();
		}
		m_layers->clear();
	}

	void LayerManager::push_layer(Layer* layer)
	{
		m_layers->emplace(m_layers->begin() + m_layerInsertIndex, layer);
		m_layerInsertIndex++;
	}

	void LayerManager::push_overlay(Layer* layer)
	{
		m_layers->emplace_back(layer);
	}

	void LayerManager::pop_layer(Layer* layer)
	{
		auto it = std::find(m_layers->begin(), m_layers->begin() + m_layerInsertIndex, layer);
		if (it != m_layers->begin() + m_layerInsertIndex) {
			layer->on_detach();
			m_layers->erase(it);
			m_layerInsertIndex--;
		}
	}

	void LayerManager::pop_overlay(Layer* layer)
	{
		auto it = std::find(m_layers->begin() + m_layerInsertIndex, m_layers->end(), layer);
		if (it != m_layers->end())
		{
			layer->on_detach();
			m_layers->erase(it);
		}
	}

}
