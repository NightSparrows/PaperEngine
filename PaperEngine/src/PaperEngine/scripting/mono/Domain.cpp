#include "Domain.h"
#include <mono/metadata/assembly.h>
#include "PaperEngine/utils/File.h"

namespace PaperEngine::Mono {

	Domain::Domain(const std::string_view& name) :
		m_name(name)
	{
		m_domain = mono_domain_create_appdomain(m_name.data(), nullptr);
	}

	Domain::~Domain()
	{
		m_assemblies.clear();
		mono_domain_unload(m_domain);
		m_domain = nullptr;
	}

	Assembly Domain::loadAssembly(const std::filesystem::path& filePath)
	{
		auto resultAssembly = Domain::LoadAssemblyImpl(m_domain, filePath);
		m_assemblies[filePath] = resultAssembly;

		return resultAssembly;
	}

	Assembly Domain::getAssembly(const std::filesystem::path& filePath)
	{
		if (!m_assemblies.contains(filePath)) {
			return loadAssembly(filePath);
		}
		return m_assemblies[filePath];
	}

	void Domain::reload()
	{
		mono_domain_unload(m_domain);
		m_domain = mono_domain_create_appdomain(m_name.data(), nullptr);
		for (auto& [filePath, assembly] : m_assemblies) {

			m_assemblies[filePath] = Domain::LoadAssemblyImpl(m_domain, filePath);

		}

	}

	void Domain::SetCurrentDomain(Domain* domain)
	{
		mono_domain_set(domain->m_domain, 1);
		mono_domain_get();
	}

	Assembly Domain::LoadAssemblyImpl(MonoDomain* domain, const std::filesystem::path& filePath)
	{
		File file(filePath);

		if (mono_domain_get() != domain)
			mono_domain_set(domain, 1);


		size_t fileSize = 0;
		void* fileData = file.readFully(fileSize);

		if (!fileData) {
			PE_CORE_ERROR("[ScriptEngine] Failed to read dll file: {}", filePath.string());
			return {};
		}
		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(
			static_cast<char*>(fileData),
			static_cast<uint32_t>(fileSize),
			1,
			&status,
			0);
		free(fileData);

		if (status != MONO_IMAGE_OK)
		{
			PE_CORE_ERROR("[ScriptEngine] Failed to open mono image: {}", mono_image_strerror(status));
			return nullptr;
		}

		MonoAssembly* assembly = mono_assembly_load_from_full(
			image,
			filePath.string().c_str(),
			&status,
			0);
		mono_image_close(image);

		Assembly resultAssembly(assembly);

		return resultAssembly;
	}

}
