
#include "../utils/nvrhi_helper.hlsli"

#pragma pack_matrix(row_major)

struct GlobalData
{
	float4x4 projViewMatrix;
	float4x4 viewMatrix;
	float4x4 inverseProjMatrix;

	uint numXSlices;
	uint numYSlices;
	uint numZSlices;
	
	uint pointLightCount;
	
	float nearPlane;
	float farPlane;
	
	uint screenWidth;
	uint screenHeight;

};
DECLARE_CONSTANT_BUFFER(GlobalData, g_globalData, 0, 0);

// 可能在另一個compute shader compute  每個cluster的min max depth
// 然後放到一個buffer裡
// DECLARE_TEXTURE2D_SRV(u_depthTexture, 0, 0);

struct PointLightData
{
	float x, y, z; // position vector
	float r, g, b;
	float radius; 
};


struct ClusterRange
{
	uint offset;
	uint count;
};

DECLARE_STRUCTURE_BUFFER_SRV(PointLightData, g_pointLightData, 0, 0);
/**
uint g_globalLightIndices[];
*/
DECLARE_RW_STRUCTURE_BUFFER_UAV(uint, g_globalLightIndices, 0, 0);
/**
ClusterRange g_clusterRanges[];
*/
DECLARE_RW_STRUCTURE_BUFFER_UAV(ClusterRange, g_clusterRanges, 1, 0);
/**
uint g_globalCounter;
*/
DECLARE_RW_BYTE_ADDRESS_BUFFER_UAV(g_globalCounter, 2, 0);				// 用於計算最終把各個list算回globalLightIndices

struct AABB
{
	float3 min;
	float3 max;
};

float LinearDepthToNDC(float zView, float nearPlane, float farPlane)
{
    // DirectX NDC z ∈ [0,1]
	return (farPlane / (farPlane - nearPlane)) * (1.0 - nearPlane / zView);
}

struct Plane
{
	float3 normal;
	float distance;
};

struct Frustum
{
	Plane planes[6];
};

Plane makePlane(const float3 p0, const float3 p1, const float3 p2)
{
	Plane plane;
	plane.normal  = normalize(cross(p1 - p0, p2 - p0));
	plane.distance = dot(plane.normal, p0);
	return plane;
}

Frustum makeFrustum(const float3 corners[8])
{
	Frustum frustum;
	
	// near
	frustum.planes[0] = makePlane(corners[0], corners[1], corners[3]);
	// far
	frustum.planes[1] = makePlane(corners[5], corners[4], corners[6]);
	
    // Left  (near-lb, far-lb, far-lt)
	frustum.planes[2] = makePlane(corners[0], corners[4], corners[6]);

    // Right (near-rb, far-rb, far-rt)
	frustum.planes[3] = makePlane(corners[5], corners[1], corners[7]);

    // Top   (near-lt, far-lt, far-rt)
	frustum.planes[4] = makePlane(corners[2], corners[6], corners[7]);

    // Bottom(near-lb, far-lb, far-rb)
	frustum.planes[5] = makePlane(corners[0], corners[4], corners[5]);

	return frustum;
}

static bool SphereIntersect(const Frustum f, const float3 center, const float radius)
{
	for (int i = 2; i < 6; i++)
	{
		Plane plane = f.planes[i];
		float dist = dot(plane.normal, center) + plane.distance;
		
		if (dist < -radius)
			return false;
	}
	return true;
}

bool SphereAABBIntersect(float3 center, float radius, float3 boxMin, float3 boxMax)
{
	float3 p = clamp(center, boxMin, boxMax);
	float3 d = center - p;
	return dot(d, d) <= radius * radius;
}

static float4 ClipToView(float4 clip)
{
	// view space position
	float4 view = mul(clip, g_globalData.inverseProjMatrix);
	// Perspective projection
	view = view / view.w;
	
	return view;
}

/**
* Dispatch(X, Y, Z) 要dispatch幾個threadGroup
* numthreads(x, y, z) threadGroup有多少thread
* 
* Dispatch(numXSlices, numYSlices, numZSlices);
* 每個threadGroup處理一個cluster
* 
*/

#define MAX_LIGHT_COUNT_IN_LIST 2048

#define GROUP_THREAD_SIZE_X 8
#define GROUP_THREAD_SIZE_Y 8
#define GROUP_THREAD_SIZE_Z 1
// cluster 內的data （per threadGroup）
groupshared Frustum clusterFrustum;
groupshared float3 clusterAABB[2];	// min max
groupshared uint lightCountInCluster;
groupshared uint lightIndices[MAX_LIGHT_COUNT_IN_LIST + 4];

