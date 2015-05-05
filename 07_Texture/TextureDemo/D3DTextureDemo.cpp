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
	 mTheta(1.5f * MathHelper::PI),
	 mPhi(0.1f * MathHelper::PI),
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

	// Directional light.
	mDirectLight.Ambient   = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
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

	mLandMaterial.Ambient  = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	mLandMaterial.Diffuse  = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	mLandMaterial.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	mWaveMaterial.Ambient  = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	mWaveMaterial.Diffuse  = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	mWaveMaterial.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 96.0f);
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
		L"ddd",
		0,
		&mGrassMapSRV,
		0,
		nullptr),
		L"Create DDS texture");


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

	CBLightingObject cbLightingObjectPS;
	cbLightingObjectPS.gMaterial = mLandMaterial;
	cbLightingObjectPS.gDirLight = mDirectLight;
	cbLightingObjectPS.gPointLight = mPointLight;
	cbLightingObjectPS.gSpotLight = mSpotLight;
	cbLightingObjectPS.gEyePosW   = mEyePosW;
	mCBLightingObject.Data = cbLightingObjectPS;
	mCBLightingObject.Apply(m_d3dImmediateContext);
	auto bufferPS = mCBLightingObject.Buffer();
	m_d3dImmediateContext->PSSetConstantBuffers(0, 1, &bufferPS);

	XMMATRIX world             = XMLoadFloat4x4(&mLandWorld);
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldViewProj     = XMMatrixTranspose(world * view * proj);
	CBObject cbObjectVS;
	XMStoreFloat4x4(&cbObjectVS.gWorld, world);
	XMStoreFloat4x4(&cbObjectVS.gWorldInvTranspose, worldInvTranspose);
	XMStoreFloat4x4(&cbObjectVS.gWorldViewProj, worldViewProj);
	mCBObject.Data = cbObjectVS;
	mCBObject.Apply(m_d3dImmediateContext);
	auto bufferVS = mCBObject.Buffer();
	m_d3dImmediateContext->VSSetConstantBuffers(0, 1, &bufferVS);
	
	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &mLandVB, &stride, &offset);
	m_d3dImmediateContext->IASetIndexBuffer(mLandIB, DXGI_FORMAT_R32_UINT, 0);
	m_d3dImmediateContext->DrawIndexed(mLandIndexCount, 0, 0);



	cbLightingObjectPS.gMaterial = mWaveMaterial;
	mCBLightingObject.Data = cbLightingObjectPS;
	mCBLightingObject.Apply(m_d3dImmediateContext);
	bufferPS = mCBLightingObject.Buffer();
	m_d3dImmediateContext->PSSetConstantBuffers(0, 1, &bufferPS);

	world             = XMLoadFloat4x4(&mWaveWorld);
	worldInvTranspose = MathHelper::InverseTranspose(world);
	worldViewProj     = XMMatrixTranspose(world * view * proj);
	XMStoreFloat4x4(&cbObjectVS.gWorld, world);
	XMStoreFloat4x4(&cbObjectVS.gWorldInvTranspose, worldInvTranspose);
	XMStoreFloat4x4(&cbObjectVS.gWorldViewProj, worldViewProj);
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
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HR(m_d3dDevice->CreateInputLayout(
		vertexDesc,
		2, 
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