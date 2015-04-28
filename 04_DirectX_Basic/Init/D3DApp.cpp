#include <string>

#include <Windows.h>
#include <d3d11_1.h>

#include "GameTimer.h"
#include "D3DApp.h"


D3DApp::D3DApp(HINSTANCE hInstance)
	:m_hAppInstancce(hInstance),
	 m_hMainWnd(0),
	 m_bAppPaused(false),
	 m_bMinimized(false),
	 m_bMaximized(false),
	 m_Resizing(false),
	 m_4xMsaaQuality(0),
	 m_d3dDevice(0),
	 m_d3dImmediateContext(0),
	 m_d3dDriverType(D3D_DRIVER_TYPE_HARDWARE),
	 m_SwapChain(0),
	 m_DepthStencilBuffer(0),
	 m_RenderTargetView(0),
	 m_DepthStencilView(0),
	 m_MainWndCaption(L"D3D11 Application"),
	 m_ClientWidth(800),
	 m_ClientHeight(600),
	 m_Enable4xMsaa(false)
{
	ZeroMemory(&m_SceenViewport, sizeof(D3D11_VIEWPORT));
}


D3DApp::~D3DApp(void)
{
	
}

bool D3DApp::InitDirect3D()
{
	UINT createDeviceFlages = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlages != D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(
		0,
		m_d3dDriverType,
		0,
		createDeviceFlages,
		0,
		0,
		D3D11_SDK_VERSION,
		&m_d3dDevice,
		&featureLevel,
		&m_d3dImmediateContext);

	if (FAILED(hr))
	{
		MessageBox(0, L"D3D11CreateDevice Failed", 0, 0);
		return false;
	}


}

