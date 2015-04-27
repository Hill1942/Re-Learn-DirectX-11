#pragma once
class D3DApp
{
public:
	D3DApp(HINSTANCE hInstance);
	~D3DApp(void);

	HINSTANCE AppInstance() const;
	HWND MainWnd() const;
	float AspectRadio() const;

	int Run();

	virtual bool Init();
	virtual void OnResize();
	virtual void UpdateScene(float dt) = 0;
	virtual void DrawScene();
	virtual LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	virtual void OnMouseDown(WPARAM btnState, int x, int y);
	virtual void OnMouseUp(WPARAM btnState, int x, int y);
	virtual void OnMouseMove(WPARAM btnState, int x, int y);

protected:
	bool InitMainWindow();
	bool InitDirect3D();
	void CalculateFrameStats();

protected:
	HINSTANCE m_hAppInstancce;
	HWND      m_hMainWnd;
	bool      m_bAppPaused;
	bool      m_bMinimized;
	bool      m_bMaximized;
	bool      m_Resizing;
	UINT      m_4xMsaaQuality;

	GameTimer m_Timer;

	ID3D11Device*           m_d3dDevice;
	ID3D11DeviceContext*    m_d3dImmediateContext;
	D3D_DRIVER_TYPE         m_d3dDriverType;
	IDXGISwapChain*         m_SwapChain;
	ID3D11Texture2D*        m_DepthStencilBuffer;
	ID3D11RenderTargetView* m_RenderTargetView;
	ID3D11DepthStencilView* m_DepthStencilView;
	D3D11_VIEWPORT          m_SceenViewport;
	

	std::wstring m_MainWndCaption;
	int  m_ClientWidth;
	int  m_ClientHeight;
	bool m_Enable4xMsaa;

};

