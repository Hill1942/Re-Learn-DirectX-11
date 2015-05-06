#include "MathHelper.h"
#include "ShaderHelper.h"
#include "ConstantBuffer.h"
#include "GeometryGenerator.h"
#include "DDSTextureLoader.h"
#include "D3DTextureDemo.h"


D3DTextureDemo::D3DTextureDemo(HINSTANCE hInstance)
	:D3DApp(hInstance),
	 mLandIB(0),
	 mLandVB(0),
	 mWaveIB(0),
	 mWaveVB(0),
	 mGrassMapSRV(0),
	 mWavesMapSRV(0),
	 mWaterTexOffset(0.0f, 0.0f),
	 mInputLayout(0),
	 mEyePosW(0.0f, 0.0f, 0.0f),
	 mTheta(1.3f * MathHelper::PI),
	 mPhi(0.4f * MathHelper::PI),
	 mRadius(80.0f)
{
	m_MainWndCaption = L"D3DTextureDemo";

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mLandWorld, I);
	XMStoreFloat4x4(&mWaveWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	XMMATRIX wavesOffset = XMMatrixTranslation(0.0f, -3.0f, 0.0f);
	XMStoreFloat4x4(&mWaveWorld, wavesOffset);

	XMMATRIX grassTexScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);
	XMStoreFloat4x4(&mGrassTexTransform, grassTexScale);

	mDirLights[0].Ambient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mDirLights[1].Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse  = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	mLandMaterial.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mLandMaterial.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mLandMaterial.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	mWaveMaterial.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mWaveMaterial.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mWaveMaterial.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);
}


D3DTextureDemo::~D3DTextureDemo(void)
{
	ReleaseCOM(mLandVB);
	ReleaseCOM(mLandIB);
	ReleaseCOM(mWaveVB);
	ReleaseCOM(mWaveIB);
	ReleaseCOM(mGrassMapSRV);
	ReleaseCOM(mWavesMapSRV);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mPSBlob);
	ReleaseCOM(mVSBlob);
}

bool D3DTextureDemo::Init()
{
	if (!D3DApp::Init())
		return false;

	mWaves.Init(160, 160, 1.0f, 0.03f, 3.25f, 0.4f);
	BuildLandGemetryBuffers();
	BuildWaveGemetryBuffers();
	LoadShader();
	BuildVertexLayout();
	BuildRasterState();
	mCBObject.Initilize(m_d3dDevice);
	mCBLightingObject.Initilize(m_d3dDevice);

	//HR(D3DX11CreateShaderResourceViewFromFile(
	HR(CreateDDSTextureFromFile(
		m_d3dDevice,
		L"Texture/grass.dds",
		0,
		&mGrassMapSRV,
		0,
		nullptr),
		L"Create DDS grass texture");

	HR(CreateDDSTextureFromFile(
		m_d3dDevice,
		L"Texture/water2.dds",
		0,
		&mWavesMapSRV,
		0,
		nullptr),
		L"Create DDS wave texture");

	return true;
}

void D3DTextureDemo::OnResize()
{
	D3DApp::OnResize();
	XMMATRIX p = XMMatrixPerspectiveFovLH(
		0.25f * 3.14,
		AspectRadio(), 
		1.0f, 
		1000.0f);

	XMStoreFloat4x4(&mProj, p);
}

void D3DTextureDemo::UpdateScene(float dt)
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

	XMMATRIX wavesScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);
	mWaterTexOffset.y += 0.05f * dt;
	mWaterTexOffset.x += 0.1f  * dt;	
	XMMATRIX wavesOffset = XMMatrixTranslation(mWaterTexOffset.x, mWaterTexOffset.y, 0.0f);
	XMStoreFloat4x4(&mWaterTexTransform, wavesScale*wavesOffset);
}

