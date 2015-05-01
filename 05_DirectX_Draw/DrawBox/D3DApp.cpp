#include <string>
#include <sstream>
#include <vector>
#include <Windowsx.h>
#include <d3d11_1.h>

#include "GameTimer.h"
#include "D3DApp.h"

D3DApp* g_d3dApp = 0;

LRESULT CALLBACK MainWinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return g_d3dApp->MsgProc(hwnd, msg, wParam, lParam);
}

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
	ZeroMemory(&m_ScreenViewport, sizeof(D3D11_VIEWPORT));
	g_d3dApp = this;
}

D3DApp::~D3DApp(void)
{
	ReleaseCOM(m_RenderTargetView);
	ReleaseCOM(m_DepthStencilView);
	ReleaseCOM(m_SwapChain);
	ReleaseCOM(m_DepthStencilBuffer);

	if (m_d3dImmediateContext)
		m_d3dImmediateContext->ClearState();

	ReleaseCOM(m_d3dImmediateContext);
	ReleaseCOM(m_d3dDevice);
}

bool D3DApp::Init()
{
	if (!InitMainWindow())
		return false;
	if (!InitDirect3D())
		return false;
	return true;
}

int D3DApp::Run()
{
	MSG msg = {0};
	m_Timer.Reset();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			m_Timer.Tick();
			if (!m_bAppPaused)
			{
				CalculateFrameStats();
				UpdateScene(m_Timer.DeltaTime());
				DrawScene();
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}

bool D3DApp::InitDirect3D()
{
	UINT createDeviceFlages = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlages |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	IDXGIFactory * pFactory;
	HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory));

	UINT i = 0; 
	IDXGIAdapter * pAdapter; 
	std::vector <IDXGIAdapter*> vAdapters; 
	while(pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND) 
	{ 
		vAdapters.push_back(pAdapter); 
		++i; 
	}

	const D3D_FEATURE_LEVEL feature_levels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};
	D3D_FEATURE_LEVEL max_feature_level;
	unsigned level_count = sizeof(feature_levels) / sizeof(feature_levels[0]);
	for (int k = 0; k < vAdapters.size(); k++)
	{
		HR(D3D11CreateDevice(
			vAdapters[k],
			D3D_DRIVER_TYPE_UNKNOWN,
			0,
			createDeviceFlages,
			feature_levels,
			level_count,
			D3D11_SDK_VERSION,
			&m_d3dDevice,
			&max_feature_level,
			&m_d3dImmediateContext),
			L"CreateD3DDevice");

		if (max_feature_level == D3D_FEATURE_LEVEL_11_0 || 
			max_feature_level == D3D_FEATURE_LEVEL_11_1)
			break;

		if (k == vAdapters.size() - 1)
		{
			MessageBox(0, L"Your system does not support DirextX11", 0, 0);
			ReleaseCOM(m_d3dDevice);
			ReleaseCOM(m_d3dImmediateContext);
			return false;
		}

		ReleaseCOM(m_d3dDevice);
		ReleaseCOM(m_d3dImmediateContext);
	}

	if (FAILED(m_d3dDevice->CheckMultisampleQualityLevels(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		4,
		&m_4xMsaaQuality)))
	{
		MessageBox(0, L"Check MultisampleQuality Failed", 0, 0);
		return false;
	}

	assert(m_4xMsaaQuality > 0);

	DXGI_SWAP_CHAIN_DESC swap_chain_desc;
	swap_chain_desc.BufferDesc.Width = m_ClientWidth;
	swap_chain_desc.BufferDesc.Height = m_ClientHeight;
	swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
	swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	if (m_Enable4xMsaa)
	{
		swap_chain_desc.SampleDesc.Count = 4;
		swap_chain_desc.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	else
	{
		swap_chain_desc.SampleDesc.Count = 1;
		swap_chain_desc.SampleDesc.Quality = 0;
	}
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount = 1;
	swap_chain_desc.OutputWindow = m_hMainWnd;
	swap_chain_desc.Windowed    = true;
	swap_chain_desc.SwapEffect  = DXGI_SWAP_EFFECT_DISCARD;
	swap_chain_desc.Flags = 0;

	IDXGIDevice* dxgiDevice;
	if (FAILED(m_d3dDevice->QueryInterface(
		__uuidof(IDXGIDevice), 
		(void**)&dxgiDevice)))
	{
		MessageBox(0, L"Query IDXGIDevice failed", 0, 0);
		return false;
	}

	IDXGIAdapter* dxgiAdapter;
	if (FAILED(dxgiDevice->GetParent(
		__uuidof(IDXGIAdapter), 
		(void**)&dxgiAdapter)))
	{
		MessageBox(0, L"Get IDXGIAdapter failed", 0, 0);
		return false;
	}

	IDXGIFactory* dxgiFactory;
	if (FAILED(dxgiAdapter->GetParent(
		__uuidof(IDXGIFactory), 
		(void**)&dxgiFactory)))
	{
		MessageBox(0, L"Get IDXGIFactory failed", 0, 0);
		return false;
	}

	if (FAILED(dxgiFactory->CreateSwapChain(
		m_d3dDevice, 
		&swap_chain_desc,
		&m_SwapChain)))
	{
		MessageBox(0, L"Create Swap Chain failed", 0, 0);
		return false;
	}

	ReleaseCOM(dxgiDevice);
	ReleaseCOM(dxgiAdapter);
	ReleaseCOM(dxgiFactory);

	OnResize();

	return true;
}

