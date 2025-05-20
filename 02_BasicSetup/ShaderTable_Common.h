#pragma once

struct PointerWithSize
{
	void *ptr;
	UINT size;

	PointerWithSize() : ptr(nullptr), size(0) {}
	PointerWithSize(void* _ptr, UINT _size) : ptr(_ptr), size(_size) {};
};

class ShaderRecord
{
public:
	PointerWithSize shaderIdentifier;
	PointerWithSize localRootArguments;

	ShaderRecord(void* pShaderIdentifier, UINT shaderIdentifierSize) :
		shaderIdentifier(pShaderIdentifier, shaderIdentifierSize)
	{
		int a = 0;
	}

	ShaderRecord(void* pShaderIdentifier, UINT shaderIdentifierSize, void* pLocalRootArguments, UINT localRootArgumentsSize) :
		shaderIdentifier(pShaderIdentifier, shaderIdentifierSize),
		localRootArguments(pLocalRootArguments, localRootArgumentsSize)
	{
		int a = 0;
	}

	void CopyTo(void* dest) const
	{
		uint8_t* byteDest = static_cast<uint8_t*>(dest);
		memcpy(byteDest, shaderIdentifier.ptr, shaderIdentifier.size);
		if (localRootArguments.ptr)
		{
			memcpy(byteDest + shaderIdentifier.size, localRootArguments.ptr, localRootArguments.size);
		}
	}


};