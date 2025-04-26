
#version 460

layout (set = 0, binding = 0) uniform globalBuffer {
	mat4 projectionMatrix;
	mat4 viewMatrix;
} globalData;

layout (set = 2, binding = 0) uniform instanceInfo {
	mat4 transformationMatrix;
	int meshType;
}in_instanceInfo;

struct BasicVertexData {
	float x, y, z;
	float nx, ny, nz;
	float u, v;
};

layout (std430, set = 2, binding = 1) readonly buffer basicVertexBuffer {
	BasicVertexData vertexData[];
}in_basicVertexBuffer;

struct BoneVertexData {
	ivec4 boneIndices;
	vec4 weights;
};

layout (std430, set = 2, binding = 2) readonly buffer boneVertexBuffer {
	BoneVertexData vertexBoneData[];
}in_boneVertexBuffer;

struct BoneTransformData {
	mat4 transform;
};

layout (std430, set = 2, binding = 3) readonly buffer boneTransformBuffer {
	BoneTransformData boneTransform[];
}in_boneTransformBuffer;

// output
layout (location = 0) out vec2 vs_texCoord;

void main() {
	
	BasicVertexData vtx = in_basicVertexBuffer.vertexData[gl_VertexIndex];

	vec3 in_position = vec3(vtx.x, vtx.y, vtx.z);
	vec3 in_normal = vec3(vtx.nx, vtx.ny, vtx.nz);
	vec2 in_texCoord = vec2(vtx.u, vtx.v);

	gl_Position = 
		globalData.projectionMatrix * 
		globalData.viewMatrix * 
		in_instanceInfo.transformationMatrix * 
		vec4(in_position, 1.0);
	
	vs_texCoord = in_texCoord;
	
}