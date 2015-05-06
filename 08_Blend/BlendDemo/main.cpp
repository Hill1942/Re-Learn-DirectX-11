#include "D3DBlendDemo.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	D3DBlendDemo theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	return theApp.Run();
}