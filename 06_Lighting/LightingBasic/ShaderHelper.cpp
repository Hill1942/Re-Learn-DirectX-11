#include <fstream>
#include <memory>
#include <d3dcompiler.h>
#include "ShaderHelper.h"


ShaderHelper::ShaderHelper(void)
{
}


ShaderHelper::~ShaderHelper(void)
{
}

HRESULT ShaderHelper::LoadCompiledShader(const char* filename, 
										 ID3DBlob** blob)
{
	std::ifstream ifs(filename, std::ios::binary);
	if (ifs.bad() || ifs.fail())
	{
		std::string failmsg = "Failed to load shader from ";
		failmsg.append(filename);
		return S_FALSE;
	}

	ifs.seekg(0, std::ios::end);
	size_t size = static_cast<size_t>(ifs.tellg());
	auto buf = std::unique_ptr<char>(new char[size]);

	ifs.seekg(0, std::ios::beg);
	ifs.read(buf.get(), size);
	ifs.close();

	if (FAILED(D3DCreateBlob(size, blob)))
		return S_FALSE;

	memcpy((*blob)->GetBufferPointer(), buf.get(), size);

	return S_OK;
}
