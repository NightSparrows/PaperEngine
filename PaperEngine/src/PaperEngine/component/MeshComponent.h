#pragma once

#include <PaperEngine/core/Base.h>

#include <PaperEngine/renderer/Mesh.h>
#include <PaperEngine/renderer/Buffer.h>
#include <PaperEngine/renderer/Material.h>
#include <PaperEngine/renderer/DescriptorSet.h>
#include <PaperEngine/core/Application.h>

namespace PaperEngine {

	struct MeshComponent {
		/// <summary>
		/// I think this mesh is not belong to this component
		/// for memory saving, but if the animator need to change the mesh
		/// you need to clone the mesh to let the mesh only use by this mesh component
		/// </summary>
		MeshHandle mesh;

		// only use in render thread
		// mesh can be load in other thread, but the scene component edit must to in render thread
		void setMesh(MeshHandle mesh) {
			this->mesh = mesh;
			const uint32_t swapchainCount = Application::Get().get_window().get_context().get_swapchain_image_count();
			if (instanceSets.size() != swapchainCount) {
				instanceSets.resize(swapchainCount);
			}
			// reset the descriptor set
			instanceSets[Application::Get().get_window().get_context().get_current_swapchain_index()] = nullptr;

#pragma region Create (initialize) bone transformation buffer for each swapchain in flight
			if (mesh->get_type() == Animated) {
				BufferSpecification boneTransformBufferSpec;
				boneTransformBufferSpec
					.setSize(mesh->get_bone_count() * sizeof(glm::mat4))
					.setIsStorageBuffer(true);
				this->boneTransformBuffer.resize(swapchainCount);
				for (uint32_t i = 0; i < swapchainCount; i++) {
					this->boneTransformBuffer[i] = Buffer::Create(boneTransformBufferSpec);
				}
			}
#pragma endregion

		}

		// vector<Material> for materials it must be one by one to submeshes
		std::vector<MaterialHandle> materials;

		// Control by the mesh renderer, do not modify this
		// there frame in flight instance set
		mutable std::vector<DescriptorSetHandle> instanceSets;

		/// <summary>
		/// Control by the mesh renderer, do not modify this
		/// </summary>
		mutable std::vector<BufferHandle> instanceInfoBuffer;

		// Control by the mesh renderer, do not modify this
		// bone transformations if mesh is animated (skinned)
		// the mesh component do not control how this buffer update data it just use it
		// other that control bone component will use this buffer to update its controlled bone transformations
		mutable std::vector<BufferHandle> boneTransformBuffer;
	};

}
