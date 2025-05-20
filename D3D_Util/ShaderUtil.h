#pragma once

BOOL CreateShaderCodeFromFile(BYTE** ppOutCodeBuffer, DWORD* pdwOutCodeSize, SYSTEMTIME* pOutLastWriteTime, const WCHAR* wchFileName);
void DeleteShaderCode(BYTE* pCodeBuffer);
HRESULT CompileShaderFromFileWithDXC(IDxcLibrary* pLibrary, IDxcCompiler* pCompiler, IDxcIncludeHandler* pIncludeHandler, const WCHAR* wchFileName, const WCHAR* wchEntryPoint, const WCHAR* wchShaderModel, IDxcBlob** ppOutCodeBlob, BOOL bDisableOptimize, SYSTEMTIME* pOutLastWriteTime, DWORD dwFlags);