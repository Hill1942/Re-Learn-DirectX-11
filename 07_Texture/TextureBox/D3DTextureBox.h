#pragma once
#include "DirectXMath.h"
#include "ConstantBuffer.h"
#include "LightHelper.h"
#include "d3dapp.h"

using namespace DirectX;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Texture;
};

struct CBObjectVS
{
	XMFLOAT4X4 gWorld;
	XMFLOAT4X4 gWorldInvTranspose;
	XMFLOAT4X4 gWorldViewProj;
	XMFLOAT4X4 gTexTransform;
};

struct CBObjectPS
{
	Material         gMaterial;
	DirectionalLight gDirLight;
	PointLight       gPointLight;
	SpotLight        gSpotLight;
	XMFLOAT3         gEyePosW;
};

class D3DTextureBox: public D3DApp
{
public:
	D3DTextureBox(HINSTANCE hInstance);
	~D3DTextureBox(void);

	bool Init() override;
	void OnResize() override;
	void UpdateScene(float dt) override;
	void DrawScene() override;

	void OnMouseDown(WPARAM btnState, int x, int y) override;
	void OnMouseUp(WPARAM btnState, int x, int y) override;
	void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	void BuildBoxGemetryBuffers();
	void LoadShader();
	void BuildVertexLayout();
	void BuildRasterState();

private:
	ConstantBuffer<CBObjectVS> mCBObjectVS;
	ConstantBuffer<CBObjectPS> mCBObjectPS;

	ID3D11Buffer*              mBoxVB;
	ID3D11Buffer*              mBoxIB;
						       
	ID3DBlob*                  mPSBlob;
	ID3DBlob*                  mVSBlob;
	ID3D11PixelShader*         mPixelShader;
	ID3D11VertexShader*        mVertexShader;
	ID3D11InputLayout*         mInputLayout;
	ID3D11RasterizerState*     mRasterState;
	ID3D11ShaderResourceView*  mDiffuseMapSRV;

	DirectionalLight       mDirectLight;
	PointLight             mPointLight;
	SpotLight              mSpotLight;
	Material               mBoxMaterial;

	XMFLOAT4X4             mTexTransform;
	XMFLOAT4X4             mBoxWorld;
	XMFLOAT4X4             mView;
	XMFLOAT4X4             mProj;
	XMFLOAT3               mEyePosW;

	float                  mTheta;
	float                  mPhi;
	float                  mRadius;
	int                    mBoxIndexCount;

	POINT                  mLastMousePos;
};

