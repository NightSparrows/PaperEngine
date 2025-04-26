//glsl version 4.6
#version 460

// material set is 0
layout (set = 1, binding = 1) uniform sampler2D textureSampler;

layout (location = 0) in vec2 vs_texCoord;

//output write
layout (location = 0) out vec4 outFragColor;

void main()
{
	
	outFragColor = vec4(0, 1, 0, 1);

	//return red
	// outFragColor = texture(textureSampler, vs_texCoord);
}