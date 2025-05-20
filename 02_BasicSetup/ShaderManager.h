#pragma once

#define		MAX_SHADER_NAME_BUFFER_LEN		256
#define		MAX_SHADER_NAME_LEN				(MAX_SHADER_NAME_BUFFER_LEN-1)
#define		MAX_SHADER_NUM					2048
#define		MAX_CODE_SIZE					(1024*1024)

struct SHADER_HANDLE
{
	DWORD	dwFlags;
	DWORD	dwCodeSize;
	DWORD	dwShaderNameLen;
	WCHAR	wchShaderName[MAX_SHADER_NAME_BUFFER_LEN];
	DWORD	pCodeBuffer[1];
};

class CD3D12Renderer;
class CShaderManager
{
	HMODULE					m_hDXL = nullptr;
	IDxcLibrary*			m_pLibrary = nullptr;
	IDxcCompiler*			m_pCompiler = nullptr;
	IDxcIncludeHandler*		m_pIncludeHandler = nullptr;
	BOOL	m_bDisableOptimize = FALSE;
	WCHAR	m_wchDefaultShaderPath[_MAX_PATH] = {};

	BOOL	InitDXC();
	void	CleanupDXC();
	void	Cleanup();
public:
	BOOL			Initialize(CD3D12Renderer* pRenderer, const WCHAR* wchShaderPath, BOOL bDisableOptimize);
	SHADER_HANDLE*	CreateShaderDXC(const WCHAR* wchShaderFileName, const WCHAR* wchEntryPoint, const WCHAR* wchShaderModel, DWORD dwFlags);
	void			ReleaseShader(SHADER_HANDLE* pShaderHandle);

	CShaderManager();
	~CShaderManager();
};