[numthreads(GROUP_THREAD_SIZE_X, GROUP_THREAD_SIZE_Y, GROUP_THREAD_SIZE_Z)] // 每個threadGroup使用64個thread
void main_cs(
	uint3 globalThreadID : SV_DispatchThreadID,	// Unique thread ID
	uint3 groupID : SV_GroupID,					// Dispatch 的ID (thread group ID)
	uint threadIdx : SV_GroupIndex				// 在threadGroup內個index (linear)
)
{
	// 已從外部設定g_globalCounter = 0
	//if (globalThreadID.x == 0 && globalThreadID.y == 0 && globalThreadID.z == 0)
	//{
	//	// per run
	//	g_globalCounter.Store(0, 0);
	//}
	
	const uint numberOfThreadInGroup = GROUP_THREAD_SIZE_X * GROUP_THREAD_SIZE_Y * GROUP_THREAD_SIZE_Z;
	
	if (threadIdx == 0) // per cluster
	{
		lightCountInCluster = 0;
		
		// 計算Cluster的AABB，感覺在viewspace可以不用計算（或者說只計算一次）		
		// 透視投影非線性 z 切片
		float sliceNear = g_globalData.nearPlane * pow(g_globalData.farPlane / g_globalData.nearPlane, float(groupID.z) / float(g_globalData.numZSlices));
		float sliceFar = g_globalData.nearPlane * pow(g_globalData.farPlane / g_globalData.nearPlane, float(groupID.z + 1) / float(g_globalData.numZSlices));

		
		float2 clusterNdcXYMin = float2(
			lerp(-1.0, 1.0, float(groupID.x) / float(g_globalData.numXSlices)),
			lerp(-1.0, 1.0, float(groupID.y) / float(g_globalData.numYSlices)));
		float2 clusterNdcXYMax = float2(
			lerp(-1.0, 1.0, float(groupID.x + 1) / float(g_globalData.numXSlices)),
			lerp(-1.0, 1.0, float(groupID.y + 1) / float(g_globalData.numYSlices)));

		// ray of the camera
		float4 clipCorners[8];
		int idx = 0;
		for (int y = 0; y < 2; y++)
		{
			for (int x = 0; x < 2; x++)
			{
				for (int z = 0; z < 2; z++)
				{
					float ndcX = (x == 0) ? clusterNdcXYMin.x : clusterNdcXYMax.x;
					float ndcY = (y == 0) ? clusterNdcXYMin.y : clusterNdcXYMax.y;
					clipCorners[idx++] = float4(ndcX, ndcY, -1, 1.0); // forward = -z 看向
				}
			}
		}
		float3 viewCorners[8];
		for (int i = 0; i < 4; i++)
		{
			float3 dir = ClipToView(clipCorners[i]).xyz;
			dir /= abs(dir.z); // normalize ray direction to z axis

			viewCorners[i + 0] = dir * sliceNear;
			viewCorners[i + 4] = dir * sliceFar;
		}
		float3 minPt = float3(1e9, 1e9, 1e9);
		float3 maxPt = float3(-1e9, -1e9, -1e9);

		for (int i = 0; i < 8; i++)
		{
			minPt = min(minPt, viewCorners[i]);
			maxPt = max(maxPt, viewCorners[i]);
		}
		clusterAABB[0] = minPt;
		clusterAABB[1] = maxPt;

		const float fov = radians(70.0f);
		const float aspectRatio = 2560.0 / 1440.0;
		
		float tanHalfFov = tan(0.5 * fov);
		float hNear = tanHalfFov * sliceNear;
		float wNear = hNear * aspectRatio;
		float hFar = tanHalfFov * sliceFar;
		float wFar = hFar * aspectRatio;
		
		// View space corners
		float3 corners[8];
		const float edgeBias = 0.05f;
		
		corners[0] = float3(clusterNdcXYMin.x * wNear - edgeBias, clusterNdcXYMin.y * hNear - edgeBias, -sliceNear);
		corners[1] = float3(clusterNdcXYMax.x * wNear + edgeBias, clusterNdcXYMin.y * hNear - edgeBias, -sliceNear);
		corners[2] = float3(clusterNdcXYMin.x * wNear - edgeBias, clusterNdcXYMax.y * hNear + edgeBias, -sliceNear);
		corners[3] = float3(clusterNdcXYMax.x * wNear + edgeBias, clusterNdcXYMax.y * hNear + edgeBias, -sliceNear);
		// far corners
		corners[4] = float3(clusterNdcXYMin.x * wFar - edgeBias, clusterNdcXYMin.y * hFar - edgeBias, -sliceFar);
		corners[5] = float3(clusterNdcXYMax.x * wFar + edgeBias, clusterNdcXYMin.y * hFar - edgeBias, -sliceFar);
		corners[6] = float3(clusterNdcXYMin.x * wFar - edgeBias, clusterNdcXYMax.y * hFar + edgeBias, -sliceFar);
		corners[7] = float3(clusterNdcXYMax.x * wFar + edgeBias, clusterNdcXYMax.y * hFar + edgeBias, -sliceFar);
		
		clusterFrustum = makeFrustum(corners);
		
		
		// X/Y slice in view space: 將螢幕 tile 對應到 NDC (-1~1) 再反投影到 view space near plane
		//float sliceWidth = 2.0 / float(g_globalData.numXSlices); // NDC space
		//float sliceHeight = 2.0 / float(g_globalData.numYSlices);

		//clusterAABB.min.x = -1.0 + float(groupID.x) * sliceWidth;
		//clusterAABB.min.y = -1.0 + float(groupID.y) * sliceHeight;
		//clusterAABB.max.x = -1.0 + float(groupID.x + 1) * sliceWidth;
		//clusterAABB.max.y = -1.0 + float(groupID.y + 1) * sliceHeight;
		
		//// 透視投影非線性 z 切片
		//float sliceNear = g_globalData.nearPlane * pow(g_globalData.farPlane / g_globalData.nearPlane, float(groupID.z) / float(g_globalData.numZSlices));
		//float sliceFar = g_globalData.nearPlane * pow(g_globalData.farPlane / g_globalData.nearPlane, float(groupID.z + 1) / float(g_globalData.numZSlices));

		//clusterAABB.min.z = LinearDepthToNDC(sliceNear, g_globalData.nearPlane, g_globalData.farPlane);
		//clusterAABB.max.z = LinearDepthToNDC(sliceFar, g_globalData.nearPlane, g_globalData.farPlane);

		// 如果超出depthTexture的區間，則直接=0
	}
	GroupMemoryBarrierWithGroupSync();
	if (false)
	{
		// 不會有light需要計算
	}
	else
	{
		// Process lights
		for (uint i = threadIdx; i < g_globalData.pointLightCount; i += numberOfThreadInGroup)
		{
			PointLightData light = g_pointLightData[i];
			
			// 計算有沒有在cluster裡
			float4 lightPosInClipSpace = mul(float4(light.x, light.y, light.z, 1.0), g_globalData.projViewMatrix);
			float4 lightPosInViewSpace = mul(float4(light.x, light.y, light.z, 1.0), g_globalData.viewMatrix);
			float3 ndcCenter = lightPosInClipSpace.xyz / lightPosInClipSpace.w;
			
			
			
			// ---- ndc space radius 近似 ----
			// 用三個偏移方向估計半徑在 clip 空間的投影範圍
			//float3 ndcExtent = 0;

			//{
			//	float4 offsetX = mul(float4(light.x + light.radius, light.y, light.z, 1.0), g_globalData.projViewMatrix);
			//	ndcExtent.x = abs((offsetX.x / offsetX.w) - ndcCenter.x);

			//	float4 offsetY = mul(float4(light.x, light.y + light.radius, light.z, 1.0), g_globalData.projViewMatrix);
			//	ndcExtent.y = abs((offsetY.y / offsetY.w) - ndcCenter.y);

			//	float4 offsetZ = mul(float4(light.x, light.y, light.z + light.radius, 1.0), g_globalData.projViewMatrix);
			//	ndcExtent.z = abs((offsetZ.z / offsetZ.w) - ndcCenter.z);
			//}

			//float3 lightMin = ndcCenter - ndcExtent;
			//float3 lightMax = ndcCenter + ndcExtent;

			//// ---- cluster AABB 已經在 ndc space ----
			//bool insideX = (lightMax.x >= clusterAABB.min.x) && (lightMin.x <= clusterAABB.max.x);
			//bool insideY = (lightMax.y >= clusterAABB.min.y) && (lightMin.y <= clusterAABB.max.y);
			//bool insideZ = (lightMax.z >= clusterAABB.min.z) && (lightMin.z <= clusterAABB.max.z);
		
			
			//if (SphereIntersect(clusterFrustum, lightPosInViewSpace.xyz, light.radius))
			if (SphereAABBIntersect(lightPosInViewSpace.xyz, light.radius, clusterAABB[0], clusterAABB[1]))
			{
				uint localIndex;
				InterlockedAdd(lightCountInCluster, 1, localIndex);
				if (localIndex < MAX_LIGHT_COUNT_IN_LIST)
				{
					lightIndices[localIndex] = i;
				}

			}
		}
	}
	GroupMemoryBarrierWithGroupSync();

	if (threadIdx == 0)
	{
		ClusterRange clusterRange;
		if (lightCountInCluster != 0)
		{
			if (lightCountInCluster > MAX_LIGHT_COUNT_IN_LIST)
				lightCountInCluster = MAX_LIGHT_COUNT_IN_LIST;

			uint offset;
			g_globalCounter.InterlockedAdd(0, lightCountInCluster, offset);
			for (uint i = 0; i < lightCountInCluster; i++)
			{
				g_globalLightIndices[offset + i] = lightIndices[i];
			}
			clusterRange.count = lightCountInCluster;
			clusterRange.offset = offset;
		}
		else
		{
			clusterRange.count = 0;
			clusterRange.offset = 0;
		}
		g_clusterRanges[groupID.z * g_globalData.numYSlices * g_globalData.numXSlices +
		                groupID.y * g_globalData.numXSlices +
		                groupID.x] = clusterRange;
	}
}
