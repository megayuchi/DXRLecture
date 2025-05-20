#include "pch.h"
#include <d3d12.h>
#include <d3dx12.h>
#include "typedef.h"
#include "../D3D_Util/D3DUtil.h"
#include "ShaderManager.h"
#include "D3D12Renderer.h"
#include "ShaderTable.h"
#include "RayTracingManager.h"

const wchar_t* c_raygenShaderName = { L"MyRaygenShader_RadianceRay" };

CRayTracingManager::CRayTracingManager()
{
}
BOOL CRayTracingManager::Initialize(CD3D12Renderer* pRenderer, DWORD dwWidth, DWORD dwHeight)
{
	m_pRenderer = pRenderer;
	m_pD3DDevice = pRenderer->INL_GetD3DDevice();
	CShaderManager* pShaderManager = pRenderer->INL_GetShaderManager();


	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	if (FAILED(m_pD3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue))))
	{
		__debugbreak();
	}

	CreateCommandList();

	// Create synchronization objects.
	CreateFence();

	m_dwWidth = dwWidth;
	m_dwHeight = dwHeight;

	CreateDescriptorHeapCBV_SRV_UAV();
	CreateShaderVisibleHeap();

	m_pRayShader = pShaderManager->CreateShaderDXC(L"Raytracing.hlsl", L"", L"lib_6_3", 0);

	CreateOutputDiffuseBuffer(m_dwWidth, m_dwHeight);
	CreateOutputDepthBuffer(m_dwWidth, m_dwHeight);

	CreateRootSignatures();
	CreateRaytracingPipelineStateObject();

	BuildShaderTables();
	// build geometry

	// build acceleration structure

	return TRUE;
}


void CRayTracingManager::CreateRaytracingPipelineStateObject()
{
	// 총 7개의 Subobject를 생성하여 RTPSO(Ray Tracing Pipeline State Object)를 구성
	// Subobject는 각각의 DXIL export(즉, 쉐이더 엔트리 포인트)에 기본 또는 명시적 방식으로 연결됨

	// 구성:
	// 1 - DXIL(DirectX Intermediate Language) library
	// 1 - Triangle hit group
	// 1 - Shader config (payload, attribute 크기)
	// 2 - Local root signature and association
	// 1 - Global root signature
	// 1 - Pipeline config (재귀 깊이 등)
	CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };


	// 1) DXIL 라이브러리 Subobject 생성
	// 셰이더는 서브오브젝트로 간주되지 않으므로 DXIL 라이브러리를 통해서 전달되어야 한다.
	// DXIL library
	// This contains the shaders and their entrypoints for the state object.
	// Since shaders are not considered a subobject, they need to be passed in via DXIL library subobjects.
	CD3DX12_DXIL_LIBRARY_SUBOBJECT* pLib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();

	// Shader Bytecode 설정 (컴파일된 DXIL)
	D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE(m_pRayShader->pCodeBuffer, m_pRayShader->dwCodeSize);
	pLib->SetDXILLibrary(&libdxil);

	//
	// DXIL 라이브러리에서 사용할 쉐이더 export들을 정의
	//
	pLib->DefineExport(c_raygenShaderName);

	
	
	
	// 2) Triangle hit group
	// 히트 그룹 Subobject 생성
	// 히트 그룹은 Geometry에 레이가 교차했을 때 실행할 ClosestHit, AnyHit, Intersection 쉐이더를 정의
	// 이 예제에서는 hit group을 사용하지 않는다.
	// 
	
	// 3) Shader config
	// Defines the maximum sizes in bytes for the ray payload and attribute structure.
	// Payload와 Attribute 구조의 최대 크기를 설정
	CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT* pShaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
	UINT payloadSize = PAYLOAD_SIZE;
	UINT attributeSize = 2 * sizeof(float); // float2 barycentrics
	pShaderConfig->Config(payloadSize, attributeSize);

	// 4,5) Local root signature and shader association
	// Local Root Signature 및 연결 설정 (명시적 연결 사용)
	// Shader Table에서 각 쉐이더가 고유한 인자를 받을 수 있도록 해줌
	//
	// Raytracing Pipeline State Object에 Local Root Signature 서브오브젝트를 추가
	//
	// 이 샘플에서는 필요하지 않다. 생략한다.
	//
	//

	
	// 6) Global root signature
	// Global Root Signature Subobject 생성
	// DispatchRays() 호출 중 모든 쉐이더가 볼 수 있는 루트 시그니처
	CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT* pGlobalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
	pGlobalRootSignature->SetRootSignature(m_pRaytracingGlobalRootSignature);
	
	// 7) Pipeline config
	// TraceRay() 함수의 최대 재귀 깊이를 설정
	CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT* pPipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
	// PERFOMANCE TIP: Set max recursion depth as low as needed 
	// as drivers may apply optimization strategies for low recursion depths. 
	UINT maxRecursionDepth = MAX_RECURSION_DEPTH; // ~ primary rays only. 
	pPipelineConfig->Config(maxRecursionDepth);


	// Create the state object.
	const D3D12_STATE_OBJECT_DESC* pRaytracingPipeline = raytracingPipeline;
	if (FAILED(m_pD3DDevice->CreateStateObject(pRaytracingPipeline, IID_PPV_ARGS(&m_pDXRStateObject))))
	{
		__debugbreak();
	}
	m_pDXRStateObject->SetName(L"CRayTracingManager::m_pDXRStateObject");
}

