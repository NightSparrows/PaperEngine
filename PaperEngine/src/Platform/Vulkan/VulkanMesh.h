#pragma once

#include <vulkan/vulkan.h>

#include <PaperEngine/renderer/Mesh.h>

namespace PaperEngine {

	class VulkanMesh : public Mesh {
	public:
		VulkanMesh(MeshType type) : Mesh(type) {}


	private:

		VkBuffer m_buffer{ VK_NULL_HANDLE };			// the buffer which store the mesh data

	};

}
