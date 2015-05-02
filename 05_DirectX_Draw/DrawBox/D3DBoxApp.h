#pragma once
#include "DirectXMath.h"
#include "ConstantBuffer.h"
#include "d3dapp.h"

struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT4 Color;
};

struct CBPerspectiveObject
{
	DirectX::XMFLOAT4X4 mWorldViewProj;
};

class D3DHillApp: public D3DApp
{
public:
	D3DHillApp(HINSTANCE hInstance);
	~D3DHillApp(void);

	bool Init() override;
	void OnResize() override;
	void UpdateScene(float dt) override;
	void DrawScene() override;

	void OnMouseDown(WPARAM btnState, int x, int y) override;
	void OnMouseUp(WPARAM btnState, int x, int y) override;
	void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	void BuildGemetryBuffers();
	void LoadShader();
	void BuildVertexLayout();
	void BuildRasterState();

private:
	ConstantBuffer<CBPerspectiveObject> mCBObject;
	ID3D11Buffer*          mHillVB;
	ID3D11Buffer*          mBoxIB;
	ID3DBlob*              mPSBlob;
	ID3DBlob*              mVSBlob;
	ID3D11PixelShader*     mPixelShader;
	ID3D11VertexShader*    mVertexShader;
	ID3D11InputLayout*     mInputLayout;
	ID3D11RasterizerState* mRasterState;

	DirectX::XMFLOAT4X4    mWorld;
	DirectX::XMFLOAT4X4    mView;
	DirectX::XMFLOAT4X4    mProj;

	float                  mTheta;
	float                  mPhi;
	float                  mRadius;

	POINT                  mLastMousePos;
};

