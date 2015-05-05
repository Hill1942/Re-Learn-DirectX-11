#include "D3DTextureBox.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	D3DTextureBox theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	return theApp.Run();
}