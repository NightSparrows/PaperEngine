#pragma once

#include <PaperEngine/events/Event.h>

#include <PaperEngine/core/KeyCodes.h>

namespace PaperEngine {

	class KeyEvent : public Event {
	public:
		KeyCode get_key_code() const { return m_keyCode; }

		int get_scancode() const { return m_scancode; }

		int get_mods() const { return m_mods; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
		KeyEvent(const KeyCode keycode, int scancode, int mods) : m_keyCode(keycode), m_scancode(scancode), m_mods(mods) {}

		KeyCode m_keyCode;
		int m_scancode{ 0 };
		int m_mods{ 0 };
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(const KeyCode keycode, int scancode, int mods, bool isRepeat = false)
			: KeyEvent(keycode, scancode, mods), m_IsRepeat(isRepeat) {
		}

		bool IsRepeat() const { return m_IsRepeat; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_keyCode << " (repeat = " << m_IsRepeat << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		bool m_IsRepeat;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(const KeyCode keycode, int scancode, int mods)
			: KeyEvent(keycode, scancode, mods) {
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_keyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(const KeyCode keycode)
			: KeyEvent(keycode, 0, 0) {
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_keyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)
	};
}
