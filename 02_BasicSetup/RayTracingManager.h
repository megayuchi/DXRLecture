#pragma once

class CShaderTable;
class CD3D12Renderer;
class CRayTracingManager
{
	enum COMMON_DESCRIPTOR_INDEX
	{
		COMMON_DESCRIPTOR_INDEX_OUTPUT_DIFFUSE_UAV,	// UAV - Output - Diffuse
		COMMON_DESCRIPTOR_INDEX_OUTPUT_DEPTH_UAV,	// UAV - Output - Depth
		COMMON_DESCRIPTOR_COUNT,
	};
	enum DISPATCH_DESCRIPTOR_INDEX
	{
		DISPATCH_DESCRIPTOR_INDEX_RAYTRACING_CBV,
		DISPATCH_DESCRIPTOR_INDEX_OUTPUT_DIFFUSE,
		DISPATCH_DESCRIPTOR_INDEX_OUTPUT_DEPTH,
		DISPATCH_DESCRIPTOR_INDEX_COUNT,
	};
	static const DWORD MAX_RECURSION_DEPTH = 1;
	static const DWORD MAX_RADIANCE_RECURSION_DEPTH = min(MAX_RECURSION_DEPTH, 1);


	CD3D12Renderer* m_pRenderer = nullptr;
	ID3D12Device5*	m_pD3DDevice = nullptr;
	ID3D12CommandQueue*	m_pCommandQueue = nullptr;
	ID3D12CommandAllocator* m_pCommandAllocator = nullptr;
	ID3D12GraphicsCommandList6* m_pCommandList = nullptr;

	HANDLE	m_hFenceEvent = nullptr;
	ID3D12Fence* m_pFence = nullptr;
	UINT64	m_ui64FenceValue = 0;

	ID3D12Resource*	m_pOutputDiffuse = nullptr;	// raytracing output - diffuse
	ID3D12Resource*	m_pOutputDepth = nullptr;	// raytracing output - depth	
	DWORD m_dwWidth = 0;
	DWORD m_dwHeight = 0;

	SHADER_HANDLE*	m_pRayShader = nullptr;
	ID3D12RootSignature*	m_pRaytracingGlobalRootSignature = nullptr;
	ID3D12StateObject*		m_pDXRStateObject = nullptr;

	ID3D12DescriptorHeap*	m_pCommonDescriptorHeap = nullptr;
	ID3D12DescriptorHeap*	m_pShaderVisibleDescriptorHeap = nullptr;	// ID에 따라 srv,uav 위치 고정.
	UINT	m_DescriptorSize = 0;



	CShaderTable* m_pRayGenShaderTable = nullptr;
	UINT m_ShaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;


	void	CreateCommandList();
	void	CleanupCommandList();
	void	CreateFence();
	void	CleanupFence();
	UINT64	Fence();
	void	WaitForFenceValue();

	void	BuildShaderTables();
	void	CleanupShaderTables();

	void	CreateRootSignatures();
	void	CreateRaytracingPipelineStateObject();

	void	CreateDescriptorHeapCBV_SRV_UAV();
	void	CleanupDescriptorHeapForCBV_SRV_UAV();

	void	CreateShaderVisibleHeap();
	void	CleanupDispatchHeap();

	BOOL	CreateOutputDiffuseBuffer(UINT Width, UINT Height);
	void	CleanupOutputDiffuseBuffer();
	BOOL	CreateOutputDepthBuffer(UINT Width, UINT Height);
	void	CleanupOutputDepthBuffer();
	void	Cleanup();


public:
	BOOL	Initialize(CD3D12Renderer* pRenderer, DWORD dwWidth, DWORD dwHeight);

	void	DoRaytracing(ID3D12GraphicsCommandList6* pCommandList);
	void	UpdateWindowSize(DWORD dwWidth, DWORD dwHeight);

	ID3D12Resource*	INL_GetOutputResource() { return m_pOutputDiffuse; }
	ID3D12Resource* INL_GetDepthResource() { return m_pOutputDepth; }

	CRayTracingManager();
	~CRayTracingManager();
};