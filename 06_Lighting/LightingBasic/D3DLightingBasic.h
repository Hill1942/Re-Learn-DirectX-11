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
};

struct CBObject
{
	XMFLOAT4X4 gWorld;
	XMFLOAT4X4 gWorldInvTranspose;
	XMFLOAT4X4 gWorldViewProj;
};

struct CBLightingObject
{
	Material         gMaterial;
	DirectionalLight gDirLight;
	PointLight       gPointLight;
	SpotLight        gSpotLight;
	XMFLOAT3         gEyePosW;
};

class D3DLightingBasic: public D3DApp
{
public:
	D3DLightingBasic(HINSTANCE hInstance);
	~D3DLightingBasic(void);

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
	void LoadShader();
	void BuildVertexLayout();
	void BuildRasterState();

	float    GetHillHeight(float x, float z) const;
	XMFLOAT3 GetHillNormal(float x, float z) const;
private:
	ConstantBuffer<CBObject> mCBObject;
	ConstantBuffer<CBLightingObject> mCBLightingObject;

	ID3D11Buffer*          mLandVB;
	ID3D11Buffer*          mLandIB;
	ID3D11Buffer*          mWaveVB;
	ID3D11Buffer*          mWaveIB;

	ID3DBlob*              mPSBlob;
	ID3DBlob*              mVSBlob;
	ID3D11PixelShader*     mPixelShader;
	ID3D11VertexShader*    mVertexShader;
	ID3D11InputLayout*     mInputLayout;
	ID3D11RasterizerState* mRasterState;

	Waves                  mWaves;
	DirectionalLight       mDirectLight;
	PointLight             mPointLight;
	SpotLight              mSpotLight;
	Material               mLandMaterial;
	Material               mWaveMaterial;

	XMFLOAT4X4             mLandWorld;
	XMFLOAT4X4             mWaveWorld;
	XMFLOAT4X4             mView;
	XMFLOAT4X4             mProj;
	XMFLOAT3               mEyePosW;

	float                  mTheta;
	float                  mPhi;
	float                  mRadius;
	int                    mLandIndexCount;

	POINT                  mLastMousePos;
};