void CRayTracingManager::BuildShaderTables()
{
	// Get shader identifiers.
	ID3D12StateObjectProperties* pStateObjectProperties = nullptr;
	m_pDXRStateObject->QueryInterface(IID_PPV_ARGS(&pStateObjectProperties));

	void* pRayGenShaderIdentifier = pStateObjectProperties->GetShaderIdentifier(c_raygenShaderName);

	// raygen shader table
	ShaderRecord rayGenShaderRecord = ShaderRecord(pRayGenShaderIdentifier, m_ShaderIdentifierSize, nullptr, 0);
	m_pRayGenShaderTable = new CShaderTable;
	m_pRayGenShaderTable->Initiailze(m_pD3DDevice, m_ShaderIdentifierSize, L"RayGenShaderTable");
	m_pRayGenShaderTable->CommitResource(1);
	m_pRayGenShaderTable->InsertShaderRecord(&rayGenShaderRecord);

	// Miss shader table

	// hitgroup Shader Table

	if (pStateObjectProperties)
	{
		pStateObjectProperties->Release();
		pStateObjectProperties = nullptr;
	}
}
void CRayTracingManager::CleanupShaderTables()
{
	if (m_pRayGenShaderTable)
	{
		delete m_pRayGenShaderTable;
		m_pRayGenShaderTable = nullptr;
	}
}
void CRayTracingManager::CreateDescriptorHeapCBV_SRV_UAV()
{
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = COMMON_DESCRIPTOR_COUNT;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	if (FAILED(m_pD3DDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_pCommonDescriptorHeap))))
		__debugbreak();

	m_pCommonDescriptorHeap->SetName(L"CD3D12Renderer::m_pCommonDescriptorHeap");

	m_DescriptorSize = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}
