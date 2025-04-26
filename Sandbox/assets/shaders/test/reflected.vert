#version 450

struct BasicVertexData
{
    vec3 position;
    vec3 normal;
    vec2 uv;
};

struct BoneVertexData
{
    ivec4 boneIndices;
    vec4 weights;
};

struct BoneTransformData
{
    mat4 transform;
};

layout(binding = 1, std430) readonly buffer basicVertexBuffer
{
    BasicVertexData vertexData[];
} in_basicVertexBuffer;

layout(binding = 0, std140) uniform globalBuffer
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
} globalData;

layout(binding = 0, std140) uniform instanceInfo
{
    mat4 transformationMatrix;
    int meshType;
} in_instanceInfo;

layout(binding = 2, std430) readonly buffer boneVertexBuffer
{
    BoneVertexData vertexBoneData[];
} in_boneVertexBuffer;

layout(binding = 3, std430) readonly buffer boneTransformBuffer
{
    BoneTransformData boneTransform[];
} in_boneTransformBuffer;

layout(location = 0) out vec2 vs_texCoord;

void main()
{
    vec3 in_position = in_basicVertexBuffer.vertexData[gl_VertexID].position;
    vec3 in_normal = in_basicVertexBuffer.vertexData[gl_VertexID].normal;
    vec2 in_texCoord = in_basicVertexBuffer.vertexData[gl_VertexID].uv;
    gl_Position = ((globalData.projectionMatrix * globalData.viewMatrix) * in_instanceInfo.transformationMatrix) * vec4(in_position, 1.0);
    vs_texCoord = in_texCoord;
}

