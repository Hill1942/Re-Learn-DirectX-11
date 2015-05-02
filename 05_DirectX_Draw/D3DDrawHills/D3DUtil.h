#ifndef D3D_UTIL_H
#define D3D_UTIL_H


#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <cassert>

using namespace DirectX;

#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x, source)											    \
	{																\
		HRESULT hr = (x);											\
		if(FAILED(hr))												\
		{															\
			LPWSTR output;                                          \
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |				\
						FORMAT_MESSAGE_IGNORE_INSERTS |				\
						FORMAT_MESSAGE_ALLOCATE_BUFFER,				\
						NULL,										\
						hr,											\
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  \
						(LPTSTR) &output,							\
						0,											\
						NULL);										\
			MessageBox(NULL, output, source, MB_OK);				\
		}															\
	}
#endif
#else
	#ifndef HR
	#define HR(x, source) (x)
	#endif
#endif 

#define ReleaseCOM(x) { if (x) { x->Release(); x = 0;} }


#endif
