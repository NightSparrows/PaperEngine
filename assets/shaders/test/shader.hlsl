

#include "../utils/nvrhi_helper.hlsli"

#pragma pack_matrix(row_major)


struct GlobalData
{
	float4x4 proj;
	float4x4 view;
	float4x4 viewProj;
	float3 cameraPos;
	float padding0;
	uint directionalLightCount;
	uint pointLightCount;
	uint spotLightCount;
	uint _pad1;
};

DECLARE_CONSTANT_BUFFER(GlobalData, g_globalData, 0, 0);

struct DirectionalLightData
{
	// 不使用float3的原因是因為他會被pad成16bytes
	float x, y, z;		// direction vector
	float r, g, b;
};

DECLARE_STRUCTURE_BUFFER_SRV(DirectionalLightData, g_directionalLightData, 0, 0);

struct PointLightData
{
	float x, y, z;		// position vector
	float r, g, b;
	float radius;		// pad to 32 bytes
};

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
	float3 normal : NORMAL;
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
	output.normal = mul(float4(input.normal, 0.0), entityData.trans).xyz;
	return output;
}

PS_OUTPUT main_ps(PS_INPUT input)
{
	PS_OUTPUT output;
	

	/// Light calculation
	float3 totalDiffuse = float3(0, 0, 0);
	
	for (uint i = 0; i < g_globalData.directionalLightCount; i++)
	{
		DirectionalLightData lightData = g_directionalLightData[i];
		float3 toLightVector = -float3(lightData.x, lightData.y, lightData.z);
		
		float3 unitNormal = normalize(input.normal);
		float3 unitLightVector = normalize(toLightVector);
		
		float nDotl = dot(unitNormal, unitLightVector);
		float brightness = max(0.2, nDotl);
		float3 diffuse = brightness * float3(lightData.r, lightData.g, lightData.b);
		
		totalDiffuse += diffuse;

	}
	/// End Light calculation
	
	output.col = float4(totalDiffuse, 1.0) * texture0.Sample(sampler0, input.uv);
	
	return output;
}
