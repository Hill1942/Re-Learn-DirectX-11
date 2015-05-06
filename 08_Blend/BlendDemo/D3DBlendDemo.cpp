#include <DDSTextureLoader.h>
#include "MathHelper.h"
#include "ShaderHelper.h"
#include "ConstantBuffer.h"
#include "GeometryGenerator.h"
#include "RenderStates.h"
#include "D3DBlendDemo.h"


D3DBlendDemo::D3DBlendDemo(HINSTANCE hInstance)
	:D3DApp(hInstance),
	 mLandIB(0),
	 mLandVB(0),
	 mWaveIB(0),
	 mWaveVB(0),
	 mBoxIB(0),
	 mBoxVB(0),
	 mGrassMapSRV(0),
	 mWaterMapSRV(0),
	 mBoxMapSRV(0),
	 mWaterTexOffset(0.0f, 0.0f),
	 mInputLayout(0),
	 mEyePosW(0.0f, 0.0f, 0.0f),
	 mTheta(1.3f * MathHelper::PI),
	 mPhi(0.4f * MathHelper::PI),
	 mRadius(80.0f)
{
	m_MainWndCaption = L"D3DBlendDemo";

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mLandWorld, I);
	XMStoreFloat4x4(&mWaveWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	XMMATRIX boxScale = XMMatrixScaling(15.0f, 15.0f, 15.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(8.0f, 5.0f, -15.0f);
	XMStoreFloat4x4(&mBoxWorld, boxScale*boxOffset);

	XMMATRIX grassTexScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);
	XMStoreFloat4x4(&mGrassTexTransform, grassTexScale);

	// Directional light.
	mDirectLight.Ambient   = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	mDirectLight.Diffuse   = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirectLight.Specular  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirectLight.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
 
	// Point light--position is changed every frame to animate in UpdateScene function.
	mPointLight.Ambient    = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mPointLight.Diffuse    = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mPointLight.Specular   = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mPointLight.Att        = XMFLOAT3(0.0f, 0.1f, 0.0f);
	mPointLight.Range      = 25.0f;

	// Spot light--position and direction changed every frame to animate in UpdateScene function.
	mSpotLight.Ambient     = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mSpotLight.Diffuse     = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	mSpotLight.Specular    = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mSpotLight.Att         = XMFLOAT3(1.0f, 0.0f, 0.0f);
	mSpotLight.Spot        = 96.0f;
	mSpotLight.Range       = 10000.0f;

	mLandMaterial.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mLandMaterial.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mLandMaterial.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	mWaveMaterial.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mWaveMaterial.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	mWaveMaterial.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);

	mBoxMaterial.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mBoxMaterial.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMaterial.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
}


D3DBlendDemo::~D3DBlendDemo(void)
{
	ReleaseCOM(mLandVB);
	ReleaseCOM(mLandIB);
	ReleaseCOM(mWaveVB);
	ReleaseCOM(mWaveIB);
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mPSBlob);
	ReleaseCOM(mVSBlob);
	ReleaseCOM(mGrassMapSRV);
	ReleaseCOM(mWaterMapSRV);
	ReleaseCOM(mBoxMapSRV);
	RenderStates::DestroyAll();
}

bool D3DBlendDemo::Init()
{
	if (!D3DApp::Init())
		return false;
	HR(CreateDDSTextureFromFile(
		m_d3dDevice,
		L"Texture/grass.dds",
		0,
		&mGrassMapSRV,
		0,
		nullptr),
		L"Create Grass Texture");
	HR(CreateDDSTextureFromFile(
		m_d3dDevice,
		L"Texture/water2.dds",
		0,
		&mWaterMapSRV,
		0,
		nullptr),
		L"Create Water Texture");
	HR(CreateDDSTextureFromFile(
		m_d3dDevice,
		L"Texture/WoodCrate02.dds",
		0,
		&mBoxMapSRV,
		0,
		nullptr),
		L"Create Box Texture");
	mWaves.Init(160, 160, 1.0f, 0.03f, 3.25f, 0.4f);
	BuildLandGemetryBuffers();
	BuildWaveGemetryBuffers();
	BuildBoxGemetryBuffers();
	LoadShader();
	BuildVertexLayout();
	BuildRasterState();
	mCBObjectVS.Initilize(m_d3dDevice);
	mCBObjectPS.Initilize(m_d3dDevice);
	RenderStates::InitAll(m_d3dDevice);

	return true;
}

