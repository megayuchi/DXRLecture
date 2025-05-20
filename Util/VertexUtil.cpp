#include "pch.h"
#include <Windows.h>

DWORD AddVertex(BasicVertex* pVertexList, DWORD dwMaxVertexCount, DWORD* pdwInOutVertexCount, const BasicVertex* pVertex);

#include <DirectXMath.h>
#include <iostream>

using namespace DirectX;

XMVECTOR ComputeNormal(const XMVECTOR& p0, const XMVECTOR& p1, const XMVECTOR& p2)
{
	XMVECTOR v1 = XMVectorSubtract(p1, p0);
	XMVECTOR v2 = XMVectorSubtract(p2, p0);
	XMVECTOR normal = XMVector3Cross(v1, v2);
	normal = XMVector3Normalize(normal);

	return normal;
}
XMVECTOR ComputeTangent(const XMVECTOR& p0, const XMVECTOR& p1, const XMVECTOR& p2, const XMFLOAT2& uv0, const XMFLOAT2& uv1, const XMFLOAT2& uv2)
{
	XMVECTOR edge1 = XMVectorSubtract(p1, p0);
	XMVECTOR edge2 = XMVectorSubtract(p2, p0);

	float du1 = uv1.x - uv0.x;
	float dv1 = uv1.y - uv0.y;
	float du2 = uv2.x - uv0.x;
	float dv2 = uv2.y - uv0.y;

	float f = 1.0f / (du1 * dv2 - du2 * dv1);

	XMVECTOR tangent = XMVectorSet(
		f * (dv2 * XMVectorGetX(edge1) - dv1 * XMVectorGetX(edge2)),
		f * (dv2 * XMVectorGetY(edge1) - dv1 * XMVectorGetY(edge2)),
		f * (dv2 * XMVectorGetZ(edge1) - dv1 * XMVectorGetZ(edge2)),
		0.0f
	);

	return XMVector3Normalize(tangent);
}

DWORD CreateGridPerPlane(BasicVertex* pOutVertexList, DWORD dwMaxVertexBufferCount, WORD* pOutIndexList, DWORD dwMaxIndexBufferCount, const XMFLOAT3* pStart, const XMFLOAT3* pEnd,
						 DWORD dwStartVertexIndex,
						 int iWidth, int iHeight,
						 int u_index, int v_index,
						 const XMFLOAT4* pColor,
						 DWORD* pdwOutIndexCount)
{
	DWORD dwVertexCount = 0;
	DWORD dwIndexCount = 0;

	DWORD dwRequiredVertexCount = (iWidth + 1) * (iHeight + 1);
	DWORD dwRequiredIndexCount = iWidth * iHeight * 2 * 3;

	if (dwMaxVertexBufferCount < dwRequiredVertexCount)
		__debugbreak();

	if (dwMaxIndexBufferCount < dwRequiredIndexCount)
		__debugbreak();

	const float* p_start_u = &pStart->x + u_index;
	const float* p_start_v = &pStart->x + v_index;
	const float* p_end_u = &pEnd->x + u_index;
	const float* p_end_v = &pEnd->x + v_index;

	float pos_offset_u = (*p_end_u - *p_start_u) / (float)iWidth;
	float pos_offset_v = (*p_end_v - *p_start_v) / (float)iHeight;

	int width_vertex_count = iWidth + 1;
	int height_vertex_count = iHeight + 1;
	
	float tex_coord_offset_u = 1.0f / (float)iWidth;
	float tex_coord_offset_v = 1.0f / (float)iHeight;

	for (int v = 0; v < height_vertex_count; v++)
	{
		for (int u = 0; u < width_vertex_count; u++)
		{
			BasicVertex* pDestVertex = pOutVertexList + (u + v * width_vertex_count);
			pDestVertex->position = *pStart;
			pDestVertex->color = *pColor;
			
			float* p_dest_u = &pDestVertex->position.x + u_index;
			float* p_dest_v = &pDestVertex->position.x + v_index;
			*p_dest_u += pos_offset_u * (float)u;
			*p_dest_v += pos_offset_v * (float)v;

			pDestVertex->texCoord.x = tex_coord_offset_u * (float)u;
			pDestVertex->texCoord.y = tex_coord_offset_v * (float)v;
			dwVertexCount++;
		}
	}

	for (int v = 0; v < iHeight; v++)
	{
		for (int u = 0; u < iWidth; u++)
		{
			WORD* pDestIndex = pOutIndexList + ((u * 2 * 3) + v * iWidth * (2 * 3));
			pDestIndex[0] = u + (v * width_vertex_count);
			pDestIndex[1] = (u + 1) + (v * width_vertex_count);
			pDestIndex[2] = (u + 1) + ((v + 1) * width_vertex_count);
			pDestIndex[3] = pDestIndex[0];
			pDestIndex[4] = pDestIndex[2];
			pDestIndex[5] = (u) + ((v + 1) * width_vertex_count);
			dwIndexCount += 6;
		}
	}
	
	XMVECTOR p0 = { pOutVertexList[pOutIndexList[0]].position.x, pOutVertexList[pOutIndexList[0]].position.y, pOutVertexList[pOutIndexList[0]].position.z };
	XMVECTOR p1 = { pOutVertexList[pOutIndexList[1]].position.x, pOutVertexList[pOutIndexList[1]].position.y, pOutVertexList[pOutIndexList[1]].position.z };
	XMVECTOR p2 = { pOutVertexList[pOutIndexList[2]].position.x, pOutVertexList[pOutIndexList[2]].position.y, pOutVertexList[pOutIndexList[2]].position.z };

	XMFLOAT2 uv0 = { pOutVertexList[pOutIndexList[0]].texCoord.x, pOutVertexList[pOutIndexList[0]].texCoord.y };
	XMFLOAT2 uv1 = { pOutVertexList[pOutIndexList[1]].texCoord.x, pOutVertexList[pOutIndexList[1]].texCoord.y };
	XMFLOAT2 uv2 = { pOutVertexList[pOutIndexList[2]].texCoord.x, pOutVertexList[pOutIndexList[2]].texCoord.y };
	
	XMVECTOR normal = ComputeNormal(p0, p1, p2);
	XMVECTOR tangent = ComputeTangent(p0, p1, p2, uv0, uv1, uv2);
	for (DWORD i = 0; i < dwVertexCount; i++)
	{
		pOutVertexList[i].normal = { normal.m128_f32[0], normal.m128_f32[1], normal.m128_f32[2] };
		pOutVertexList[i].tangent = { tangent.m128_f32[0], tangent.m128_f32[1], tangent.m128_f32[2] };
	}

	for (DWORD i = 0; i < dwIndexCount; i++)
	{
		pOutIndexList[i] += dwStartVertexIndex;
	}

	*pdwOutIndexCount = dwIndexCount;
	return dwVertexCount;
}

