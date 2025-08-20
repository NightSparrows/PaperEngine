
#include <PaperEngine/core/Window.h>

#include "GLFWWindow.h"

namespace PaperEngine {
	Scope<Window> Window::Create(const WindowProps& props)
	{
		return CreateScope<GLFWWindow>(props);
	}
}