void D3DBlendDemo::OnResize()
{
	D3DApp::OnResize();
	XMMATRIX p = XMMatrixPerspectiveFovLH(
		0.25f * 3.14,
		AspectRadio(), 
		1.0f, 
		1000.0f);

	XMStoreFloat4x4(&mProj, p);
}

void D3DBlendDemo::UpdateScene(float dt)
{
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	mEyePosW = XMFLOAT3(x, y, z);

	XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view      = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);

	static float t_base = 0.0f;
	if ((m_Timer.TotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;
		DWORD i = 5 + rand() % (mWaves.RowCount()    - 10);
		DWORD j = 5 + rand() % (mWaves.ColumnCount() - 10);
		float r = MathHelper::RandF(1.0f, 2.0f);
		mWaves.Disturb(i, j, r);
	}
	mWaves.Update(dt);

	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_d3dImmediateContext->Map(
		mWaveVB, 
		0, 
		D3D11_MAP_WRITE_DISCARD, 
		0, 
		&mappedData),
		L"Context Map");

	Vertex* v = reinterpret_cast<Vertex*>(mappedData.pData);
	for(UINT i = 0; i < mWaves.VertexCount(); ++i)
	{
		v[i].Pos    = mWaves[i];
		v[i].Normal = mWaves.Normal(i);
		v[i].Tex.x  = 0.5f + mWaves[i].x / mWaves.Width();
		v[i].Tex.y  = 0.5f - mWaves[i].z / mWaves.Depth();
	}

	m_d3dImmediateContext->Unmap(mWaveVB, 0);

	//
	// Animate the lights.
	//

	// Circle light over the land surface.
	mPointLight.Position.x = 70.0f*cosf( 0.2f*m_Timer.TotalTime() );
	mPointLight.Position.z = 70.0f*sinf( 0.2f*m_Timer.TotalTime() );
	mPointLight.Position.y = MathHelper::Max(
		GetHillHeight(mPointLight.Position.x, mPointLight.Position.z), 
		-3.0f)
		+ 10.0f;


	// The spotlight takes on the camera position and is aimed in the
	// same direction the camera is looking.  In this way, it looks
	// like we are holding a flashlight.
	mSpotLight.Position = mEyePosW;
	XMStoreFloat3(&mSpotLight.Direction, XMVector3Normalize(target - pos));

	// Tile water texture.
	XMMATRIX wavesScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);

	// Translate texture over time.
	mWaterTexOffset.y += 0.05f*dt;
	mWaterTexOffset.x += 0.1f*dt;	
	XMMATRIX wavesOffset = XMMatrixTranslation(mWaterTexOffset.x, mWaterTexOffset.y, 0.0f);

	// Combine scale and translation.
	XMStoreFloat4x4(&mWaterTexTransform, wavesScale*wavesOffset);
}

void D3DBlendDemo::DrawScene()
{
	m_d3dImmediateContext->IASetInputLayout(mInputLayout);
	m_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_d3dImmediateContext->ClearRenderTargetView(
		m_RenderTargetView, 
		reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	m_d3dImmediateContext->ClearDepthStencilView(
		m_DepthStencilView,
		D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,
		1.0f, 
		0);

	m_d3dImmediateContext->PSSetShader(mPixelShader, NULL, 0);
	m_d3dImmediateContext->VSSetShader(mVertexShader, NULL, 0);

	DrawLand();
	DrawWaves();
	DrawBox();
	
	

	HR(m_SwapChain->Present(0, 0), L"SwapChain");
}

void D3DBlendDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;
	SetCapture(m_hMainWnd);
}
void D3DBlendDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}
void D3DBlendDemo::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));	
		
		mTheta -= dx;
		mPhi   -= dy;

		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::PI - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

		mRadius -= dx - dy;
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void D3DBlendDemo::BuildLandGemetryBuffers()
{
	GeometryGenerator::MeshData grid;
	GeometryGenerator landGenerator;
	landGenerator.createGrid(160.0f, 160.0f, 50, 50, grid);

	mLandIndexCount = grid.Indices.size();
	std::vector<Vertex> vertices(grid.Vertices.size());
	for(size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;

		p.y = GetHillHeight(p.x, p.z);
		
		vertices[i].Pos    = p;
		vertices[i].Normal = GetHillNormal(p.x, p.z);
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage               = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth           = sizeof(Vertex) * grid.Vertices.size();
	vbd.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags      = 0;
	vbd.MiscFlags           = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_d3dDevice->CreateBuffer(
		&vbd,
		&vinitData,
		&mLandVB),
		L"Create Vertex Buffer");

	D3D11_BUFFER_DESC ibd;
	ibd.Usage               = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth           = sizeof(UINT) * mLandIndexCount;
	ibd.BindFlags           = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags      = 0;
	ibd.MiscFlags           = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
	HR(m_d3dDevice->CreateBuffer(
		&ibd,
		&iinitData,
		&mLandIB),
		L"Create Index Buffer");
}

