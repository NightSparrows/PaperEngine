#pragma once

#include <PaperEngine/events/Event.h>

namespace PaperEngine {

	/// <summary>
	/// 當視窗的DPI改變時
	/// </summary>
	class DisplayScaleChangedEvent : public Event
	{
	public:
		DisplayScaleChangedEvent(float scaleX, float scaleY)
			: m_scaleX(scaleX), m_scaleY(scaleY) {
		}
		float getScaleX() const { return m_scaleX; }
		float getScaleY() const { return m_scaleY; }
		
		std::string toString() const override
		{
			std::stringstream ss;
			ss << "DisplayScaleChangedEvent: " << m_scaleX << ", " << m_scaleY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(DisplayScaleChanged)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		float m_scaleX, m_scaleY;
	};

	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(unsigned int width, unsigned int height)
			: m_Width(width), m_Height(height) {
		}

		unsigned int GetWidth() const { return m_Width; }
		unsigned int GetHeight() const { return m_Height; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		unsigned int m_Width, m_Height;
	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;

		EVENT_CLASS_TYPE(WindowClose)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class AppTickEvent : public Event
	{
	public:
		AppTickEvent() = default;

		EVENT_CLASS_TYPE(AppTick)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent() = default;

		EVENT_CLASS_TYPE(AppUpdate)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() = default;

		EVENT_CLASS_TYPE(AppRender)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

}