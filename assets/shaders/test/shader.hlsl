

#include "../utils/nvrhi_helper.hlsli"

#pragma pack_matrix(row_major)


struct GlobalData
{
	float4x4 proj;
	float4x4 view;
	float4x4 viewProj;
	float3 cameraPos;
	float padding0;			// 一定要pad
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
	float radius;
};

DECLARE_STRUCTURE_BUFFER_SRV(PointLightData, g_pointLightData, 1, 0);
DECLARE_STRUCTURE_BUFFER_SRV(uint, g_globalLightIndices, 2, 0);

struct ClusterRange
{
	uint offset;
	uint count;
};
DECLARE_STRUCTURE_BUFFER_SRV(ClusterRange, g_clusterRanges, 3, 0);



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
	float3 worldPos : WORLD_POSITION;
	float3 viewPos : VIEW_POSITION;
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
	float4 viewPosition = mul(worldPosition, g_globalData.view);
	output.pos = mul(worldPosition, g_globalData.viewProj);
	output.uv = input.uv;
	output.normal = mul(float4(input.normal, 0.0), entityData.trans).xyz;
	output.worldPos = worldPosition.xyz;
	output.viewPos = viewPosition.xyz;
	return output;
}

PS_OUTPUT main_ps(PS_INPUT input)
{
	PS_OUTPUT output;
	

	/// Light calculation
	float3 totalDiffuse = float3(0, 0, 0);
	
	// direction light
	for (uint i = 0; i < g_globalData.directionalLightCount; i++)
	{
		DirectionalLightData lightData = g_directionalLightData[i];
		float3 toLightVector = -float3(lightData.x, lightData.y, lightData.z);
		
		float3 unitNormal = normalize(input.normal);
		float3 unitLightVector = normalize(toLightVector);
		
		float nDotl = dot(unitNormal, unitLightVector);
		float brightness = max(0, nDotl);
		float3 diffuse = brightness * float3(lightData.r, lightData.g, lightData.b);
		
		totalDiffuse += diffuse;

	}
	
	const uint c_numXSlices = 32;
	const uint c_numYSlices = 32;
	const uint c_numZSlices = 32;
	const float c_nearPlane = 0.1f;
	const float c_farPlane = 1000.f;
	
	// --- Clustered point lights ---
	float4 clipPos = mul(float4(input.worldPos, 1.0), g_globalData.viewProj);
	//float4 viewPos = mul(float4(input.worldPos, 1.0), g_globalData.view);
	float3 ndcPos = clipPos.xyz / clipPos.w;	
	
	uint clusterX = clamp(uint(floor((ndcPos.x + 1.0) * 0.5 * c_numXSlices)), 0, c_numXSlices - 1);
	uint clusterY = clamp(uint(floor((ndcPos.y + 1.0) * 0.5 * c_numYSlices)), 
	0, c_numYSlices - 1);
	
	// NDC z → view space depth (DirectX 0~1)
	float viewZ = c_nearPlane * c_farPlane / (c_farPlane - ndcPos.z * (c_farPlane - c_nearPlane));
	// 對數分割 z → clusterZ
	float slice = log(abs(input.viewPos.z) / c_nearPlane) / log(c_farPlane / c_nearPlane);
	uint clusterZ = min(uint(floor(slice * c_numZSlices)), c_numZSlices - 1);
	clusterZ = clamp(clusterZ, 0, c_numZSlices - 1);
	//uint clusterZ = 0;
	
	uint clusterIndex = clusterX + clusterY * c_numXSlices + clusterZ * c_numXSlices * c_numYSlices;
	ClusterRange range = g_clusterRanges[clusterIndex];
	for (uint clusterLightIndex = 0; clusterLightIndex < range.count; clusterLightIndex++)
	{
		uint lightIdx = g_globalLightIndices[range.offset + clusterLightIndex];
		PointLightData light = g_pointLightData[lightIdx];
		
		float3 toLightVector = float3(light.x, light.y, light.z) - input.worldPos;
		float dist = max(length(toLightVector), 0.0001f);

		float3 lightDir = toLightVector / dist; // let it unit
		
		float NDotL = max(dot(normalize(input.normal), lightDir), 0.0);
		//float attenuation = (1.0 / max(1, dist * dist)) * (1 - pow(dist / light.radius, 4));
		float attenuation = saturate(1.0f - (dist / light.radius)); // 線性衰減 0~1
		attenuation *= attenuation; // 可選二次衰減效果
		//float attenuation = 1;
		float3 diffuse = NDotL * attenuation * float3(light.r, light.g, light.b);
		
		totalDiffuse += diffuse;
	}
	/// End Light calculation
	output.col = float4(totalDiffuse, 1.0) * texture0.Sample(sampler0, input.uv);
	
	
	// debug
	//output.col = float4(float(clusterX) / c_numXSlices, float(clusterY) / c_numYSlices, float(clusterZ) / c_numZSlices, 1.0);
	//float zColor = float(clusterZ) / float(c_numZSlices - 1); // 0~1
	//output.col = float4(zColor, 0, 1 - zColor, 1.0); // 從藍到紅

	// test
	//float4 clusterColor = float4(0, 0, float(range.count) * 0.5, 1.0);
	//if (range.count >= 20)
	//{
	//	clusterColor.r = 0.5f;
	//}
	
	//output.col = clusterColor;
	/// end test
	
	return output;
}
