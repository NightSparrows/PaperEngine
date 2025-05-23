﻿#pragma once

#include <vector>

#include <PaperEngine/core/Layer.h>

namespace PaperEngine {


	/// <summary>
	/// Not used by user, used by the engine
	/// </summary>
	class PE_API LayerManager {
	public:

		LayerManager() {
			m_layers = new std::vector<Layer*>();
		}
		~LayerManager() {
			delete m_layers;
		}

		void cleanUp();

		void push_layer(Layer* layer);
		void push_overlay(Layer* layer);
		void pop_layer(Layer* layer);
		void pop_overlay(Layer* layer);

		std::vector<Layer*>::iterator begin() { return m_layers->begin(); }
		std::vector<Layer*>::iterator end() { return m_layers->end(); }
		std::vector<Layer*>::reverse_iterator rbegin() { return m_layers->rbegin(); }
		std::vector<Layer*>::reverse_iterator rend() { return m_layers->rend(); }

		std::vector<Layer*>::const_iterator begin() const { return m_layers->begin(); }
		std::vector<Layer*>::const_iterator end() const { return m_layers->end(); }
		std::vector<Layer*>::const_reverse_iterator rbegin() const { return m_layers->rbegin(); }
		std::vector<Layer*>::const_reverse_iterator rend() const { return m_layers->rend(); }
	private:
		std::vector<Layer*>* m_layers;
		uint32_t m_layerInsertIndex{ 0 };




	};

}
