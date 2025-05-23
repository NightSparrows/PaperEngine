#include "AssimpLoader.h"
#include <PaperEngine/core/Logger.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace PaperEngine {

    static bool LoadMaterials(ModelHandle model, const aiScene* scene) {
        
        if (scene->HasMaterials())
            return true;

        for (uint32_t materialIdx = 0; materialIdx < scene->mNumMaterials; materialIdx++) {
            const auto aMaterial = scene->mMaterials[materialIdx];

            
            // TODO

        }

        return true;
    }

    ModelHandle AssimpLoader::LoadModel(const std::string& path)
    {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

        if (!scene) {
            PE_CORE_ERROR("Failed to load scene using assimp from {}", path);
            return nullptr;
        }

        auto model = std::make_shared<Model>();

        if (LoadMaterials(model, scene)) {
            return nullptr;
        }

        return model;
    }

}