void CRayTracingManager::CleanupDescriptorHeapForCBV_SRV_UAV()
{
	if (m_pCommonDescriptorHeap)
	{
		m_pCommonDescriptorHeap->Release();
		m_pCommonDescriptorHeap = nullptr;
	}
}
void CRayTracingManager::CreateShaderVisibleHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
	HeapDesc.NumDescriptors = DISPATCH_DESCRIPTOR_INDEX_COUNT;
	HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	if (FAILED(m_pD3DDevice->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&m_pShaderVisibleDescriptorHeap))))
	{
		__debugbreak();
	}
}
void CRayTracingManager::CleanupDispatchHeap()
{
	if (m_pShaderVisibleDescriptorHeap)
	{
		m_pShaderVisibleDescriptorHeap->Release();
		m_pShaderVisibleDescriptorHeap = nullptr;
	}
}
void CRayTracingManager::UpdateWindowSize(DWORD dwWidth, DWORD dwHeight)
{
	CleanupOutputDiffuseBuffer();
	CleanupOutputDepthBuffer();

	m_dwWidth = dwWidth;
	m_dwHeight = dwHeight;
	CreateOutputDiffuseBuffer(m_dwWidth, m_dwHeight);
	CreateOutputDepthBuffer(m_dwWidth, m_dwHeight);
}
void CRayTracingManager::DoRaytracing(ID3D12GraphicsCommandList6* pCommandList)
{

}

void CRayTracingManager::CreateRootSignatures()
{
	// Global Root Signature
	// This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.

	// root param 0
	// output-diffuse(uav) | output-depth(uav)

	// root param 1
	// Acceleration Sturecture

	CD3DX12_DESCRIPTOR_RANGE globalRanges[2];
	globalRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);	// b0 : CBV

	// u0 : u0-diffuse | u1 : out-depth
	globalRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 2, 0);

	// b0 : RaytracingCBV | u0 : u0-diffuse | u1 : out-depth | t0 : AccelerationStructure
	CD3DX12_ROOT_PARAMETER GlobalRootParameters[1];
	GlobalRootParameters[0].InitAsDescriptorTable(_countof(globalRanges), globalRanges, D3D12_SHADER_VISIBILITY_ALL);

	// 샘플러
	D3D12_STATIC_SAMPLER_DESC samplers[4] = {};
	SetSamplerDesc_Wrap(samplers + 0, 0);	// Wrap Linear
	SetSamplerDesc_Clamp(samplers + 1, 1);	// Clamp Linear
	SetSamplerDesc_Wrap(samplers + 2, 2);	// Wrap Point
	samplers[2].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	SetSamplerDesc_Mirror(samplers + 3, 3);	// Mirror Linear
	samplers[3].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;

	for (DWORD i = 0; i < (DWORD)_countof(samplers); i++)
	{
		samplers[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}

	CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(GlobalRootParameters), GlobalRootParameters, (DWORD)_countof(samplers), samplers);
	SerializeAndCreateRaytracingRootSignature(m_pD3DDevice, &globalRootSignatureDesc, &m_pRaytracingGlobalRootSignature);
}

BOOL CRayTracingManager::CreateOutputDiffuseBuffer(UINT Width, UINT Height)
{
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Width = Width;
	texDesc.Height = Height;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	texDesc.DepthOrArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	if (FAILED(m_pD3DDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&m_pOutputDiffuse))))
	{
		__debugbreak();
	}
	m_pOutputDiffuse->SetName(L"CRayTracingManager::m_pOutputDiffuse");

	// Create UAV

	CD3DX12_CPU_DESCRIPTOR_HANDLE	uavHandle(m_pCommonDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), COMMON_DESCRIPTOR_INDEX_OUTPUT_DIFFUSE_UAV, m_DescriptorSize);
	m_pD3DDevice->CreateUnorderedAccessView(m_pOutputDiffuse, nullptr, nullptr, uavHandle);

	return TRUE;
}
void CRayTracingManager::CleanupOutputDiffuseBuffer()
{
	if (m_pOutputDiffuse)
	{
		m_pOutputDiffuse->Release();
		m_pOutputDiffuse = nullptr;
	}
}

