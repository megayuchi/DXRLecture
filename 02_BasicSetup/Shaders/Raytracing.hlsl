#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#include "Raytracing_common.hlsl"

[shader("raygeneration")]
void MyRaygenShader_RadianceRay()
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    uint2 launchDim = DispatchRaysDimensions().xy;

    float2 xy = launchIndex.xy;
    float3 color = float3(launchIndex.xy, 1) / float3(launchDim.xy, 1);
	
    g_OutputDiffuse[launchIndex.xy] = float4(color, 1);
    g_OutputDepth[launchIndex.xy] = 0.01;
}
[shader("closesthit")]
void MyClosestHitShader_RadianceRay(inout RadiancePayload rayPayload, in BuiltInTriangleIntersectionAttributes attr)
{
    uint ArrayIndex = 0;

	
	
}
[shader("closesthit")]
void MyClosestHitShader_ShadowRay(inout ShadowPayload rayPayload, in BuiltInTriangleIntersectionAttributes attr)
{
}


[shader("miss")]
void MyMissShader_RadianceRay(inout RadiancePayload rayPayload)
{
    rayPayload.radiance = float3(0, 0, 1);
    rayPayload.depth = 1.2;
}

[shader("anyhit")]
void MyAnyHitShader_RadianceRay(inout RadiancePayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
	
}

#endif // RAYTRACING_HLSL
