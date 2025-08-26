
#include <mono/jit/jit.h>

#include <PaperEngine/core/Base.h>
#include "ScriptEngine.h"

namespace PaperEngine {

	struct ScriptEngineData
	{
		MonoDomain* m_domain = nullptr;
	};

	static ScriptEngineData s_data;

	static void InitMono() {

		s_data.m_domain = mono_jit_init("PaperEngineDomain");

		PE_CORE_ASSERT(s_data.m_domain);
	}

	static void ShutdownMono() {
		mono_jit_cleanup(s_data.m_domain);
		s_data.m_domain = nullptr;
	}

	void ScriptEngine::Init() {
		InitMono();
	}

	void ScriptEngine::Shutdown() {
		ShutdownMono();
	}
}
