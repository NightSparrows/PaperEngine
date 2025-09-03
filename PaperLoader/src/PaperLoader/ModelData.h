#pragma once

#include <PaperEngine/graphics/Mesh.h>
#include <PaperEngine/graphics/Material.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

namespace PaperEngine {

	// mesh裡的bone 沒有relation關係
	struct BoneData
	{
		uint32_t id{UINT32_MAX};			// index of this bone
		glm::mat4 offsetMatrix = glm::identity<glm::mat4>();
	};

	// 不一定effect mesh 所以我稱為Joint
	struct JointData
	{
		// 如果這個joint是bone
		uint32_t boneId{ UINT32_MAX };

		std::string name;
		glm::mat4 transformation = glm::identity<glm::mat4>();

		std::weak_ptr<JointData> parent;
		std::list<Ref<JointData>> children;
	};

	struct ModelData
	{
		// Load a model will load only one mesh
		MeshHandle mesh;

		// Animated mesh stuffs
#pragma region Animated mesh stuffs
		// [boneName, data]
		std::unordered_map<std::string, BoneData> bones;
		Ref<JointData> rootJoint;
#pragma endregion

	};

}
