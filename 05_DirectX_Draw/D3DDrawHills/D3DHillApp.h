#pragma once
#include "DirectXMath.h"
#include "ConstantBuffer.h"
#include "d3dapp.h"

struct CBPerspectiveObject
{
	XMFLOAT4X4 mWorldViewProj;
};

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;

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
	float GetHeight(float x, float z) const;

private:
	ConstantBuffer<CBPerspectiveObject> mCBObject;
	ID3D11Buffer*          mHillVB;
	ID3D11Buffer*          mHillIB;
	ID3DBlob*              mPSBlob;
	ID3DBlob*              mVSBlob;
	ID3D11PixelShader*     mPixelShader;
	ID3D11VertexShader*    mVertexShader;
	ID3D11InputLayout*     mInputLayout;
	ID3D11RasterizerState* mRasterState;

	XMFLOAT4X4    mWorld;
	XMFLOAT4X4    mView;
	XMFLOAT4X4    mProj;

	int                    mGridIndexCount;
	float                  mTheta;
	float                  mPhi;
	float                  mRadius;

	POINT                  mLastMousePos;
};

