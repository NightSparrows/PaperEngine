#include "Assembly.h"

#include "PaperEngine/core/Logger.h"

namespace PaperEngine::Mono {

	Assembly::Assembly(MonoAssembly* assembly) :
		m_assembly(assembly)
	{
		m_image = mono_assembly_get_image(assembly);
	}

}