void D3DBlendDemo::BuildWaveGemetryBuffers()
{

	D3D11_BUFFER_DESC vbd;
	vbd.Usage               = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth           = sizeof(Vertex) * mWaves.VertexCount();
	vbd.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags           = 0;
	vbd.StructureByteStride = 0;

	HR(m_d3dDevice->CreateBuffer(
		&vbd,
		0,
		&mWaveVB),
		L"Create Vertex Buffer");


	std::vector<UINT> indices(3*mWaves.TriangleCount()); // 3 indices per face

	// Iterate over each quad.
	UINT m = mWaves.RowCount();
	UINT n = mWaves.ColumnCount();
	int k = 0;
	for(UINT i = 0; i < m-1; ++i)
	{
		for(DWORD j = 0; j < n-1; ++j)
		{
			indices[k]   = i*n+j;
			indices[k+1] = i*n+j+1;
			indices[k+2] = (i+1)*n+j;

			indices[k+3] = (i+1)*n+j;
			indices[k+4] = i*n+j+1;
			indices[k+5] = (i+1)*n+j+1;

			k += 6; // next quad
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage               = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth           = sizeof(UINT) * indices.size();
	ibd.BindFlags           = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags      = 0;
	ibd.MiscFlags           = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_d3dDevice->CreateBuffer(
		&ibd,
		&iinitData,
		&mWaveIB),
		L"Create Index Buffer");
}

void D3DBlendDemo::BuildBoxGemetryBuffers()
{
	GeometryGenerator::MeshData box;

	GeometryGenerator geoGen;
	geoGen.createBox(1.0f, 1.0f, 1.0f, box);

	std::vector<Vertex> vertices(box.Vertices.size());
	for(UINT i = 0; i < box.Vertices.size(); ++i)
	{
		vertices[i].Pos    = box.Vertices[i].Position;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].Tex    = box.Vertices[i].TexC;
	}

    D3D11_BUFFER_DESC vbd;
	vbd.Usage               = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth           = sizeof(Vertex) * box.Vertices.size();
	vbd.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags           = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_d3dDevice->CreateBuffer(
		&vbd,
		&vinitData,
		&mBoxVB),
		L"Create Vertex Buffer");
	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage               = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth           = sizeof(UINT) * box.Indices.size();
	ibd.BindFlags           = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags      = 0;
	ibd.MiscFlags           = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &box.Indices[0];
	HR(m_d3dDevice->CreateBuffer(
		&ibd,
		&iinitData,
		&mBoxIB),
		L"Create Index Buffer");
}

void D3DBlendDemo::BuildVertexLayout()
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HR(m_d3dDevice->CreateInputLayout(
		vertexDesc,
		3, 
		mVSBlob->GetBufferPointer(),
		mVSBlob->GetBufferSize(),
		&mInputLayout),
		L"Create Input Layout");
}

void D3DBlendDemo::LoadShader()
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
    shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	HR(ShaderHelper::LoadCompiledShader(
		"BlendDemoPS.cso",
		&mPSBlob),
		L"LoadShader PS");
	HR(m_d3dDevice->CreatePixelShader(
		mPSBlob->GetBufferPointer(),
		mPSBlob->GetBufferSize(),
		NULL, 
		&mPixelShader),
		L"Create Pixel Shader");

	HR(ShaderHelper::LoadCompiledShader(
		"BlendDemoVS.cso",
		&mVSBlob),
		L"LoadShader VS");
	HR(m_d3dDevice->CreateVertexShader(
		mVSBlob->GetBufferPointer(),
		mVSBlob->GetBufferSize(),
		NULL, 
		&mVertexShader),
		L"Create Vertex Shader");
}

