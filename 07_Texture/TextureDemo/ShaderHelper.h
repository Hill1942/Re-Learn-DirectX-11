#pragma once

#include "d3d11_1.h"


class ShaderHelper
{
public:
	ShaderHelper(void);
	~ShaderHelper(void);

	static HRESULT LoadCompiledShader(const char* filename, ID3DBlob** blod);
};

