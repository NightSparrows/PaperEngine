#include "File.h"

namespace PaperEngine {

    File::File(const std::filesystem::path& filePath) :
        m_path(filePath)
    {

    }

    void File::setFilePath(const std::filesystem::path& filePath)
    {
        m_path = filePath;
    }

    void* File::readFully(size_t& size)
    {
        std::ifstream file(m_path, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            return nullptr;
        }

        size_t fileSize = file.tellg();
        
        file.seekg(0, std::ios::beg);
        void* data = malloc(fileSize);
        file.read(static_cast<char*>(data), fileSize);
        if (!file.good()) {
            free(data);
            return nullptr;
        }
        size = fileSize;

        return data;
    }

}
