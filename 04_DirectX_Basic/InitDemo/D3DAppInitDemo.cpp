#include <sstream>
#include "windowsx.h"
#include "D3DApp.h"
#include "D3DAppInitDemo.h"


D3DAppInitDemo::D3DAppInitDemo(HINSTANCE hInstance)
	:D3DApp(hInstance)
{
}


D3DAppInitDemo::~D3DAppInitDemo(void)
{
}

bool D3DAppInitDemo::Init()
{
	if (!D3DApp::Init())
		return false;

	return true;
}

void D3DAppInitDemo::OnResize()
{
	D3DApp::OnResize();
}

void D3DAppInitDemo::UpdateScene(float dt)
{
	
}

void D3DAppInitDemo::DrawScene()
{
	assert(m_d3dImmediateContext);
	assert(m_SwapChain);

	m_d3dImmediateContext->ClearRenderTargetView(
		m_RenderTargetView, 
		reinterpret_cast<const float*>(&DirectX::Colors::Blue));
	m_d3dImmediateContext->ClearDepthStencilView(
		m_DepthStencilView, 
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f, 
		0);

	HR(m_SwapChain->Present(0, 0), L"Present");
}
 