void D3DBlendDemo::BuildRasterState()
{
	D3D11_RASTERIZER_DESC rs;
	memset(&rs, 0, sizeof(rs));
	rs.FillMode              = D3D11_FILL_SOLID;
	rs.CullMode              = D3D11_CULL_BACK;
	rs.AntialiasedLineEnable = true;
	rs.DepthClipEnable       = true;

	mRasterState = NULL;
	HR(m_d3dDevice->CreateRasterizerState(
		&rs,
		&mRasterState),
		L"Create Rasterizer State");
}

float D3DBlendDemo::GetHillHeight(float x, float z)const
{
	return 0.3f*( z*sinf(0.1f*x) + x*cosf(0.1f*z) );
}

XMFLOAT3 D3DBlendDemo::GetHillNormal(float x, float z)const
{
	// n = (-df/dx, 1, -df/dz)
	XMFLOAT3 n(
		-0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
		1.0f,
		-0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));
	
	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

void D3DBlendDemo::DrawLand()
{
	CBObjectPS cbObjectPSData;
	cbObjectPSData.gMaterial   = mLandMaterial;
	cbObjectPSData.gDirLight   = mDirectLight;
	cbObjectPSData.gPointLight = mPointLight;
	cbObjectPSData.gSpotLight  = mSpotLight;
	cbObjectPSData.gEyePosW    = mEyePosW;
	cbObjectPSData.gFogRange    = 175.0f;
	cbObjectPSData.gFogStart    = 15.0f;
	XMStoreFloat4(&cbObjectPSData.gFogColor, Colors::Silver);
	mCBObjectPS.Data = cbObjectPSData;
	mCBObjectPS.Apply(m_d3dImmediateContext);
	auto bufferPS = mCBObjectPS.Buffer();
	m_d3dImmediateContext->PSSetConstantBuffers(0, 1, &bufferPS);
	m_d3dImmediateContext->PSSetShaderResources(1, 1, &mGrassMapSRV);

	XMMATRIX view              = XMLoadFloat4x4(&mView);
	XMMATRIX proj              = XMLoadFloat4x4(&mProj);
	XMMATRIX world             = XMLoadFloat4x4(&mLandWorld);
	XMMATRIX texTransform      = XMLoadFloat4x4(&mGrassTexTransform);
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldViewProj     = XMMatrixTranspose(world * view * proj);
	CBObjectVS cbObjectVS;
	XMStoreFloat4x4(&cbObjectVS.gWorld, world);
	XMStoreFloat4x4(&cbObjectVS.gWorldInvTranspose, worldInvTranspose);
	XMStoreFloat4x4(&cbObjectVS.gWorldViewProj, worldViewProj);
	XMStoreFloat4x4(&cbObjectVS.gTexTransform, texTransform);
	mCBObjectVS.Data = cbObjectVS;
	mCBObjectVS.Apply(m_d3dImmediateContext);
	auto bufferVS = mCBObjectVS.Buffer();
	m_d3dImmediateContext->VSSetConstantBuffers(0, 1, &bufferVS);
	
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &mLandVB, &stride, &offset);
	m_d3dImmediateContext->IASetIndexBuffer(mLandIB, DXGI_FORMAT_R32_UINT, 0);
	m_d3dImmediateContext->DrawIndexed(mLandIndexCount, 0, 0);
}

