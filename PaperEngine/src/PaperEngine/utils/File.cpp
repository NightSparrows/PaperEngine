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

    Ref<FileData> File::readBinaryFully()
    {
        std::ifstream file(m_path, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            return nullptr;
        }

        auto fileData = CreateRef<FileData>();
        fileData->size = file.tellg();
        

        file.seekg(0, std::ios::beg);
        fileData->data = malloc(fileData->size);
        file.read(static_cast<char*>(fileData->data), fileData->size);
        if (!file.good()) {
            return nullptr;
        }

        return fileData;
    }

}
