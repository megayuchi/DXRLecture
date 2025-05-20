#pragma once

struct SHADER_HANDLE;
class CShaderManager;
class CRayTracingManager;
class CD3D12Renderer
{
	HWND	m_hWnd = nullptr;
	ID3D12Device5*	m_pD3DDevice = nullptr;
	ID3D12CommandQueue*	m_pCommandQueue = nullptr;

	ID3D12CommandAllocator* m_pCommandAllocator = nullptr;
	ID3D12GraphicsCommandList6* m_pCommandList = nullptr;
	UINT64	m_ui64FenceValue = 0;
	CShaderManager*	m_pShaderManager = nullptr;
	CRayTracingManager* m_pRayTracingManager = nullptr;
	D3D_FEATURE_LEVEL	m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;
	DXGI_ADAPTER_DESC1	m_AdapterDesc = {};
	IDXGISwapChain3*	m_pSwapChain = nullptr;
	D3D12_VIEWPORT	m_Viewport = {};
	D3D12_RECT		m_ScissorRect = {};
	DWORD			m_dwWidth = 0;
	DWORD			m_dwHeight = 0;

	ID3D12Resource*	m_pRenderTargets[SWAP_CHAIN_FRAME_COUNT] = {};
	ID3D12Resource*	m_pDepthStencil = nullptr;

	ID3D12DescriptorHeap*		m_pRTVHeap = nullptr;
	ID3D12DescriptorHeap*		m_pDSVHeap = nullptr;

	UINT	m_rtvDescriptorSize = 0;
	UINT	m_dsvDescriptorSize = 0;

	UINT	m_dwSwapChainFlags = 0;
	UINT	m_uiRenderTargetIndex = 0;
	HANDLE	m_hFenceEvent = nullptr;
	ID3D12Fence* m_pFence = nullptr;

	DWORD	m_dwCurContextIndex = 0;



	void	CreateFence();
	void	CleanupFence();
	void	CreateCommandList();
	void	CleanupCommandList();

	BOOL	CreateDescriptorHeapForRTV();
	void	CleanupDescriptorHeapForRTV();
	BOOL	CreateDescriptorHeapForDSV();
	void	CleanupDescriptorHeapForDSV();


	BOOL	CreateDepthStencil(UINT Width, UINT Height);
	void	CleanupDepthStencil();

	UINT64	Fence();
	void	WaitForFenceValue();

	void	Cleanup();

public:
	BOOL	Initialize(HWND hWnd, BOOL bEnableDebugLayer, BOOL bEnableGBV, BOOL bDebugShader, const WCHAR* wchShaderPath);
	void	BeginRender();
	void	EndRender();

	void	Present();
	BOOL	UpdateWindowSize(DWORD dwBackBufferWidth, DWORD dwBackBufferHeight);


	// for internal
	ID3D12Device5* INL_GetD3DDevice() const { return m_pD3DDevice; }
	CShaderManager* INL_GetShaderManager() { return m_pShaderManager; }
	CRayTracingManager* INL_GetRayTracingManager() { return m_pRayTracingManager; }

	DWORD GetWidth() const { return m_dwWidth; }
	DWORD GetHeight() const { return m_dwHeight; }
	CD3D12Renderer();
	~CD3D12Renderer();
};

