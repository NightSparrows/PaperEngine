

#include <PaperEngine/core/Application.h>
#include <PaperEngine/core/Logger.h>

#include "ModelLoader.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace PaperEngine {

    // convert matrix
    static glm::mat4 ToMatrix(const aiMatrix4x4& from);
    static void ProcessJointRelation(const aiNode* aiJointNode, Ref<ModelData> modelData, Ref<JointData> jointData);

    static bool ProcessMesh(const aiScene* aiScene, Ref<ModelData> modelData)
    {
        if (!aiScene->HasMeshes()) {
            PE_CORE_ERROR("[ModelLoader] Scene does not contain mesh!");
            return false;
        }

        // calculate total vertices
        uint32_t totalVertices = 0;
        uint32_t totalIndices = 0;
        bool hasBone = false;

        for (uint32_t i = 0; i < aiScene->mNumMeshes; i++)
        {
            const auto aiMesh = aiScene->mMeshes[i];
            totalVertices += aiMesh->mNumVertices;
            if (aiMesh->HasBones())
                hasBone = true;

            for (uint32_t j = 0; j < aiMesh->mNumFaces; j++)
            {
                const auto& aiFace = aiMesh->mFaces[i];
                for (uint32_t k = 0; k < aiFace.mNumIndices; k++)
                {
                    totalIndices++;
                }
            }
        }

        // create a mesh first
        MeshHandle mesh = CreateRef<Mesh>();

        std::vector<uint32_t> indices;
        indices.reserve(totalIndices);
        std::vector<StaticVertex> vertices;
        vertices.reserve(totalVertices);
        std::vector<SkeletalVertexInfo> boneInfos;
        if (hasBone)
        {
            boneInfos.resize(totalVertices);
        }

        uint32_t vertexIndexOffset = 0;
        uint32_t meshIndicesOffset = 0;
        uint32_t subMeshIndicesCount = 0;
        uint32_t currentBoneIndex = 0;

        for (uint32_t i = 0; i < aiScene->mNumMeshes; i++)
        {
            const auto aiMesh = aiScene->mMeshes[i];

            struct BoneWeight
            {
                uint32_t boneId = 0;
                float weight = 0;
            };
            std::unordered_map<uint32_t, std::vector<BoneWeight>> vwList;

            // TODO process bones
            for (uint32_t j = 0; j < aiMesh->mNumBones; j++)
            {
                const auto aiBone = aiMesh->mBones[j];

                auto& boneData = modelData->bones[aiBone->mName.C_Str()];

                boneData.id = currentBoneIndex;
                boneData.offsetMatrix = ToMatrix(aiBone->mOffsetMatrix);

                for (uint32_t k = 0; k < aiBone->mNumWeights; k++)
                {
                    const auto& aiWeight = aiBone->mWeights[k];

                    uint32_t vertexIndex = vertexIndexOffset + aiWeight.mVertexId;
                    vwList[vertexIndex].emplace_back(currentBoneIndex , aiWeight.mWeight);
                }
                currentBoneIndex++;
            }

            // process indices
            subMeshIndicesCount = 0;            // reset the count for the submesh
            for (uint32_t j = 0; j < aiMesh->mNumFaces; j++)
            {
                const auto& aiFace = aiMesh->mFaces[j];
                for (uint32_t k = 0; k < aiFace.mNumIndices; k++)
                {
                    indices.emplace_back(meshIndicesOffset + aiFace.mIndices[k]);
                    subMeshIndicesCount++;
                }
            }

            // process vertices
            for (uint32_t j = 0; j < aiMesh->mNumVertices; j++)
            {
                uint32_t currentVertexIndex = vertexIndexOffset + j;

                const auto& aiVertex = aiMesh->mVertices[j];

                auto& currentVertex = vertices.emplace_back();

                currentVertex.position.x = aiVertex.x;
                currentVertex.position.y = aiVertex.y;
                currentVertex.position.z = aiVertex.z;

                if (aiMesh->HasTextureCoords(0))
                {
                    const auto& aiTexCoords = aiMesh->mTextureCoords[0][j];

                    currentVertex.texcoord.x = aiTexCoords.x;
                    currentVertex.texcoord.y = aiTexCoords.y;
                }

                if (aiMesh->HasNormals())
                {
                    const auto& aiNormal = aiMesh->mNormals[j];
                    currentVertex.normal.x = aiNormal.x;
                    currentVertex.normal.y = aiNormal.y;
                    currentVertex.normal.z = aiNormal.z;
                }

                // process bone infos per vertex
                if (vwList.contains(currentVertexIndex))
                {
                    const auto& list = vwList[currentVertexIndex];
                    for (uint32_t k = 0; k < 4; k++)    // TODO change to MAX_BONE_PER_VERTEX definition
                    {
                        if (k < list.size()) {
                            boneInfos[currentVertexIndex].boneIndices[k] = list[k].boneId;
                            boneInfos[currentVertexIndex].boneWeights[k] = list[k].weight;
                        }
                    }
                }

                currentVertexIndex++;
            }

            // set the submesh info
            auto& subMeshInfo = mesh->getSubMeshes().emplace_back();
            subMeshInfo.indicesCount = subMeshIndicesCount;
            subMeshInfo.indicesOffset = meshIndicesOffset;
            subMeshInfo.materialIndex = aiMesh->mMaterialIndex;

            meshIndicesOffset += subMeshIndicesCount;
            vertexIndexOffset += aiMesh->mNumVertices;
        }

        // upload to gpu in mesh
        nvrhi::CommandListHandle cmd = Application::Get()->GetNVRHIDevice()->createCommandList();

        cmd->open();
        mesh->loadIndexBuffer(cmd, indices.data(), totalIndices);
        if (hasBone)
        {
            // Process Joint Relationship
            modelData->rootJoint = CreateRef<JointData>();
            ProcessJointRelation(aiScene->mRootNode, modelData, modelData->rootJoint);
            mesh->loadSkeletalMesh(cmd, vertices, boneInfos);
        }
        else {
            mesh->loadStaticMesh(cmd, vertices);
        }
        cmd->close();
        Application::Get()->GetNVRHIDevice()->executeCommandList(cmd);

        modelData->mesh = mesh;

        return true;
    }

    static void ProcessJointRelation(const aiNode* aiJointNode, Ref<ModelData> modelData, Ref<JointData> jointData)
    {
        jointData->name = aiJointNode->mName.C_Str();
        jointData->transformation = ToMatrix(aiJointNode->mTransformation);
        if (modelData->bones.contains(jointData->name))
        {
            // 這個joint是bone
            const auto& boneData = modelData->bones[jointData->name];
            jointData->boneId = boneData.id;
        }

        for (uint32_t i = 0; i < aiJointNode->mNumChildren; i++)
        {
            const aiNode* aiChildNode = aiJointNode->mChildren[i];

            auto childJointData = jointData->children.emplace_back(CreateRef<JointData>());
            childJointData->parent = jointData;
            ProcessJointRelation(aiChildNode, modelData, childJointData);
        }
    }

    Ref<ModelData> ModelLoader::LoadFromAssimp(const std::filesystem::path& filePath)
    {
        Assimp::Importer importer;

        auto aiScene = importer.ReadFile(filePath.string(), aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenSmoothNormals);

        if (!aiScene) {
            PE_CORE_ERROR("Failed to read file: {}", filePath.string());
            return nullptr;
        }
        auto modelData = CreateRef<ModelData>();

        if (!ProcessMesh(aiScene, modelData))
        {
            // load mesh failed
            PE_CORE_ERROR("Failed to process mesh: {}", filePath.string());
            return nullptr;
        }
        // TODO process materials

        return modelData;
    }

    static glm::mat4 ToMatrix(const aiMatrix4x4& from)
    {
        glm::mat4 to;
        //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
        to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
        to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
        to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
        to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
        return to;
    }

}
