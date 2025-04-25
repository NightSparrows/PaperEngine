
#version 450

layout (set = 0, binding = 0) uniform globalBuffer {
	mat4 projectionMatrix;
	mat4 viewMatrix;
} globalData;

layout (set = 2, binding = 0) uniform instanceInfo {
	int meshType;
	mat4 transformationMatrix;
}in_instanceInfo;

struct BasicVertexData {
	vec3 position;
	vec3 normal;
	vec2 uv;
};

layout (set = 2, binding = 1) readonly buffer basicVertexBuffer {
	BasicVertexData vertexData[];
}in_basicVertexBuffer;

struct BoneVertexData {
	ivec4 boneIndices;
	vec4 weights;
};

layout (set = 2, binding = 2) readonly buffer boneVertexBuffer {
	BoneVertexData vertexBoneData[];
}in_boneVertexBuffer;

struct BoneTransformData {
	mat4 transform;
};

layout (set = 2, binding = 3) readonly buffer boneTransformBuffer {
	BoneTransformData boneTransform[];
}in_boneTransformBuffer;

// output
layout (location = 0) out vec2 vs_texCoord;

void main() {
	
	vec3 in_position = in_basicVertexBuffer.vertexData[gl_VertexIndex].position;
	vec3 in_normal = in_basicVertexBuffer.vertexData[gl_VertexIndex].normal;
	vec2 in_texCoord = in_basicVertexBuffer.vertexData[gl_VertexIndex].uv;

	gl_Position = 
		globalData.projectionMatrix * 
		globalData.viewMatrix * 
		in_instanceInfo.transformationMatrix * 
		vec4(in_position, 1.0);
	
	vs_texCoord = in_texCoord;
	
}