bool D3DApp::InitMainWindow()
{
	WNDCLASS wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = MainWinProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = m_hAppInstancce;
	wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULLREGION);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = L"D3DWindow";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"Register Class Failed", 0, 0);
		return false;
	}

	RECT rect = {0, 0, m_ClientWidth, m_ClientHeight};
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	m_hMainWnd = CreateWindow(
		L"D3DWindow",
		m_MainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width, 
		height,
		NULL,
		NULL,
		m_hAppInstancce,
		NULL);

	if (!m_hMainWnd)
	{
		MessageBox(0, L"Create Window Failed", 0, 0);
		return false;
	}

	ShowWindow(m_hMainWnd, SW_SHOW);
	UpdateWindow(m_hMainWnd);

	return true;
}

void D3DApp::OnResize()
{
	assert(m_d3dImmediateContext);
	assert(m_d3dDevice);
	assert(m_SwapChain);

	ReleaseCOM(m_RenderTargetView);
	ReleaseCOM(m_DepthStencilView);
	ReleaseCOM(m_DepthStencilBuffer);

	if (FAILED(m_SwapChain->ResizeBuffers(
		1,
		m_ClientWidth,
		m_ClientHeight,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0)))
	{
		MessageBox(0, L"Resize buffer failed", 0, 0);
		return;
	}

	ID3D11Texture2D* backBuffer;
	if (FAILED(m_SwapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(void**)&backBuffer)))
	{
		MessageBox(0, L"Get Buffer failed when resizing", 0, 0);
		return;
	}

	if (FAILED(m_d3dDevice->CreateRenderTargetView(
		backBuffer, 
		0, 
		&m_RenderTargetView)))
	{
		MessageBox(0, L"Create render target view failed when resizing", 0, 0);
		return;
	}

	ReleaseCOM(backBuffer);

	D3D11_TEXTURE2D_DESC depth_stenil_desc;
	depth_stenil_desc.Width     = m_ClientWidth;
	depth_stenil_desc.Height    = m_ClientHeight;
	depth_stenil_desc.MipLevels = 1;
	depth_stenil_desc.ArraySize = 1;
	depth_stenil_desc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
	if (m_Enable4xMsaa)
	{
		depth_stenil_desc.SampleDesc.Count = 4;
		depth_stenil_desc.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	else
	{
		depth_stenil_desc.SampleDesc.Count = 1;
		depth_stenil_desc.SampleDesc.Quality = 0;
	}
	depth_stenil_desc.Usage          = D3D11_USAGE_DEFAULT;
	depth_stenil_desc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
	depth_stenil_desc.CPUAccessFlags = 0;
	depth_stenil_desc.MiscFlags      = 0;

	if (FAILED(m_d3dDevice->CreateTexture2D(
		&depth_stenil_desc, 
		0, 
		&m_DepthStencilBuffer)))
	{
		MessageBox(0, L"Create depth stencil buffer failed when resizing", 0, 0);
		return;
	}

	if (FAILED(m_d3dDevice->CreateDepthStencilView(
		m_DepthStencilBuffer, 
		0, 
		&m_DepthStencilView)))
	{
		MessageBox(0, L"Create depth stencil view failed when resizing", 0, 0);
		return;
	}

	m_d3dImmediateContext->OMSetRenderTargets(
		1, &m_RenderTargetView, m_DepthStencilView);

	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width    = (float)m_ClientWidth;
	m_ScreenViewport.Height   = (float)m_ClientHeight;
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;

	m_d3dImmediateContext->RSSetViewports(1, &m_ScreenViewport);
}

LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_bAppPaused = true;
			m_Timer.Stop();
		}
		else
		{
			m_bAppPaused = false;
			m_Timer.Start();
		}
		return 0;

	case WM_SIZE:
		m_ClientWidth = LOWORD(lParam);
		m_ClientHeight = HIWORD(lParam);
		if (m_d3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				m_bAppPaused = true;
				m_bMinimized = true;
				m_bMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				m_bAppPaused = false;
				m_bMinimized = false;
				m_bMaximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{
				if (m_bMinimized)
				{
					m_bAppPaused = false;
					m_bMinimized = false;
					OnResize();
				}
				else if (m_bMaximized)
				{
					m_bAppPaused = false;
					m_bMaximized = false;
					OnResize();
				}
				else if (m_Resizing)
				{
					
				}
				else
				{
					OnResize();
				}
			}
		}
		
		return 0;

	case WM_ENTERSIZEMOVE:
		m_bAppPaused = true;
		m_Resizing = true;
		m_Timer.Stop();
		return 0;

	case WM_EXITSIZEMOVE:
		m_bAppPaused = false;
		m_Resizing   = false;
		m_Timer.Start();
		OnResize();
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

HINSTANCE D3DApp::AppInstance() const
{
	return m_hAppInstancce;
}

HWND D3DApp::MainWnd() const
{
	return m_hMainWnd;
}

float D3DApp::AspectRadio() const
{
	return (float)(m_ClientWidth / m_ClientHeight);
}

void D3DApp::CalculateFrameStats()
{
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt;
		float mspf = 1000.0f / fps;

		std::wostringstream outs;
		outs.precision(6);
		outs << m_MainWndCaption << L"    "
			 << L"FPS: "  << fps << L"    "
			 << L"Frame Time: " << mspf << L" (ms)";
		SetWindowText(m_hMainWnd, outs.str().c_str());

		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

void D3DApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	
}
void D3DApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	
}
void D3DApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	
}




