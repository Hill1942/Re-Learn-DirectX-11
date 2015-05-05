#include "D3DTextureDemo.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	D3DTextureDemo theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	return theApp.Run();
}