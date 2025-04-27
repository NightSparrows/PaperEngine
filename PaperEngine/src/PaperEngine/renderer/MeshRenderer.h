#pragma once

#include "Renderer.h"
#include "Material.h"
#include "Mesh.h"
#include "DescriptorSet.h"

namespace PaperEngine {

	class MeshRenderer : public Renderer {
	public:

		/// <summary>
		/// the format of the mesh instance is
		/// 0: instance infomation buffer (per instnace)
		/// struct InfoBuffer {
		/// mat4 transformationMatrix;
		/// int meshType;
		/// };
		/// 1: basic vertex storage buffer (per vertex)
		/// struct BasicVertex {
		///		vec3 position;
		///		vec3 normal;
		///		vec2 uv;
		/// };
		/// 2: bone vertex storage buffer (per vertex)
		/// struct BoneVertex {
		///		ivec4 boneIndex;
		///		vec4 boneWeight;
		/// };
		/// 3: bone transformation storage buffer (per bone)
		/// struct BoneTransform {
		///		mat4 transform;
		/// };
		/// </summary>
		/// <returns></returns>
		virtual DescriptorSetHandle allocateInstanceSet() = 0;

	public:

		PE_API static Ref<MeshRenderer> Create();

	};

}
