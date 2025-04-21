#pragma once

#include <iostream>

#include <PaperEngine/core/Base.h>
#include <PaperEngine/events/Event.h>
#include <PaperEngine/core/Timestep.h>

namespace PaperEngine {

	class PE_API Layer {

	public:
		Layer() = default;
		virtual ~Layer() = default;

		virtual void on_attach() {}
		virtual void on_detach() {}
		virtual void on_update(Timestep delta_time) {}
		virtual void on_event(Event& e) {}

		virtual void on_imgui_render() {}

	};

}