DWORD CreateGridBox(BasicVertex** ppOutVertexList, WORD** ppOutIndexList, DWORD* pdwOutIndexCount, int iWidth, int iHeight, float fHalfBoxLen)
{
	DWORD dwVertexCount = 0;
	DWORD dwIndexCount = 0;

	XMFLOAT3 pWorldPosList[8];
	pWorldPosList[0] = { -fHalfBoxLen, fHalfBoxLen, fHalfBoxLen };
	pWorldPosList[1] = { -fHalfBoxLen, -fHalfBoxLen, fHalfBoxLen };
	pWorldPosList[2] = { fHalfBoxLen, -fHalfBoxLen, fHalfBoxLen };
	pWorldPosList[3] = { fHalfBoxLen, fHalfBoxLen, fHalfBoxLen };
	pWorldPosList[4] = { -fHalfBoxLen, fHalfBoxLen, -fHalfBoxLen };
	pWorldPosList[5] = { -fHalfBoxLen, -fHalfBoxLen, -fHalfBoxLen };
	pWorldPosList[6] = { fHalfBoxLen, -fHalfBoxLen, -fHalfBoxLen };
	pWorldPosList[7] = { fHalfBoxLen, fHalfBoxLen, -fHalfBoxLen };


	//      0 -- 3
	//    / |  / |
	//  /   1 /- 2
	// 4 -/ 7  /
	// | /  |/
	// 5 -- 6

	// +z
	XMFLOAT3 p_z_start = pWorldPosList[3];
	XMFLOAT3 p_z_end = pWorldPosList[1];
	XMFLOAT4 p_z_color = { 0.0f, 0.0f, 1.0f, 1.0f };
	
	// -z
	XMFLOAT3 n_z_start = pWorldPosList[4];
	XMFLOAT3 n_z_end = pWorldPosList[6];
	XMFLOAT4 n_z_color = { 0.5f, 0.5f, 0.0f, 1.0f };

	// -x
	XMFLOAT3 n_x_start = pWorldPosList[0];
	XMFLOAT3 n_x_end = pWorldPosList[5];
	XMFLOAT4 n_x_color = { 0.0f, 0.5f, 0.5f, 1.0f };

	// +x
	XMFLOAT3 p_x_start = pWorldPosList[7];
	XMFLOAT3 p_x_end = pWorldPosList[2];
	XMFLOAT4 p_x_color = { 1.0f, 0.0f, 0.0f, 1.0f };

	// +y
	XMFLOAT3 p_y_start = pWorldPosList[0];
	XMFLOAT3 p_y_end = pWorldPosList[7];
	XMFLOAT4 p_y_color = { 0.0f, 1.0f, 0.0f, 1.0f };

	// -y
	XMFLOAT3 n_y_start = pWorldPosList[2];
	XMFLOAT3 n_y_end = pWorldPosList[5];
	XMFLOAT4 n_y_color = { 0.5f, 0.0f, 0.5f, 1.0f };
	//
	

	DWORD dwMaxVertexCountPerPlane = (iWidth + 1) * (iHeight + 1);
	DWORD dwMaxVertexCount = dwMaxVertexCountPerPlane * 6;
	BasicVertex* pVertexList = new BasicVertex[dwMaxVertexCount];
	memset(pVertexList, 0, sizeof(BasicVertex) * dwMaxVertexCount);

	DWORD dwMaxIndexCountPerPlane = iWidth * iHeight * 2 * 3;
	DWORD dwMaxIndexCount = dwMaxIndexCountPerPlane * 6;
	WORD* pIndexList = new WORD[dwMaxIndexCount];
	memset(pIndexList, 0, sizeof(WORD) * dwMaxIndexCount);
	
	// -z
	DWORD dwIndexCountPerPlane = 0;
	DWORD dwBasicVertexCountPerPlane = CreateGridPerPlane(pVertexList, dwMaxVertexCount, pIndexList, dwMaxIndexCount, &n_z_start, &n_z_end, dwVertexCount, iWidth, iHeight, 0, 1, &n_z_color, &dwIndexCountPerPlane);
	dwVertexCount += dwBasicVertexCountPerPlane;
	dwIndexCount += dwIndexCountPerPlane;

	// +z
	dwBasicVertexCountPerPlane = CreateGridPerPlane(pVertexList + dwVertexCount, dwMaxVertexCount - dwVertexCount, pIndexList + dwIndexCount, dwMaxIndexCount - dwIndexCount, &p_z_start, &p_z_end, dwVertexCount, iWidth, iHeight, 0, 1, &p_z_color, &dwIndexCountPerPlane);
	dwVertexCount += dwBasicVertexCountPerPlane;
	dwIndexCount += dwIndexCountPerPlane;

	// -x
	dwBasicVertexCountPerPlane = CreateGridPerPlane(pVertexList + dwVertexCount, dwMaxVertexCount - dwVertexCount, pIndexList + dwIndexCount, dwMaxIndexCount - dwIndexCount, &n_x_start, &n_x_end, dwVertexCount, iWidth, iHeight, 2, 1, &n_x_color, &dwIndexCountPerPlane);
	dwVertexCount += dwBasicVertexCountPerPlane;
	dwIndexCount += dwIndexCountPerPlane;

	// +x
	dwBasicVertexCountPerPlane = CreateGridPerPlane(pVertexList + dwVertexCount, dwMaxVertexCount - dwVertexCount, pIndexList + dwIndexCount, dwMaxIndexCount - dwIndexCount, &p_x_start, &p_x_end, dwVertexCount, iWidth, iHeight, 2, 1, &p_x_color, &dwIndexCountPerPlane);
	dwVertexCount += dwBasicVertexCountPerPlane;
	dwIndexCount += dwIndexCountPerPlane;

	// +y
	dwBasicVertexCountPerPlane = CreateGridPerPlane(pVertexList + dwVertexCount, dwMaxVertexCount - dwVertexCount, pIndexList + dwIndexCount, dwMaxIndexCount - dwIndexCount, &p_y_start, &p_y_end, dwVertexCount, iWidth, iHeight, 0, 2, &p_y_color, &dwIndexCountPerPlane);
	dwVertexCount += dwBasicVertexCountPerPlane;
	dwIndexCount += dwIndexCountPerPlane;

	// -y
	dwBasicVertexCountPerPlane = CreateGridPerPlane(pVertexList + dwVertexCount, dwMaxVertexCount - dwVertexCount, pIndexList + dwIndexCount, dwMaxIndexCount - dwIndexCount, &n_y_start, &n_y_end, dwVertexCount, iWidth, iHeight, 0, 2, &n_y_color, &dwIndexCountPerPlane);
	dwVertexCount += dwBasicVertexCountPerPlane;
	dwIndexCount += dwIndexCountPerPlane;

	*ppOutVertexList = pVertexList;
	*pdwOutIndexCount = dwIndexCount;
	*ppOutIndexList = pIndexList;

	return dwVertexCount;
}
void DeleteGridBox(BasicVertex** ppInOutVertexList, WORD** ppInOutIndexList)
{
	BasicVertex* pVertexList = *ppInOutVertexList;
	if (pVertexList)
	{
		delete[] pVertexList;
		*ppInOutVertexList = nullptr;
	}
	
	WORD* pIndexList = *ppInOutIndexList;
	if (pIndexList)
	{
		delete[] pIndexList;
		*ppInOutIndexList = nullptr;
	}
}
DWORD CreateBoxMesh(BasicVertex** ppOutVertexList, WORD* pOutIndexList, DWORD dwMaxBufferCount, float fHalfBoxLen)
{
	const DWORD INDEX_COUNT = 36;
	if (dwMaxBufferCount < INDEX_COUNT)
		__debugbreak();

	const WORD pIndexList[INDEX_COUNT] =
	{
		// +z
		3, 0, 1,
		3, 1, 2,

		// -z
		4, 7, 6,
		4, 6, 5,

		// -x
		0, 4, 5,
		0, 5, 1,

		// +x
		7, 3, 2,
		7, 2, 6,

		// +y
		0, 3, 7,
		0, 7, 4,

		// -y
		2, 1, 5,
		2, 5, 6
	};
	
	TVERTEX pTexCoordList[INDEX_COUNT] =
	{
		// +z
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
		
		// -z
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},

		// -x
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},

		// +x
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},

		// +y
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
		
		// -y
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}
	};
	FLOAT3 pNormalList[INDEX_COUNT] =
	{
		// +z
		{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
		{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
		
		// -z
		{0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},
		{0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},

		// -x
		{-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
		{-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},

		// +x
		{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
		{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},

		// +y
		{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
		{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
		
		// -y
		{0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f},
		{0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}
	};
	FLOAT3 pTangentList[INDEX_COUNT] = {};
	
	FLOAT3 pWorldPosList[8];
	pWorldPosList[0] = { -fHalfBoxLen, fHalfBoxLen, fHalfBoxLen };
	pWorldPosList[1] = { -fHalfBoxLen, -fHalfBoxLen, fHalfBoxLen };
	pWorldPosList[2] = { fHalfBoxLen, -fHalfBoxLen, fHalfBoxLen };
	pWorldPosList[3] = { fHalfBoxLen, fHalfBoxLen, fHalfBoxLen };
	pWorldPosList[4] = { -fHalfBoxLen, fHalfBoxLen, -fHalfBoxLen };
	pWorldPosList[5] = { -fHalfBoxLen, -fHalfBoxLen, -fHalfBoxLen };
	pWorldPosList[6] = { fHalfBoxLen, -fHalfBoxLen, -fHalfBoxLen };
	pWorldPosList[7] = { fHalfBoxLen, fHalfBoxLen, -fHalfBoxLen };

	
	for (DWORD i = 0; i < INDEX_COUNT / 3; i++)
	{
		XMVECTOR p0 = { pWorldPosList[pIndexList[i * 3 + 0]].x, pWorldPosList[pIndexList[i * 3 + 0]].y, pWorldPosList[pIndexList[i * 3 + 0]].z, 1.0f };
		XMVECTOR p1 = { pWorldPosList[pIndexList[i * 3 + 1]].x, pWorldPosList[pIndexList[i * 3 + 1]].y, pWorldPosList[pIndexList[i * 3 + 1]].z, 1.0f };
		XMVECTOR p2 = { pWorldPosList[pIndexList[i * 3 + 2]].x, pWorldPosList[pIndexList[i * 3 + 2]].y, pWorldPosList[pIndexList[i * 3 + 2]].z, 1.0f };

		XMFLOAT2 uv0 = { pTexCoordList[i * 3 + 0].u, pTexCoordList[i * 3 + 0].v };
		XMFLOAT2 uv1 = { pTexCoordList[i * 3 + 1].u, pTexCoordList[i * 3 + 1].v };
		XMFLOAT2 uv2 = { pTexCoordList[i * 3 + 2].u, pTexCoordList[i * 3 + 2].v };

		XMVECTOR tangent = ComputeTangent(p0, p1, p2, uv0, uv1, uv2);

		pTangentList[i * 3 + 0].x = tangent.m128_f32[0];
		pTangentList[i * 3 + 0].y = tangent.m128_f32[1];
		pTangentList[i * 3 + 0].z = tangent.m128_f32[2];

		pTangentList[i * 3 + 1].x = tangent.m128_f32[0];
		pTangentList[i * 3 + 1].y = tangent.m128_f32[1];
		pTangentList[i * 3 + 1].z = tangent.m128_f32[2];

		pTangentList[i * 3 + 2].x = tangent.m128_f32[0];
		pTangentList[i * 3 + 2].y = tangent.m128_f32[1];
		pTangentList[i * 3 + 2].z = tangent.m128_f32[2];
	}

	const DWORD MAX_WORKING_VERTEX_COUNT = 65536;
	BasicVertex* pWorkingVertexList = new BasicVertex[MAX_WORKING_VERTEX_COUNT];
	memset(pWorkingVertexList, 0, sizeof(BasicVertex)*MAX_WORKING_VERTEX_COUNT);
	DWORD dwBasicVertexCount = 0;

	for (DWORD i = 0; i < INDEX_COUNT; i++)
	{
		BasicVertex v;
		v.color = { 1.0f, 1.0f, 1.0f, 1.0f };
		v.position = { pWorldPosList[pIndexList[i]].x, pWorldPosList[pIndexList[i]].y, pWorldPosList[pIndexList[i]].z };
		v.normal = {pNormalList[i].x, pNormalList[i].y, pNormalList[i].z };
		v.tangent = {pTangentList[i].x, pTangentList[i].y, pTangentList[i].z };
		v.texCoord = { pTexCoordList[i].u, pTexCoordList[i].v };

		pOutIndexList[i] = AddVertex(pWorkingVertexList, MAX_WORKING_VERTEX_COUNT, &dwBasicVertexCount, &v);
	}
	BasicVertex* pNewVertexList = new BasicVertex[dwBasicVertexCount];
	memcpy(pNewVertexList, pWorkingVertexList, sizeof(BasicVertex) * dwBasicVertexCount);

	*ppOutVertexList = pNewVertexList;

	delete[] pWorkingVertexList;
	pWorkingVertexList = nullptr;

	return dwBasicVertexCount;
}

DWORD CreateBottomMesh(BasicVertex* pOutVertexList, DWORD dwMaxVertexCount, WORD* pOutIndexList, DWORD dwMaxIndexCount, float fHalfWidthDepth, float fHeight)
{
	if (dwMaxVertexCount < 4)
		__debugbreak();

	if (dwMaxIndexCount < 6)
		__debugbreak();

	// x-z 평면,  시계방향
	
	// z
	// | 0   1
	// | 3   2
	// +------ x
	const WORD pIndexList[6] =
	{
		0, 1, 2,
		0, 2, 3
	};
	
	FLOAT3 pWorldPosList[4];
	pWorldPosList[0] = { -fHalfWidthDepth, fHeight, fHalfWidthDepth };
	pWorldPosList[1] = { fHalfWidthDepth, fHeight, fHalfWidthDepth };
	pWorldPosList[2] = { fHalfWidthDepth, fHeight, -fHalfWidthDepth };
	pWorldPosList[3] = { -fHalfWidthDepth, fHeight, -fHalfWidthDepth };

	TVERTEX pTexCoordList[4] =
	{
		{0.0f, 0.0f}, 
		{4.0f, 0.0f},
		{4.0f, 4.0f},
		{0.0f, 4.0f}
	};

	XMVECTOR p0 = { pWorldPosList[0].x, pWorldPosList[0].y, pWorldPosList[0].z };
	XMVECTOR p1 = { pWorldPosList[1].x, pWorldPosList[1].y, pWorldPosList[1].z };
	XMVECTOR p2 = { pWorldPosList[2].x, pWorldPosList[2].y, pWorldPosList[2].z };
	XMFLOAT2 t0 = { pTexCoordList[0].u,pTexCoordList[0].v };
	XMFLOAT2 t1 = { pTexCoordList[1].u,pTexCoordList[1].v };
	XMFLOAT2 t2 = { pTexCoordList[2].u,pTexCoordList[2].v };

	XMVECTOR tangent = ComputeTangent(p0, p1, p2, t0, t1, t2);
	for (DWORD i = 0; i < 4; i++)
	{
		BasicVertex v;
		v.position = { pWorldPosList[i].x, pWorldPosList[i].y, pWorldPosList[i].z };
		v.texCoord = { pTexCoordList[i].u, pTexCoordList[i].v };
		v.color = { 1.0f, 1.0f, 1.0f, 1.0f };
		v.normal = { 0.0f, 1.0f, 0.0f };
		v.tangent = { tangent.m128_f32[0], tangent.m128_f32[1],tangent.m128_f32[2] };
		
		pOutVertexList[i] = v;
	}
	memcpy(pOutIndexList, pIndexList, sizeof(WORD) * 6);

	return 4;
}

DWORD CreateWallMesh(BasicVertex* pOutVertexList, DWORD dwMaxVertexCount, WORD* pOutIndexList, DWORD dwMaxIndexCount, float fHalfWidthDepth, float fHeight)
{
	if (dwMaxVertexCount < 4)
		__debugbreak();

	if (dwMaxIndexCount < 6)
		__debugbreak();

	// x-y 평면,  시계방향
	
	// y
	// | 0   1
	// | 3   2
	// +------ x
	const WORD pIndexList[6] =
	{
		0, 1, 2,
		0, 2, 3
	};
	
	FLOAT3 pWorldPosList[4];
	pWorldPosList[0] = { -fHalfWidthDepth, fHalfWidthDepth, 0.0f };
	pWorldPosList[1] = { fHalfWidthDepth, fHalfWidthDepth, 0.0f };
	pWorldPosList[2] = { fHalfWidthDepth, -fHalfWidthDepth, 0.0f };
	pWorldPosList[3] = { -fHalfWidthDepth, -fHalfWidthDepth, 0.0f };

	TVERTEX pTexCoordList[4] =
	{
		{0.0f, 0.0f}, 
		{4.0f, 0.0f},
		{4.0f, 4.0f},
		{0.0f, 4.0f}
	};

	XMVECTOR p0 = { pWorldPosList[0].x, pWorldPosList[0].y, pWorldPosList[0].z };
	XMVECTOR p1 = { pWorldPosList[1].x, pWorldPosList[1].y, pWorldPosList[1].z };
	XMVECTOR p2 = { pWorldPosList[2].x, pWorldPosList[2].y, pWorldPosList[2].z };
	XMFLOAT2 t0 = { pTexCoordList[0].u,pTexCoordList[0].v };
	XMFLOAT2 t1 = { pTexCoordList[1].u,pTexCoordList[1].v };
	XMFLOAT2 t2 = { pTexCoordList[2].u,pTexCoordList[2].v };

	XMVECTOR tangent = ComputeTangent(p0, p1, p2, t0, t1, t2);
	for (DWORD i = 0; i < 4; i++)
	{
		BasicVertex v;
		v.position = { pWorldPosList[i].x, pWorldPosList[i].y, pWorldPosList[i].z };
		v.texCoord = { pTexCoordList[i].u, pTexCoordList[i].v };
		v.color = { 1.0f, 1.0f, 1.0f, 1.0f };
		v.normal = { 0.0f, 0.0f, -1.0f };
		v.tangent = { tangent.m128_f32[0], tangent.m128_f32[1],tangent.m128_f32[2] };
		
		pOutVertexList[i] = v;
	}
	memcpy(pOutIndexList, pIndexList, sizeof(WORD) * 6);

	return 4;
}

void DeleteBoxMesh(BasicVertex* pVertexList)
{
	delete[] pVertexList;
}
DWORD AddVertex(BasicVertex* pVertexList, DWORD dwMaxVertexCount, DWORD* pdwInOutVertexCount, const BasicVertex* pVertex)
{
	DWORD dwFoundIndex = -1;
	DWORD dwExistVertexCount = *pdwInOutVertexCount;
	for (DWORD i = 0; i < dwExistVertexCount; i++)
	{
		const BasicVertex* pExistVertex = pVertexList + i;
		if (!memcmp(pExistVertex, pVertex, sizeof(BasicVertex)))
		{
			dwFoundIndex = i;
			goto lb_return;
		}
	}
	if (dwExistVertexCount + 1 > dwMaxVertexCount)
	{
		__debugbreak();
		goto lb_return;
	}
	// 새로운 vertex추가
	dwFoundIndex = dwExistVertexCount;
	pVertexList[dwFoundIndex] = *pVertex;
	*pdwInOutVertexCount = dwExistVertexCount + 1;
lb_return:
	return dwFoundIndex;
}
