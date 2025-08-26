#pragma once

#include <string>
#include <map>

#include <mono/jit/jit.h>

#include <PaperEngine/core/Base.h>

#include "Assembly.h"

namespace PaperEngine::Mono {

	/// <summary>
	/// Application Domain
	/// 跟Root domain沒關係
	/// </summary>
	class Domain {
	public:
		Domain(const std::string_view& name);
		~Domain();

		/// <summary>
		/// Mono特性，load的assembly就不能unload
		/// </summary>
		/// <param name="filePath"></param>
		/// <returns></returns>
		Assembly loadAssembly(const std::filesystem::path& filePath);

		/// <summary>
		/// Load或給予已載入的Assembly
		/// </summary>
		/// <param name="filePath"></param>
		/// <returns></returns>
		Assembly getAssembly(const std::filesystem::path& filePath);

		/// <summary>
		/// 重新建立domain並reload 之前assemblies
		/// </summary>
		void reload();

		static void SetCurrentDomain(Domain* domain);

	private:
		static Assembly LoadAssemblyImpl(MonoDomain* domain, const std::filesystem::path& filePath);

	private:
		std::string m_name;
		MonoDomain* m_domain = nullptr;
		std::map<std::filesystem::path, Assembly> m_assemblies;
	};

}
