#pragma once

#include <filesystem>
#include <fstream>

#include <PaperEngine/core/Base.h>

namespace PaperEngine {

	struct FileData
	{
		~FileData()
		{
			if (data)
				free(data);
			size = 0;
			data = nullptr;
		}

		void* data = nullptr;
		size_t size = 0;
	};

	/// <summary>
	/// 簡單的檔案IO class
	/// </summary>
	class File {
	public:
		PE_API File() = default;
		PE_API File(const std::filesystem::path& filePath);
		
		PE_API Ref<FileData> readBinaryFully();

		PE_API void setFilePath(const std::filesystem::path& filePath);

		inline PE_API const std::filesystem::path& getFilePath() const { return m_path; }

	private:
		std::filesystem::path m_path;
	};

}
