#pragma once

#include <filesystem>
#include <fstream>

#include <PaperEngine/core/Base.h>

namespace PaperEngine {

	/// <summary>
	/// 簡單的檔案IO class
	/// </summary>
	class File {
	public:
		PE_API File() = default;
		PE_API File(const std::filesystem::path& filePath);
		
		PE_API void* readFully(size_t& size);

		PE_API void setFilePath(const std::filesystem::path& filePath);

		inline PE_API const std::filesystem::path& getFilePath() const { return m_path; }

	private:
		std::filesystem::path m_path;
	};

}
