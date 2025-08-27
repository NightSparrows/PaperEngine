

#include "../../../shaders/utils/nvrhi_helper.hlsli"


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

DECLARE_STRUCTURE_BUFFER_SRV(EntityData, g_entityData, 0, 1);


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
};

PS_INPUT main_vs(VS_INPUT input)
{
	PS_INPUT output;
	EntityData entityData = g_entityData[input.instanceID];
	float4 worldPosition = mul(float4(input.pos, 1.0f), entityData.trans);
	output.pos = mul(worldPosition, g_globalData.viewProj);
	return output;
}
