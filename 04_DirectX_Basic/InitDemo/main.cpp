#include <sstream>
#include <windowsx.h>
#include "D3DApp.h"
#include "D3DAppInitDemo.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevHinstance, PSTR cmdLine, int showCmd)
{
	D3DAppInitDemo app(hInstance);
	if (!app.Init())
		return 0;

	return app.Run();
}