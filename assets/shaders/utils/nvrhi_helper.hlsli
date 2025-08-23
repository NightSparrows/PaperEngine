

#ifndef NVRHI_HELPER_HLSLI
#define NVRHI_HELPER_HLSLI

#if defined(TARGET_VULKAN)
// NVRHI有自己的偏移
#define VK_BINDING_SHADER_RESOURCE(reg, dset)		[[vk::binding(reg, dset)]]
#define VK_BINDING_SAMPLER(reg, dset)				[[vk::binding(reg + 128, dset)]]
#define VK_BINDING_CONSTANT_BUFFER(reg, dset)		[[vk::binding(reg + 256, dset)]]
#define VK_BINDING_UNORDERED_ACCESS(reg, dset)		[[vk::binding(reg + 384, dset)]]

#define VK_BINDING_TEXTURE(reg, dset)				VK_BINDING_SHADER_RESOURCE(reg, dset)

#else
#define VK_BINDING_SHADER_RESOURCE(reg, dset)
#define VK_BINDING_SAMPLER(reg, dset)
#define VK_BINDING_CONSTANT_BUFFER(reg, dset)
#define VK_BINDING_UNORDERED_ACCESS(reg, dset)

#define VK_BINDING_TEXTURE(reg, dset)

#endif

#ifdef TARGET_D3D11
#define REGISTER_HELPER(TY, REG, space) register(TY##REG)
#else
#define REGISTER_HELPER(TY, REG, SPACE) register(TY##REG, space##SPACE)
#endif

#define REGISTER_CBUFFER(reg, space) REGISTER_HELPER(b, reg, space)
#define REGISTER_SAMPLER(reg, space) REGISTER_HELPER(s, reg, space)
// SRV 指Shader Resource View (Read Only)
#define REGISTER_SRV(reg,space)     REGISTER_HELPER(t,reg,space)
#define REGISTER_UAV(reg,space)     REGISTER_HELPER(u,reg,space)


// Declare 中以NVRHI為準(reg = slot, space = bindingSetIndex)

#ifdef TARGET_D3D11

#define DECLARE_CONSTANT_BUFFER(ty, name, reg, space) VK_BINDING_CONSTANT_BUFFER(reg, space) cbuffer c_##name : REGISTER_CBUFFER(reg, space) { ty name; }
#define DECLARE_SAMPLER(name, reg, space) VK_BINDING_SAMPLER(reg, space) sampler name : REGISTER_SAMPLER(reg, space)

#else

#define DECLARE_CONSTANT_BUFFER(ty, name, reg, space) VK_BINDING_CONSTANT_BUFFER(reg, space) ConstantBuffer<ty> name : REGISTER_CBUFFER(reg, space)
#define DECLARE_SAMPLER(name, reg, space) VK_BINDING_SAMPLER(reg, space) SamplerState name : REGISTER_SAMPLER(reg, space)

#endif

#define DECLARE_TEXTURE2D_SRV(name, reg, space) VK_BINDING_SHADER_RESOURCE(reg, space) Texture2D name : REGISTER_SRV(reg, space)
// ty: 結構名稱
// name: 這個變數名稱
#define DECLARE_STRUCTURE_BUFFER_SRV(ty, name, reg, space) VK_BINDING_SHADER_RESOURCE(reg, space) StructuredBuffer<ty> name : REGISTER_SRV(reg, space)

#endif
