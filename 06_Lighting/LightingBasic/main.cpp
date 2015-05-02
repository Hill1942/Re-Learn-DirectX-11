#include "D3DLightingBasic.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	D3DLightingBasic theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	return theApp.Run();
}