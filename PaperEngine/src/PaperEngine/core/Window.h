//
// Created by NS on 2025/3/29.
//
#pragma once

#include <functional>

#include <glm/glm.hpp>

#include <PaperEngine/core/Base.h>
#include <PaperEngine/events/Event.h>

namespace PaperEngine {

	struct WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		bool disableMinimizeBox{ false };

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
		virtual void cleanUp() = 0;

		virtual bool shouldClose() const = 0;

		virtual void onUpdate() = 0;

		virtual void setTitle(const std::string& title) = 0;

		virtual glm::vec2 getCursorDeltaPosition() const = 0;

		virtual void setEventCallback(const EventCallbackFn& callback) = 0;

		virtual void* getNativeWindow() = 0;

		virtual bool createSurface(void* instance_ptr, void* surface_ptr) = 0;

		virtual uint32_t getWidth() const = 0;
		virtual uint32_t getHeight() const = 0;

		static Scope<Window> Create(const WindowProps& props = WindowProps());

	};
} // namespace PaperEngine
