#ifndef RAYTRACING_COMMON_HLSL
#define RAYTRACING_COMMON_HLSL


#include "Raytracing_typedef.hlsl"

// Global Root Parameter
RWTexture2D<float4> g_OutputDiffuse : register(u0);
RWTexture2D<float4> g_OutputDepth : register(u1);

cbuffer CONSTANT_BUFFER_RAY_TRACING : register(b0)
{
    float g_Near;
    float g_Far;
    uint g_MaxRadianceRayRecursionDepth;
    uint Reserved0;
};



#endif // RAYTRACING_COMMON_HLSL
