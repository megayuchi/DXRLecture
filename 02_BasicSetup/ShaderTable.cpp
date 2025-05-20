#include "pch.h"
#include <Windows.h>
#include <d3d12.h>
#include <d3dx12.h>
#include "../D3D_Util/D3DUtil.h"
#include "ShaderTable_Common.h"
#include "ShaderTable.h"
#include "../Util/WriteDebugString.h"


CShaderTable::CShaderTable()
{

}

BOOL CShaderTable::Initiailze(ID3D12Device5* pD3DDevice, UINT ShaderRecordSize, const WCHAR* wchResourceName)
{
	m_pD3DDevice = pD3DDevice;
	m_ShaderRecordSize = Align(ShaderRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
	wcscpy_s(m_wchResourceName, wchResourceName);

	return TRUE;
}
UINT CShaderTable::CommitResource(UINT MaxShaderRecordNum)
{
	UINT MemSize = MaxShaderRecordNum * m_ShaderRecordSize;

	// free old resource
	if (m_pResource)
	{
		m_pResource->Release();
		m_pResource = nullptr;
	}
	
	CreateUploadBuffer(m_pD3DDevice, nullptr, MemSize, &m_pResource, m_wchResourceName);
	if (!m_pResource)
	{
		__debugbreak();
	}

	// We don't unmap this until the app closes. Keeping buffer mapped for the lifetime of the resource is okay.
	CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
	HRESULT hr = m_pResource->Map(0, &readRange, (void**)&m_pMappedPtr);
	if (FAILED(hr))
		__debugbreak();

	m_CurShaderRecordNum = 0;
	m_pCurWritePtr = m_pMappedPtr;
	m_ShaderRecordSize = m_ShaderRecordSize;
	m_MaxShaderRecordNum = MaxShaderRecordNum;

	return m_MaxShaderRecordNum;
}

BOOL CShaderTable::InsertShaderRecord(const ShaderRecord* pShaderRecord)
{
	if (m_CurShaderRecordNum >= m_MaxShaderRecordNum)
		__debugbreak();

	uint8_t* byteDest = static_cast<uint8_t*>(m_pCurWritePtr);
	memcpy(byteDest, pShaderRecord->shaderIdentifier.ptr, pShaderRecord->shaderIdentifier.size);
	if (pShaderRecord->localRootArguments.ptr)
	{
		memcpy(byteDest + pShaderRecord->shaderIdentifier.size, pShaderRecord->localRootArguments.ptr, pShaderRecord->localRootArguments.size);
	}

	m_pCurWritePtr += m_ShaderRecordSize;
	m_CurShaderRecordNum++;

	return TRUE;
}

void CShaderTable::Cleanup()
{	
	if (m_pResource)
	{
		m_pResource->Release();
		m_pResource = nullptr;
	}
}
CShaderTable::~CShaderTable()
{
	Cleanup();
}