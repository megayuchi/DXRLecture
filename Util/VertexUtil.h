#pragma once


DWORD CreateBoxMesh(BasicVertex** ppOutVertexList, WORD* pOutIndexList, DWORD dwMaxBufferCount, float fHalfBoxLen);	// alloc, must free()
void DeleteBoxMesh(BasicVertex* pVertexList);	// free

DWORD CreateBottomMesh(BasicVertex* pOutVertexList, DWORD dwMaxVertexCount, WORD* pOutIndexList, DWORD dwMaxIndexCount, float fHalfWidthDepth, float fHeight); // not needed free()
DWORD CreateWallMesh(BasicVertex* pOutVertexList, DWORD dwMaxVertexCount, WORD* pOutIndexList, DWORD dwMaxIndexCount, float fHalfWidthDepth, float fHeight); // not needed free()

DWORD CreateGridPerPlane(BasicVertex* pOutVertexList, DWORD dwMaxVertexBufferCount, WORD* pOutIndexList, DWORD dwMaxIndexBufferCount, const XMFLOAT3* pStart, const XMFLOAT3* pEnd,
						 DWORD dwStartVertexIndex,
						 int iWidth, int iHeight,
						 int u_index, int v_index,
						 const XMFLOAT4* pColor,
						 DWORD* pdwOutIndexCount);

DWORD CreateGridBox(BasicVertex** ppOutVertexList, WORD** ppOutIndexList, DWORD* pdwOutIndexCount, int iWidth, int iHeight, float fHalfBoxLen);
void DeleteGridBox(BasicVertex** ppInOutVertexList, WORD** ppInOutIndexList);