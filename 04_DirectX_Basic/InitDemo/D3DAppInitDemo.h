#pragma once
class D3DAppInitDemo: public D3DApp
{
public:
	D3DAppInitDemo(HINSTANCE hInstance);
	~D3DAppInitDemo(void);

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();
};

