#pragma once
#include "DirectXMath.h"
#include "ConstantBuffer.h"
#include "LightHelper.h"
#include "Waves.h"
#include "d3dapp.h"

using namespace DirectX;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
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
	XMFLOAT4         gFogColor;
	float            gFogStart;
	float            gFogRange;
};

class D3DBlendDemo: public D3DApp
{
public:
	D3DBlendDemo(HINSTANCE hInstance);
	~D3DBlendDemo(void);

	bool Init() override;
	void OnResize() override;
	void UpdateScene(float dt) override;
	void DrawScene() override;

	void OnMouseDown(WPARAM btnState, int x, int y) override;
	void OnMouseUp(WPARAM btnState, int x, int y) override;
	void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	void BuildLandGemetryBuffers();
	void BuildWaveGemetryBuffers();
	void BuildBoxGemetryBuffers();
	void LoadShader();
	void BuildVertexLayout();
	void BuildRasterState();

	void DrawLand();
	void DrawWaves();
	void DrawBox();

	float    GetHillHeight(float x, float z) const;
	XMFLOAT3 GetHillNormal(float x, float z) const;
private:
	ConstantBuffer<CBObjectVS> mCBObjectVS;
	ConstantBuffer<CBObjectPS> mCBObjectPS;

	ID3D11Buffer*             mLandVB;
	ID3D11Buffer*             mLandIB;
	ID3D11Buffer*             mWaveVB;
	ID3D11Buffer*             mWaveIB;
	ID3D11Buffer*             mBoxVB;
	ID3D11Buffer*             mBoxIB;
						      
	ID3DBlob*                 mPSBlob;
	ID3DBlob*                 mVSBlob;
	ID3D11PixelShader*        mPixelShader;
	ID3D11VertexShader*       mVertexShader;
	ID3D11InputLayout*        mInputLayout;
	ID3D11RasterizerState*    mRasterState;
	ID3D11ShaderResourceView* mGrassMapSRV;
	ID3D11ShaderResourceView* mWaterMapSRV;
	ID3D11ShaderResourceView* mBoxMapSRV;

	Waves                     mWaves;
	DirectionalLight          mDirectLight;
	PointLight                mPointLight;
	SpotLight                 mSpotLight;
	Material                  mLandMaterial;
	Material                  mWaveMaterial;
	Material                  mBoxMaterial;
		
	XMFLOAT4X4                mGrassTexTransform;
	XMFLOAT4X4                mWaterTexTransform;
	XMFLOAT4X4                mLandWorld;
	XMFLOAT4X4                mWaveWorld;
	XMFLOAT4X4                mBoxWorld;
	XMFLOAT4X4                mView;
	XMFLOAT4X4                mProj;
	XMFLOAT3                  mEyePosW;
	XMFLOAT2                  mWaterTexOffset;
						      
	float                     mTheta;
	float                     mPhi;
	float                     mRadius;
	int                       mLandIndexCount;
						      
	POINT                     mLastMousePos;
};

