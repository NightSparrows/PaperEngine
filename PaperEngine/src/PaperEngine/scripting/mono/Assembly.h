#pragma once

#include <mono/metadata/assembly.h>

namespace PaperEngine::Mono {

	class Assembly {
		friend class Domain;
	public:
		Assembly() = default;

		operator bool() {
			return m_assembly;
		}

	protected:
		Assembly(MonoAssembly* assembly);

	private:
		MonoAssembly* m_assembly = nullptr;
		MonoImage* m_image = nullptr;
	};

}