BOOL CRayTracingManager::CreateOutputDepthBuffer(UINT Width, UINT Height)
{

	// Create Output Buffer, Texture, SRV
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texDesc.Width = Width;
	texDesc.Height = Height;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	//texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	texDesc.DepthOrArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;


	if (FAILED(m_pD3DDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&m_pOutputDepth))))
	{
		__debugbreak();
	}
	m_pOutputDepth->SetName(L"CRayTracingManager::m_pOutputDepth");

	// Create UAV
	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	UAVDesc.Buffer.StructureByteStride = sizeof(float);
	UAVDesc.Buffer.NumElements = Width * Height;
	UAVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	CD3DX12_CPU_DESCRIPTOR_HANDLE	uavHandle(m_pCommonDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), COMMON_DESCRIPTOR_INDEX_OUTPUT_DEPTH_UAV, m_DescriptorSize);
	m_pD3DDevice->CreateUnorderedAccessView(m_pOutputDepth, nullptr, &UAVDesc, uavHandle);

	return TRUE;
}
void CRayTracingManager::CleanupOutputDepthBuffer()
{
	if (m_pOutputDepth)
	{
		m_pOutputDepth->Release();
		m_pOutputDepth = nullptr;
	}
}


UINT64 CRayTracingManager::Fence()
{
	m_ui64FenceValue++;
	m_pCommandQueue->Signal(m_pFence, m_ui64FenceValue);
	return m_ui64FenceValue;
}
void CRayTracingManager::WaitForFenceValue()
{
	const UINT64 ExpectedFenceValue = m_ui64FenceValue;

	// Wait until the previous frame is finished.
	if (m_pFence->GetCompletedValue() < ExpectedFenceValue)
	{
		m_pFence->SetEventOnCompletion(ExpectedFenceValue, m_hFenceEvent);
		WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}
void CRayTracingManager::CreateCommandList()
{
	if (FAILED(m_pD3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator))))
	{
		__debugbreak();
	}

	// Create the command list.
	if (FAILED(m_pD3DDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator, nullptr, IID_PPV_ARGS(&m_pCommandList))))
	{
		__debugbreak();
	}

	// Command lists are created in the recording state, but there is nothing
	// to record yet. The main loop expects it to be closed, so close it now.
	m_pCommandList->Close();
}
void CRayTracingManager::CleanupCommandList()
{
	if (m_pCommandList)
	{
		m_pCommandList->Release();
		m_pCommandList = nullptr;
	}
	if (m_pCommandAllocator)
	{
		m_pCommandAllocator->Release();
		m_pCommandAllocator = nullptr;
	}
}


void CRayTracingManager::CreateFence()
{
	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	if (FAILED(m_pD3DDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence))))
	{
		__debugbreak();
	}

	m_ui64FenceValue = 0;

	// Create an event handle to use for frame synchronization.
	m_hFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}
void CRayTracingManager::CleanupFence()
{
	if (m_hFenceEvent)
	{
		CloseHandle(m_hFenceEvent);
		m_hFenceEvent = nullptr;
	}
	if (m_pFence)
	{
		m_pFence->Release();
		m_pFence = nullptr;
	}
}

void CRayTracingManager::Cleanup()
{
	CShaderManager* pShaderManager = m_pRenderer->INL_GetShaderManager();

	if (m_pCommandQueue)
	{
		m_pCommandQueue->Release();
		m_pCommandQueue = nullptr;
	}
	CleanupCommandList();
	CleanupFence();

	CleanupOutputDiffuseBuffer();
	CleanupOutputDepthBuffer();

	CleanupShaderTables();


	// Cleanup DXRStateObject
	if (m_pDXRStateObject)
	{
		m_pDXRStateObject->Release();
		m_pDXRStateObject = nullptr;
	}
	// Cleanup RootSignature
	if (m_pRaytracingGlobalRootSignature)
	{
		m_pRaytracingGlobalRootSignature->Release();
		m_pRaytracingGlobalRootSignature = nullptr;
	}
	if (m_pRayShader)
	{
		pShaderManager->ReleaseShader(m_pRayShader);
		m_pRayShader = nullptr;
	}
	CleanupDescriptorHeapForCBV_SRV_UAV();
	CleanupDispatchHeap();
}
CRayTracingManager::~CRayTracingManager()
{
	Cleanup();
}