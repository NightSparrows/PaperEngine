

#include "../utils/nvrhi_helper.hlsli"

#pragma pack_matrix(row_major)


struct GlobalData
{
	float4x4 proj;
	float4x4 view;
	float4x4 viewProj;
	float3 cameraPos;
	float padding0;
};

DECLARE_CONSTANT_BUFFER(GlobalData, g_globalData, 0, 0);

struct EntityData
{
	float4x4 trans;
};

//struct BoneTransformation
//{
//	float4x4 trans;
//};

DECLARE_STRUCTURE_BUFFER_SRV(EntityData, g_entityData, 0, 1);

// 如果是animated
//DECLARE_STRUCTURE_BUFFER_SRV(BoneTransformation, g_boneTrans, 1, 1);

DECLARE_SAMPLER(sampler0, 0, 2);
DECLARE_TEXTURE2D_SRV(texture0, 0, 2);

struct VS_INPUT
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
	uint instanceID : SV_InstanceID;
};

struct PS_INPUT
{
	float4 pos : SV_Position;
	float2 uv : TEXCOORD0;
};

struct PS_OUTPUT
{
	float4 col : SV_Target0;
};

PS_INPUT main_vs(VS_INPUT input)
{
	PS_INPUT output;
	EntityData entityData = g_entityData[input.instanceID];
	float4 worldPosition = mul(float4(input.pos, 1.0f), entityData.trans);
	output.pos = mul(worldPosition, g_globalData.viewProj);
	output.uv = input.uv;
	return output;
}

PS_OUTPUT main_ps(PS_INPUT input)
{
	PS_OUTPUT output;
	output.col = texture0.Sample(sampler0, input.uv);
	return output;
}
