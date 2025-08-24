
#ifndef BINDING_HELPERS_HLSLI
#define BINDING_HELPERS_HLSLI

#if defined(SPIRV) || defined(TARGET_VULKAN) // Support old-style and new-style platform macros

#define VK_PUSH_CONSTANT [[vk::push_constant]]
#define VK_BINDING(reg,dset) [[vk::binding(reg,dset)]]
#define VK_DESCRIPTOR_SET(dset) ,space##dset
#define VK_IMAGE_FORMAT(format) [[vk::image_format(format)]]

// To use dual-source blending on Vulkan, a pixel shader must specify the same Location but different Index
// decorations for two outputs. In HLSL, that can only be achieved with explicit attributes.
// Use on the declarations of pixel shader outputs.
#define VK_LOCATION(loc) [[vk::location(loc)]]
#define VK_LOCATION_INDEX(loc,idx) [[vk::location(loc)]] [[vk::index(idx)]]

// Allows RWTexture2D<float4> to map to any compatible type
// see https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#formats-without-shader-storage-format
// shaderStorageImageWriteWithoutFormat and shaderStorageImageReadWithoutFormat 
#define VK_IMAGE_FORMAT_UNKNOWN [[vk::image_format("unknown")]]

#else

#define VK_PUSH_CONSTANT
#define VK_BINDING(reg,dset) 
#define VK_DESCRIPTOR_SET(dset)
#define VK_IMAGE_FORMAT(format)
#define VK_LOCATION(loc)
#define VK_LOCATION_INDEX(loc,idx)
#define VK_IMAGE_FORMAT_UNKNOWN

#endif

// Helper macro to expand the register and space macros before concatenating tokens.
// Declares a register space explicitly on DX12 and Vulkan, skips it on DX11.
#ifdef TARGET_D3D11
#define REGISTER_HELPER(TY,REG,SPACE) register(TY##REG)
#else
#define REGISTER_HELPER(TY,REG,SPACE) register(TY##REG, space##SPACE)
#endif

// Macros to declare bindings for various resource types in a cross-platform way
// using register and space indices coming from other preprocessor macros.
#define REGISTER_CBUFFER(reg,space) REGISTER_HELPER(b,reg,space)
#define REGISTER_SAMPLER(reg,space) REGISTER_HELPER(s,reg,space)
#define REGISTER_SRV(reg,space)     REGISTER_HELPER(t,reg,space)
#define REGISTER_UAV(reg,space)     REGISTER_HELPER(u,reg,space)

// Macro to declare a constant buffer in a cross-platform way, compatible with the VK_PUSH_CONSTANT attribute.
#ifdef TARGET_D3D11
#define DECLARE_CBUFFER(ty,name,reg,space) cbuffer c_##name : REGISTER_CBUFFER(reg,space) { ty name; }
#else
#define DECLARE_CBUFFER(ty,name,reg,space) ConstantBuffer<ty> name : REGISTER_CBUFFER(reg,space)
#endif

// Macro to declare a push constant block on Vulkan and a regular cbuffer on other platforms.
#define DECLARE_PUSH_CONSTANTS(ty,name,reg,space) VK_PUSH_CONSTANT DECLARE_CBUFFER(ty,name,reg,space)

#endif // BINDING_HELPERS_HLSLI

struct Constants
{
	float2 invDisplaySize;
};

DECLARE_PUSH_CONSTANTS(Constants, g_Const, 0, 0);

struct VS_INPUT
{
	float2 pos : POSITION;
	float2 uv : TEXCOORD0;
	float4 col : COLOR0;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
	float2 uv : TEXCOORD0;
};

PS_INPUT main_vs(VS_INPUT input)
{
	PS_INPUT output;
	output.pos.xy = input.pos.xy * g_Const.invDisplaySize * float2(2.0, -2.0) + float2(-1.0, 1.0);
	output.pos.zw = float2(0, 1);
	output.col = input.col;
	output.uv = input.uv;
	return output;
}

// #if defined(SPIRV) || defined(TARGET_VULKAN)
// [[vk::binding(1, 0)]] SamplerState sampler0  : register(s1, space0);
// [[vk::binding(0, 0)]] Texture2D    texture0  : register(t0, space0);
// #else
// sampler sampler0 : register(s1);
// Texture2D texture0 : register(t0);

#define VK_BINDING_SAMPLER(reg, space) [[vk::binding(reg + 128, space)]]
// #endif
//[[vk::binding(128)]] sampler sampler0 : register(s0, space0);
VK_BINDING_SAMPLER(0, 0) sampler sampler0 : register(s0, space0);
[[vk::binding(0)]] Texture2D texture0 : register(t0, space0);

float4 main_ps(PS_INPUT input) : SV_Target
{
	float4 out_col = input.col * texture0.Sample(sampler0, input.uv);
	return out_col;
}
