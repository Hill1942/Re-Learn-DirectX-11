#include "MathHelper.h"
#include "ShaderHelper.h"
#include "GeometryGenerator.h"
#include "ConstantBuffer.h"
#include "D3DHillApp.h"


D3DHillApp::D3DHillApp(HINSTANCE hInstance)
	:D3DApp(hInstance),
	 mHillIB(0),
	 mHillVB(0),
	 mInputLayout(0),
	 mTheta(1.5f * MathHelper::PI),
	 mPhi(0.1f * MathHelper::PI),
	 mRadius(200.0f)
{
	m_MainWndCaption = L"D3DHillApp";

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
}


D3DHillApp::~D3DHillApp(void)
{
	ReleaseCOM(mHillVB);
	ReleaseCOM(mHillIB);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mPSBlob);
	ReleaseCOM(mVSBlob);
}

bool D3DHillApp::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildGemetryBuffers();
	LoadShader();
	BuildVertexLayout();
	BuildRasterState();
	mCBObject.Initilize(m_d3dDevice);

	return true;
}

void D3DHillApp::OnResize()
{
	D3DApp::OnResize();
	XMMATRIX p = XMMatrixPerspectiveFovLH(
		0.25f * MathHelper::PI,
		AspectRadio(), 
		1.0f, 
		1000.0f);

	XMStoreFloat4x4(&mProj, p);
}

void D3DHillApp::UpdateScene(float dt)
{
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX v      = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, v);
}

void D3DHillApp::DrawScene()
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
	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &mHillVB, &stride, &offset);
	m_d3dImmediateContext->IASetIndexBuffer(mHillIB, DXGI_FORMAT_R32_UINT, 0);

	
	XMMATRIX world        = XMLoadFloat4x4(&mWorld);
	XMMATRIX view         = XMLoadFloat4x4(&mView);
	XMMATRIX proj         = XMLoadFloat4x4(&mProj);
	XMMATRIX wordViewProj = XMMatrixTranspose(world * view * proj);

	CBPerspectiveObject cbPerspectiveObject;
	XMStoreFloat4x4(&cbPerspectiveObject.mWorldViewProj, wordViewProj);
	mCBObject.Data = cbPerspectiveObject;
	mCBObject.Apply(m_d3dImmediateContext);

	auto buffer = mCBObject.Buffer();
	m_d3dImmediateContext->VSSetConstantBuffers(0, 1, &buffer);

	m_d3dImmediateContext->DrawIndexed(mGridIndexCount, 0, 0);
	HR(m_SwapChain->Present(0, 0), L"SwapChain");
}

void D3DHillApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;
	SetCapture(m_hMainWnd);
}
void D3DHillApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}
void D3DHillApp::OnMouseMove(WPARAM btnState, int x, int y)
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

void D3DHillApp::BuildGemetryBuffers()
{
	GeometryGenerator::MeshData grid;
	GeometryGenerator gridGenerator;
	gridGenerator.createGrid(160.0f, 160.0f, 50, 50, grid);
	mGridIndexCount = grid.Indices.size();

	std::vector<Vertex> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); i++)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;
		p.y = GetHeight(p.x, p.z);
		vertices[i].Pos = p;
		if( p.y < -10.0f )
			vertices[i].Color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);  // Sandy beach color.
		else if( p.y < 5.0f )
			vertices[i].Color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);  // Light yellow-green.
		else if( p.y < 12.0f )
			vertices[i].Color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);  // Dark yellow-green.
		else if( p.y < 20.0f )
			vertices[i].Color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);  // Dark brown.
		else
			vertices[i].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);   // White snow.
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
		&mHillVB),
		L"Create Vertex Buffer");


	D3D11_BUFFER_DESC ibd;
	ibd.Usage               = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth           = sizeof(UINT) * mGridIndexCount;
	ibd.BindFlags           = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags      = 0;
	ibd.MiscFlags           = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
	HR(m_d3dDevice->CreateBuffer(
		&ibd,
		&iinitData,
		&mHillIB),
		L"Create Index Buffer");
}

void D3DHillApp::BuildVertexLayout()
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HR(m_d3dDevice->CreateInputLayout(
		vertexDesc,
		2, 
		mVSBlob->GetBufferPointer(),
		mVSBlob->GetBufferSize(),
		&mInputLayout),
		L"Create Input Layout");
}

void D3DHillApp::LoadShader()
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
    shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	HR(ShaderHelper::LoadCompiledShader(
		"DrawBoxPS.cso",
		&mPSBlob),
		L"LoadShader PS");
	HR(m_d3dDevice->CreatePixelShader(
		mPSBlob->GetBufferPointer(),
		mPSBlob->GetBufferSize(),
		NULL, 
		&mPixelShader),
		L"Create Pixel Shader");

	HR(ShaderHelper::LoadCompiledShader(
		"DrawBoxVS.cso",
		&mVSBlob),
		L"LoadShader VS");
	HR(m_d3dDevice->CreateVertexShader(
		mVSBlob->GetBufferPointer(),
		mVSBlob->GetBufferSize(),
		NULL, 
		&mVertexShader),
		L"Create Vertex Shader");
}

void D3DHillApp::BuildRasterState()
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

float D3DHillApp::GetHeight(float x, float z) const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}




