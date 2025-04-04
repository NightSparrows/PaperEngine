//
// Created by NS on 2025/3/29.
//
#pragma once

#include <PaperEngine/core/Base.h>
#include <PaperEngine/events/Event.h>

#include <functional>

#include <PaperEngine/renderer/GraphicsContext.h>

namespace PaperEngine {

	struct WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		WindowProps(const std::string& title = "Paper Engine",
			uint32_t width = 1600,
			uint32_t height = 900)
			: Title(title), Width(width), Height(height)
		{
		}
	};

    class PE_API Window {
    public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;

		virtual void init() = 0;

		virtual void on_update() = 0;

		virtual void set_event_callback(const EventCallbackFn& callback) = 0;

		virtual void* get_native_window() = 0;

        virtual uint32_t get_width() const = 0;
        virtual uint32_t get_height() const = 0;

		virtual GraphicsContext& get_context() = 0;


		static Scope<Window> Create(const WindowProps& props = WindowProps());

    };
} // namespace PaperEngine
