#include "pch.h"
#include <d3d12.h>
#include <d3dx12.h>
#include <dxcapi.h>
#include "../D3D_Util/ShaderUtil.h"
#include "../Util/WriteDebugString.h"
#include "D3D12Renderer.h"
#include "ShaderManager.h"

typedef DXC_API_IMPORT HRESULT (__stdcall *DxcCreateInstanceT)(_In_ REFCLSID rclsid, _In_ REFIID riid, _Out_ LPVOID* ppv);

CShaderManager::CShaderManager()
{
}

BOOL CShaderManager::Initialize(CD3D12Renderer* pRenderer, const WCHAR* wchShaderPath, BOOL bDisableOptimize)
{
	BOOL bResult = FALSE;

	m_bDisableOptimize = bDisableOptimize;

	wcscpy_s(m_wchDefaultShaderPath, wchShaderPath);

	if (!InitDXC())
	{
		__debugbreak();
		goto lb_return;
	}
	bResult = TRUE;
lb_return:
	return bResult;

}
BOOL CShaderManager::InitDXC()
{
	BOOL bResult = FALSE;
	const WCHAR*	wchDllPath = nullptr;
#if defined(_M_ARM64EC)
	wchDllPath = L"./Dxc/arm64";
#elif defined(_M_ARM64)
	wchDllPath = L"./Dxc/arm64";
#elif defined(_M_AMD64)
	wchDllPath = L"./Dxc/x64";
#elif defined(_M_IX86)
	wchDllPath = L"./Dxc/x86";	
#endif
	WCHAR wchOldPath[_MAX_PATH];
	GetCurrentDirectoryW(_MAX_PATH, wchOldPath);
	SetCurrentDirectoryW(wchDllPath);

	m_hDXL = LoadLibrary(L"dxcompiler.dll");
	if (!m_hDXL)
		goto lb_return;

	DxcCreateInstanceT	DxcCreateInstanceFunc = (DxcCreateInstanceT)GetProcAddress(m_hDXL, "DxcCreateInstance");

	HRESULT hr = DxcCreateInstanceFunc(CLSID_DxcLibrary, IID_PPV_ARGS(&m_pLibrary));
	if (FAILED(hr))
		__debugbreak();

	hr = DxcCreateInstanceFunc(CLSID_DxcCompiler, IID_PPV_ARGS(&m_pCompiler));
	if (FAILED(hr))
		__debugbreak();

	m_pLibrary->CreateIncludeHandler(&m_pIncludeHandler);
	bResult = TRUE;

lb_return:
	SetCurrentDirectoryW(wchOldPath);
	return bResult;
}
SHADER_HANDLE* CShaderManager::CreateShaderDXC(const WCHAR* wchShaderFileName, const WCHAR* wchEntryPoint, const WCHAR* wchShaderModel, DWORD dwFlags)
{
	BOOL				bResult = FALSE;

	SYSTEMTIME	CreationTime = {};
	SHADER_HANDLE* pNewShaderHandle = nullptr;

	WCHAR wchOldPath[MAX_PATH];
	GetCurrentDirectory(_MAX_PATH, wchOldPath);

	IDxcBlob*	pBlob = nullptr;

	//	case DXIL::ShaderKind::Vertex:    entry = L"VSMain"; profile = L"vs_6_1"; break;
	// case DXIL::ShaderKind::Pixel:     entry = L"PSMain"; profile = L"ps_6_1"; break;
	// case DXIL::ShaderKind::Geometry:  entry = L"GSMain"; profile = L"gs_6_1"; break;
	// case DXIL::ShaderKind::Hull:      entry = L"HSMain"; profile = L"hs_6_1"; break;
	// case DXIL::ShaderKind::Domain:    entry = L"DSMain"; profile = L"ds_6_1"; break;
	// case DXIL::ShaderKind::Compute:   entry = L"CSMain"; profile = L"cs_6_1"; break;
	// case DXIL::ShaderKind::Mesh:      entry = L"MSMain"; profile = L"ms_6_5"; break;
	// case DXIL::ShaderKind::Amplification: entry = L"ASMain"; profile = L"as_6_5"; break;

	//"vs_6_0"
	//"ps_6_0"
	//"cs_6_0"
	//"gs_6_0"
	//"ms_6_5"
	//"as_6_5"
	//"hs_6_0"
	//"lib_6_3"

	
	SetCurrentDirectory(m_wchDefaultShaderPath);
	HRESULT	hr = CompileShaderFromFileWithDXC(m_pLibrary, m_pCompiler, m_pIncludeHandler, wchShaderFileName, wchEntryPoint, wchShaderModel, &pBlob, m_bDisableOptimize, &CreationTime, 0);
	if (FAILED(hr))
	{
		WriteDebugStringW(DEBUG_OUTPUT_TYPE_DEBUG_CONSOLE, L"Failed to compile shader : %s-%s\n", wchShaderFileName, wchEntryPoint);
		goto lb_exit;
	}
	DWORD	dwCodeSize = (DWORD)pBlob->GetBufferSize();
	const char*	pCodeBuffer = (const char*)pBlob->GetBufferPointer();

	DWORD	ShaderHandleSize = sizeof(SHADER_HANDLE) - sizeof(DWORD) + dwCodeSize;
	pNewShaderHandle = (SHADER_HANDLE*)malloc(ShaderHandleSize);
	memset(pNewShaderHandle, 0, ShaderHandleSize);

	memcpy(pNewShaderHandle->pCodeBuffer, pCodeBuffer, dwCodeSize);
	pNewShaderHandle->dwCodeSize = dwCodeSize;
	pNewShaderHandle->dwShaderNameLen = swprintf_s(pNewShaderHandle->wchShaderName, L"%s-%s", wchShaderFileName, wchEntryPoint);
	bResult = TRUE;


lb_exit:
	if (pBlob)
	{
		pBlob->Release();
		pBlob = nullptr;
	}
	SetCurrentDirectory(wchOldPath);

	return pNewShaderHandle;
}






void CShaderManager::ReleaseShader(SHADER_HANDLE* pShaderHandle)
{
	free(pShaderHandle);
}

void CShaderManager::Cleanup()
{
	CleanupDXC();
}
void CShaderManager::CleanupDXC()
{
	if (m_pIncludeHandler)
	{
		m_pIncludeHandler->Release();
		m_pIncludeHandler = nullptr;
	}
	if (m_pCompiler)
	{
		m_pCompiler->Release();
		m_pCompiler = nullptr;
	}
	if (m_pLibrary)
	{
		m_pLibrary->Release();
		m_pLibrary = nullptr;
	}
	if (m_hDXL)
	{
		FreeLibrary(m_hDXL);
		m_hDXL = nullptr;
	}
}
CShaderManager::~CShaderManager()
{
	Cleanup();
}
