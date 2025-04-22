
#version 450

layout (set= 0, binding = 0) uniform globalBuffer {
	mat4 projectionMatrix;
	mat4 viewMatrix;
} globalData;

// per vertex
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_texCoord;
layout (location = 2) in vec3 in_normal;

// per instance
layout (location = 4) in mat4 in_transformationMatrix;

// output
layout (location = 0) out vec2 vs_texCoord;

void main() {
	
	gl_Position = globalData.projectionMatrix * globalData.viewMatrix * in_transformationMatrix * vec4(in_position, 1.0);
	
	vs_texCoord = in_texCoord;
	
}