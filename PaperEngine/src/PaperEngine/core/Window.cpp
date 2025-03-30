
#include <PaperEngine/core/Window.h>
#include <Platform/Windows/core/WindowsWindow.h>

namespace PaperEngine {
	Scope<Window> Window::Create(const WindowProps& props)
	{
		return CreateScope<WindowsWindow>(props);
	}
}
