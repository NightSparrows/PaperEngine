#pragma once

#include <PaperEngine/core/UUID.h>

namespace PaperEngine
{

	struct IDComponent
	{
		UUID id;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
	};

}
