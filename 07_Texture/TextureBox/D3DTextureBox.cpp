#include <DDSTextureLoader.h>
#include "MathHelper.h"
#include "ShaderHelper.h"
#include "ConstantBuffer.h"
#include "GeometryGenerator.h"
#include "D3DTextureBox.h"


D3DTextureBox::D3DTextureBox(HINSTANCE hInstance)
	:D3DApp(hInstance),
	 mBoxIB(0),
	 mBoxVB(0),
	 mInputLayout(0),
	 mEyePosW(0.0f, 0.0f, 0.0f),
	 mTheta(1.3f * MathHelper::PI),
	 mPhi(0.4f * MathHelper::PI),
	 mRadius(2.5f)
{
	m_MainWndCaption = L"D3DTextureBox";
	mLastMousePos.x  = 0;
	mLastMousePos.y  = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mBoxWorld, I);
	XMStoreFloat4x4(&mTexTransform, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

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

	mBoxMaterial.Ambient  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMaterial.Diffuse  = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	mBoxMaterial.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
}


D3DTextureBox::~D3DTextureBox(void)
{
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mPSBlob);
	ReleaseCOM(mVSBlob);
	ReleaseCOM(mDiffuseMapSRV);
}

bool D3DTextureBox::Init()
{
	if (!D3DApp::Init())
		return false;
	HR(CreateDDSTextureFromFile(
		m_d3dDevice,
		L"Texture/WoodCrate02.dds",
		0,
		&mDiffuseMapSRV,
		0,
		nullptr),
		L"Create Texture file");
	BuildBoxGemetryBuffers();
	LoadShader();
	BuildVertexLayout();
	BuildRasterState();
	mCBObjectVS.Initilize(m_d3dDevice);
	mCBObjectPS.Initilize(m_d3dDevice);

	return true;
}

void D3DTextureBox::OnResize()
{
	D3DApp::OnResize();
	XMMATRIX p = XMMatrixPerspectiveFovLH(
		0.25f * MathHelper::PI,
		AspectRadio(), 
		1.0f, 
		1000.0f);

	XMStoreFloat4x4(&mProj, p);
}

void D3DTextureBox::UpdateScene(float dt)
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

	mSpotLight.Position = mEyePosW;
	XMStoreFloat3(&mSpotLight.Direction, XMVector3Normalize(target - pos));
}

void D3DTextureBox::DrawScene()
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

	CBObjectPS cbLightingObjectPS;
	cbLightingObjectPS.gMaterial = mBoxMaterial;
	cbLightingObjectPS.gDirLight = mDirectLight;
	cbLightingObjectPS.gPointLight = mPointLight;
	cbLightingObjectPS.gSpotLight = mSpotLight;
	cbLightingObjectPS.gEyePosW   = mEyePosW;
	mCBObjectPS.Data = cbLightingObjectPS;
	mCBObjectPS.Apply(m_d3dImmediateContext);
	auto bufferPS = mCBObjectPS.Buffer();
	m_d3dImmediateContext->PSSetConstantBuffers(0, 1, &bufferPS);
	m_d3dImmediateContext->PSSetShaderResources(1, 1, &mDiffuseMapSRV);
	
	XMMATRIX world             = XMLoadFloat4x4(&mBoxWorld);
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldViewProj     = XMMatrixTranspose(world * view * proj);
	XMMATRIX texTransform      = XMLoadFloat4x4(&mTexTransform);
	CBObjectVS cbObjectVS;
	XMStoreFloat4x4(&cbObjectVS.gWorld, world);
	XMStoreFloat4x4(&cbObjectVS.gWorldInvTranspose, worldInvTranspose);
	XMStoreFloat4x4(&cbObjectVS.gWorldViewProj, worldViewProj);
	XMStoreFloat4x4(&cbObjectVS.gTexTransform, texTransform);
	mCBObjectVS.Data = cbObjectVS;
	mCBObjectVS.Apply(m_d3dImmediateContext);
	auto bufferVS = mCBObjectVS.Buffer();
	m_d3dImmediateContext->VSSetConstantBuffers(0, 1, &bufferVS);
	
	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
	m_d3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);
	m_d3dImmediateContext->DrawIndexed(mBoxIndexCount, 0, 0);


	HR(m_SwapChain->Present(0, 0), L"SwapChain");
}

void D3DTextureBox::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;
	SetCapture(m_hMainWnd);
}
void D3DTextureBox::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}
void D3DTextureBox::OnMouseMove(WPARAM btnState, int x, int y)
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

void D3DTextureBox::BuildBoxGemetryBuffers()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator boxGenerator;
	boxGenerator.createBox(1.0f, 1.0f, 1.0f, box);

	mBoxIndexCount = box.Indices.size();
	std::vector<Vertex> vertices(box.Vertices.size());
	for(size_t i = 0; i < box.Vertices.size(); ++i)
	{
		vertices[i].Pos     = box.Vertices[i].Position;
		vertices[i].Normal  = box.Vertices[i].Normal;
		vertices[i].Texture = box.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage               = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth           = sizeof(Vertex) * box.Vertices.size();
	vbd.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags      = 0;
	vbd.MiscFlags           = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_d3dDevice->CreateBuffer(
		&vbd,
		&vinitData,
		&mBoxVB),
		L"Create Vertex Buffer");

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	D3D11_BUFFER_DESC ibd;
	ibd.Usage               = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth           = sizeof(UINT) * box.Indices.size();
	ibd.BindFlags           = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags      = 0;
	ibd.MiscFlags           = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_d3dDevice->CreateBuffer(
		&ibd,
		&iinitData,
		&mBoxIB),
		L"Create Index Buffer");
}

void D3DTextureBox::BuildVertexLayout()
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

void D3DTextureBox::LoadShader()
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
    shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	HR(ShaderHelper::LoadCompiledShader(
		"TextureBoxPS.cso",
		&mPSBlob),
		L"LoadShader PS");
	HR(m_d3dDevice->CreatePixelShader(
		mPSBlob->GetBufferPointer(),
		mPSBlob->GetBufferSize(),
		NULL, 
		&mPixelShader),
		L"Create Pixel Shader");

	HR(ShaderHelper::LoadCompiledShader(
		"TextureBoxVS.cso",
		&mVSBlob),
		L"LoadShader VS");
	HR(m_d3dDevice->CreateVertexShader(
		mVSBlob->GetBufferPointer(),
		mVSBlob->GetBufferSize(),
		NULL, 
		&mVertexShader),
		L"Create Vertex Shader");
}

void D3DTextureBox::BuildRasterState()
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



