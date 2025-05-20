#pragma once

#include "ShaderTable_Common.h"

// Shader record = {{Shader ID}, {RootArguments}}
// Shader table = {{ ShaderRecord 1}, {ShaderRecord 2}, ...}

class CD3DResourceRecycleBin;
class CShaderTable
{
	ID3D12Device5* m_pD3DDevice = nullptr;
	ID3D12Resource*	m_pResource = nullptr;
	uint8_t* m_pMappedPtr = nullptr;
	uint8_t* m_pCurWritePtr = nullptr;
	UINT m_ShaderRecordSize = 0;
	UINT m_MaxShaderRecordNum = 0;
	UINT m_CurShaderRecordNum = 0;
	WCHAR	m_wchResourceName[128] = {};

	void	Cleanup();
public:
	BOOL	Initiailze(ID3D12Device5* pD3DDevice, UINT ShaderRecordSize, const WCHAR* wchResourceName);
	UINT	CommitResource(UINT MaxShaderRecordNum);
	BOOL	InsertShaderRecord(const ShaderRecord* pShaderRecord);

	ID3D12Resource*	GetResource() const { return m_pResource; }
	UINT GetShaderRecordSize() const { return m_ShaderRecordSize; }
	UINT GetShaderRecordNum() const { return m_CurShaderRecordNum; }
	UINT GetMaxShaderRecordNum() const { return m_MaxShaderRecordNum; }
	UINT GetHitGroupShaderTableSize() const { return (m_ShaderRecordSize * m_CurShaderRecordNum); }
	CShaderTable();
	~CShaderTable();
};


