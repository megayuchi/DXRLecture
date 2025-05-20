#pragma once

#include <DirectXMath.h>
#include "../Util/LinkedList.h"

using namespace DirectX;

struct BasicVertex
{
	XMFLOAT3 position;
	XMFLOAT4 color;
};

union RGBA
{
	struct
	{
		BYTE	r;
		BYTE	g;
		BYTE	b;
		BYTE	a;
	};
	BYTE		bColorFactor[4];
};

struct TVERTEX
{
	float u;
	float v;
};
struct FLOAT3
{
	float x;
	float y;
	float z;
};

#define DEFULAT_LOCALE_NAME		L"ko-kr"

static const float NEAR_PLANE = 0.01f;
static const float FAR_PLANE = 800.0f;

inline float XMMatrixExtract(const XMMATRIX* pMatrix, int row, int col)
{
	// 1 base
	float* p = (float*)pMatrix + ((row - 1) * 4) + (col - 1);
	return *p;
}