//glsl version 4.5
#version 450

layout (set = 1, binding = 0) uniform sampler2D textureSampler;

layout (location = 0) in vec2 vs_texCoord;

//output write
layout (location = 0) out vec4 outFragColor;

void main()
{
	
	

	//return red
	outFragColor = texture(textureSampler, vs_texCoord);
}