void D3DTextureDemo::DrawScene()
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

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	XMMATRIX view  = XMLoadFloat4x4(&mView);
	XMMATRIX proj  = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view*proj;

	CBObjectPS cbObjectPS;
	cbObjectPS.gMaterial = mLandMaterial;
	cbObjectPS.gDirLights[0] = mDirLights[0];
	cbObjectPS.gDirLights[1] = mDirLights[1];
	cbObjectPS.gDirLights[2] = mDirLights[2];
	cbObjectPS.gEyePosW   = mEyePosW;
	mCBLightingObject.Data = cbObjectPS;
	mCBLightingObject.Apply(m_d3dImmediateContext);
	auto bufferPS = mCBLightingObject.Buffer();
	m_d3dImmediateContext->PSSetConstantBuffers(0, 1, &bufferPS);
	m_d3dImmediateContext->PSSetShaderResources(1, 1, &mGrassMapSRV);

	XMMATRIX world             = XMLoadFloat4x4(&mLandWorld);
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldViewProj     = XMMatrixTranspose(world * view * proj);
	XMMATRIX texTransform = XMLoadFloat4x4(&mGrassTexTransform);
	CBObjectVS cbObjectVS;
	XMStoreFloat4x4(&cbObjectVS.gWorld, world);
	XMStoreFloat4x4(&cbObjectVS.gWorldInvTranspose, worldInvTranspose);
	XMStoreFloat4x4(&cbObjectVS.gWorldViewProj, worldViewProj);
	XMStoreFloat4x4(&cbObjectVS.gTexTransform, texTransform);
	mCBObject.Data = cbObjectVS;
	mCBObject.Apply(m_d3dImmediateContext);
	auto bufferVS = mCBObject.Buffer();
	m_d3dImmediateContext->VSSetConstantBuffers(0, 1, &bufferVS);
	
	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &mLandVB, &stride, &offset);
	m_d3dImmediateContext->IASetIndexBuffer(mLandIB, DXGI_FORMAT_R32_UINT, 0);
	m_d3dImmediateContext->DrawIndexed(mLandIndexCount, 0, 0);



	cbObjectPS.gMaterial = mWaveMaterial;
	mCBLightingObject.Data = cbObjectPS;
	mCBLightingObject.Apply(m_d3dImmediateContext);
	bufferPS = mCBLightingObject.Buffer();
	m_d3dImmediateContext->PSSetConstantBuffers(0, 1, &bufferPS);
	m_d3dImmediateContext->PSSetShaderResources(1, 1, &mWavesMapSRV);

	world             = XMLoadFloat4x4(&mWaveWorld);
	worldInvTranspose = MathHelper::InverseTranspose(world);
	worldViewProj     = XMMatrixTranspose(world * view * proj);
	texTransform      = XMLoadFloat4x4(&mWaterTexTransform);
	XMStoreFloat4x4(&cbObjectVS.gWorld, world);
	XMStoreFloat4x4(&cbObjectVS.gWorldInvTranspose, worldInvTranspose);
	XMStoreFloat4x4(&cbObjectVS.gWorldViewProj, worldViewProj);
	XMStoreFloat4x4(&cbObjectVS.gTexTransform, texTransform);
	mCBObject.Data = cbObjectVS;
	mCBObject.Apply(m_d3dImmediateContext);
	bufferVS = mCBObject.Buffer();

	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &mWaveVB, &stride, &offset);
	m_d3dImmediateContext->IASetIndexBuffer(mWaveIB, DXGI_FORMAT_R32_UINT, 0);
	m_d3dImmediateContext->DrawIndexed(3 * mWaves.TriangleCount(), 0, 0);


	HR(m_SwapChain->Present(0, 0), L"SwapChain");
}

void D3DTextureDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;
	SetCapture(m_hMainWnd);
}
void D3DTextureDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}
void D3DTextureDemo::OnMouseMove(WPARAM btnState, int x, int y)
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

void D3DTextureDemo::BuildLandGemetryBuffers()
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
		vertices[i].Tex    = grid.Vertices[i].TexC;
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

void D3DTextureDemo::BuildWaveGemetryBuffers()
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

void D3DTextureDemo::BuildVertexLayout()
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

void D3DTextureDemo::LoadShader()
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
    shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	HR(ShaderHelper::LoadCompiledShader(
		"TextureDemoPS.cso",
		&mPSBlob),
		L"LoadShader PS");
	HR(m_d3dDevice->CreatePixelShader(
		mPSBlob->GetBufferPointer(),
		mPSBlob->GetBufferSize(),
		NULL, 
		&mPixelShader),
		L"Create Pixel Shader");

	HR(ShaderHelper::LoadCompiledShader(
		"TextureDemoVS.cso",
		&mVSBlob),
		L"LoadShader VS");
	HR(m_d3dDevice->CreateVertexShader(
		mVSBlob->GetBufferPointer(),
		mVSBlob->GetBufferSize(),
		NULL, 
		&mVertexShader),
		L"Create Vertex Shader");
}

void D3DTextureDemo::BuildRasterState()
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

float D3DTextureDemo::GetHillHeight(float x, float z)const
{
	return 0.3f*( z*sinf(0.1f*x) + x*cosf(0.1f*z) );
}

XMFLOAT3 D3DTextureDemo::GetHillNormal(float x, float z)const
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