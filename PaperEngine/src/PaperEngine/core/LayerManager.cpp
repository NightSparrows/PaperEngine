#include "LayerManager.h"

namespace PaperEngine {

	void LayerManager::cleanUp()
	{
		for (Layer* layer : m_layers) {
			layer->onDetach();
		}
		m_layers.clear();
	}

	void LayerManager::pushLayer(Layer* layer)
	{
		m_layers.emplace(m_layers.begin() + m_layerInsertIndex, layer);
		m_layerInsertIndex++;
		layer->onAttach();
	}

	void LayerManager::pushOverlay(Layer* layer)
	{
		m_layers.emplace_back(layer);
		layer->onAttach();
	}

	void LayerManager::popLayer(Layer* layer)
	{
		auto it = std::find(m_layers.begin(), m_layers.begin() + m_layerInsertIndex, layer);
		if (it != m_layers.begin() + m_layerInsertIndex) {
			layer->onDetach();
			m_layers.erase(it);
			m_layerInsertIndex--;
		}
	}

	void LayerManager::popOverlay(Layer* layer)
	{
		auto it = std::find(m_layers.begin() + m_layerInsertIndex, m_layers.end(), layer);
		if (it != m_layers.end())
		{
			layer->onDetach();
			m_layers.erase(it);
		}
	}

}
