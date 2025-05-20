#ifndef RAYTRACING_CONST_BUFFERL_HLSL
#define RAYTRACING_CONST_BUFFERL_HLSL

#define INT_MIN     (-2147483647 - 1)
#define INT_MAX       2147483647
#define EPSILON 1e-10
#define DWORD uint
#define UINT uint

#define HitDistanceOnMiss 0

static const uint g_IndexSizeInBytes = 2;
static const uint g_IndicesPerTriangle = 3;
static const uint g_TriangleIndexStride = g_IndicesPerTriangle * g_IndexSizeInBytes;


struct BasicVertex
{
    float3 Pos;
    float4 color;
};
struct Ray
{
    float3 origin;
    float3 direction;
};

struct RadiancePayload
{
    float3 radiance; // TODO encode
    float depth;
    uint rayRecursionDepth;
};

struct ShadowPayload
{
    float tHit; // Hit time <0,..> on Hit. -1 on miss.
};
static const float NEAR_PLANE = 0.01;
static const float FAR_PLANE = 800.0;

//typedef BuiltInTriangleIntersectionAttributes MyAttributes;

#endif