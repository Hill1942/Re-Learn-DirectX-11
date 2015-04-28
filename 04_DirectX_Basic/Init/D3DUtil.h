#ifndef D3D_UTIL_H
#define D3D_UTIL_H


#include <d3d11.h>
#include <cassert>


#define ReleaseCOM(x) { if (x) { x->Release(); x = 0;} }

#endif