void D3DBlendDemo::DrawWaves()
{
	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};

	CBObjectPS cbObjectPSData;
	cbObjectPSData.gMaterial   = mWaveMaterial;
	cbObjectPSData.gDirLight   = mDirectLight;
	cbObjectPSData.gPointLight = mPointLight;
	cbObjectPSData.gSpotLight  = mSpotLight;
	cbObjectPSData.gEyePosW    = mEyePosW;
	cbObjectPSData.gFogRange    = 175.0f;
	cbObjectPSData.gFogStart    = 15.0f;
	XMStoreFloat4(&cbObjectPSData.gFogColor, Colors::Silver);
	mCBObjectPS.Data = cbObjectPSData;
	mCBObjectPS.Apply(m_d3dImmediateContext);
	auto bufferPS = mCBObjectPS.Buffer();
	m_d3dImmediateContext->PSSetConstantBuffers(0, 1, &bufferPS);
	m_d3dImmediateContext->PSSetShaderResources(1, 1, &mWaterMapSRV);

	XMMATRIX view              = XMLoadFloat4x4(&mView);
	XMMATRIX proj              = XMLoadFloat4x4(&mProj);
	XMMATRIX world             = XMLoadFloat4x4(&mWaveWorld);
	XMMATRIX texTransform      = XMLoadFloat4x4(&mWaterTexTransform);
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldViewProj     = XMMatrixTranspose(world * view * proj);
	CBObjectVS cbObjectVS;
	XMStoreFloat4x4(&cbObjectVS.gWorld, world);
	XMStoreFloat4x4(&cbObjectVS.gWorldInvTranspose, worldInvTranspose);
	XMStoreFloat4x4(&cbObjectVS.gWorldViewProj, worldViewProj);
	XMStoreFloat4x4(&cbObjectVS.gTexTransform, texTransform);
	mCBObjectVS.Data = cbObjectVS;
	mCBObjectVS.Apply(m_d3dImmediateContext);
	auto bufferVS = mCBObjectVS.Buffer();
	m_d3dImmediateContext->VSSetConstantBuffers(0, 1, &bufferVS);
	
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &mWaveVB, &stride, &offset);
	m_d3dImmediateContext->IASetIndexBuffer(mWaveIB, DXGI_FORMAT_R32_UINT, 0);
	m_d3dImmediateContext->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);
	m_d3dImmediateContext->DrawIndexed(3 * mWaves.TriangleCount(), 0, 0);
	m_d3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
}

void D3DBlendDemo::DrawBox()
{
	CBObjectPS cbObjectPSData;
	cbObjectPSData.gMaterial   = mBoxMaterial;
	cbObjectPSData.gDirLight   = mDirectLight;
	cbObjectPSData.gPointLight = mPointLight;
	cbObjectPSData.gSpotLight  = mSpotLight;
	cbObjectPSData.gEyePosW    = mEyePosW;
	cbObjectPSData.gFogRange    = 175.0f;
	cbObjectPSData.gFogStart    = 15.0f;
	XMStoreFloat4(&cbObjectPSData.gFogColor, Colors::Silver);
	mCBObjectPS.Data = cbObjectPSData;
	mCBObjectPS.Apply(m_d3dImmediateContext);
	auto bufferPS = mCBObjectPS.Buffer();
	m_d3dImmediateContext->PSSetConstantBuffers(0, 1, &bufferPS);
	m_d3dImmediateContext->PSSetShaderResources(1, 1, &mBoxMapSRV);

	XMMATRIX view              = XMLoadFloat4x4(&mView);
	XMMATRIX proj              = XMLoadFloat4x4(&mProj);
	XMMATRIX world             = XMLoadFloat4x4(&mBoxWorld);
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldViewProj     = XMMatrixTranspose(world * view * proj);
	CBObjectVS cbObjectVS;
	XMStoreFloat4x4(&cbObjectVS.gWorld, world);
	XMStoreFloat4x4(&cbObjectVS.gWorldInvTranspose, worldInvTranspose);
	XMStoreFloat4x4(&cbObjectVS.gWorldViewProj, worldViewProj);
	XMStoreFloat4x4(&cbObjectVS.gTexTransform, XMMatrixIdentity());
	mCBObjectVS.Data = cbObjectVS;
	mCBObjectVS.Apply(m_d3dImmediateContext);
	auto bufferVS = mCBObjectVS.Buffer();
	m_d3dImmediateContext->VSSetConstantBuffers(0, 1, &bufferVS);
	
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
	m_d3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);
	m_d3dImmediateContext->RSSetState(RenderStates::NoCullRS);
	m_d3dImmediateContext->DrawIndexed(36, 0, 0);
	m_d3dImmediateContext->RSSetState(0);
}


