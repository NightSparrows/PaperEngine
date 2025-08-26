
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>

#include <PaperEngine/core/Base.h>
#include "ScriptEngine.h"
#include <PaperEngine/utils/File.h>

#include <PaperEngine/scripting/mono/Domain.h>

namespace PaperEngine {

	struct ScriptEngineData
	{
		MonoDomain* rootDomain = nullptr;

		/// <summary>
		/// 放PaperEngine-ScriptCore和
		/// Project的C# DLL
		/// </summary>
		Mono::Domain* appDomain = nullptr;
		bool enableDebugging = true;
	};

	static ScriptEngineData s_data;

	static void InitMono() {

		if (s_data.enableDebugging)
		{
			const char* argv[2] = {
				"--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=MonoDebugger.log",
				"--soft-breakpoints"
			};

			mono_jit_parse_options(2, (char**)argv);
			mono_debug_init(MONO_DEBUG_FORMAT_MONO);
		}

		s_data.rootDomain = mono_jit_init("PaperEngineDomain");
		PE_CORE_ASSERT(s_data.rootDomain);

		if (s_data.enableDebugging)
			mono_debug_domain_create(s_data.rootDomain);

		mono_thread_set_main(mono_thread_current());

		s_data.appDomain = new Mono::Domain("PaperAppDomain");
	}

	static void ShutdownMono() {

		mono_domain_set(mono_get_root_domain(), false);

		delete s_data.appDomain;
		s_data.appDomain = nullptr;

		if (s_data.enableDebugging)
			mono_debug_cleanup();

		mono_jit_cleanup(s_data.rootDomain);
		s_data.rootDomain = nullptr;
	}

	bool ScriptEngine::LoadAppAssembly(const std::filesystem::path& filePath)
	{
		if (!s_data.appDomain) {
			return false;
		}
		s_data.appDomain->loadAssembly(filePath);
		return false;
	}

	void ScriptEngine::ReloadAppAssembly()
	{
		mono_domain_set(mono_get_root_domain(), false);

		s_data.appDomain->reload();
		// TODO register C++ functions again
		// TODO Retrieve and instantiate class Entity
	}

	void ScriptEngine::Init() {
		InitMono();
		// TODO register C++ functions for C#
		// TODO load PaperEngine-ScriptCore
#ifdef PE_DEBUG	// 暫時的
		if (!s_data.appDomain->loadAssembly("build/PaperEngine-ScriptCore/Debug/PaperEngine-ScriptCore.dll")) {
			PE_CORE_ASSERT(false, "Failed to load engine core script.");
		}
#endif // PE_DEBUG
		// TODO Retrieve and instantiate class Entity

	}

	void ScriptEngine::Shutdown() {
		ShutdownMono();
	}
}
