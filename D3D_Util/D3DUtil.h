#pragma once

void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);
void GetSoftwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);
void SetDebugLayerInfo(ID3D12Device* pD3DDevice);
HRESULT CreateVertexBuffer(ID3D12Device* pDevice, UINT SizePerVertex, DWORD dwVertexNum, D3D12_VERTEX_BUFFER_VIEW* pOutVertexBufferView, ID3D12Resource **ppOutBuffer);
HRESULT CreateUAVBuffer(ID3D12Device* pDevice, UINT64 BufferSize, ID3D12Resource** ppResource, D3D12_RESOURCE_STATES initialResourceState, const WCHAR* wchResourceName);
HRESULT CreateUploadBuffer(ID3D12Device* pDevice, void *pData, UINT64 DataSize, ID3D12Resource** ppResource, const WCHAR* wchResourceName);
void SetDefaultSamplerDesc(D3D12_STATIC_SAMPLER_DESC* pOutSamperDesc, UINT RegisterIndex);
void SetSamplerDesc_Wrap(D3D12_STATIC_SAMPLER_DESC* pOutSamperDesc, UINT RegisterIndex);
void SetSamplerDesc_Clamp(D3D12_STATIC_SAMPLER_DESC* pOutSamperDesc, UINT RegisterIndex);
void SetSamplerDesc_Border(D3D12_STATIC_SAMPLER_DESC* pOutSamperDesc, UINT RegisterIndex);
void SetSamplerDesc_Mirror(D3D12_STATIC_SAMPLER_DESC* pOutSamperDesc, UINT RegisterIndex);
void SerializeAndCreateRaytracingRootSignature(ID3D12Device* pDevice, D3D12_ROOT_SIGNATURE_DESC* pDesc, ID3D12RootSignature** ppOutRootSig);
void UpdateTexture(ID3D12Device* pD3DDevice, ID3D12GraphicsCommandList6* pCommandList, ID3D12Resource* pDestTexResource, ID3D12Resource* pSrcTexResource);
inline size_t AlignConstantBufferSize(size_t size)
{
	size_t aligned_size = (size + 255) & (~255);
	return aligned_size;
}

inline UINT Align(UINT size, UINT alignment)
{
	return (size + (alignment - 1)) & ~(alignment - 1);
}

inline UINT CalculateConstantBufferByteSize(UINT byteSize)
{
	// Constant buffer size is required to be aligned.
	return Align(byteSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
}

inline UINT CeilDivide(UINT value, UINT divisor)
{
	return (value + divisor - 1) / divisor;
}

inline UINT CeilLogWithBase(UINT value, UINT base)
{
	return static_cast<UINT>(ceil(log(value) / log(base)));
}
#define SizeOfInUint32(obj) ((sizeof(obj) - 1) / sizeof(UINT32) + 1)