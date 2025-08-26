#pragma once

namespace PaperEngine {

	class ScriptEngine {
	public:

		/// <summary>
		/// 載入Application的DLL到App domain中
		/// </summary>
		/// <param name="filePath"></param>
		/// <returns></returns>
		static bool LoadAppAssembly(const std::filesystem::path& filePath);

		static void ReloadAppAssembly();

#pragma region Called by Application class
		static void Init();
		static void Shutdown();
#pragma endregion

	};

}
