#include "ShaderHelper.h"
#include "ConstantBuffer.h"
#include "D3DBoxApp.h"


D3DBoxApp::D3DBoxApp(HINSTANCE hInstance)
	:D3DApp(hInstance),
	 mBoxIB(0),
	 mBoxVB(0),
	 mInputLayout(0),
	 mTheta(1.5f * 3.14f),
	 mPhi(0.25f * 3.34f),
	 mRadius(5.0f)
{
	m_MainWndCaption = L"D3DBoxApp";

	DirectX::XMMATRIX I = DirectX::XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
}


D3DBoxApp::~D3DBoxApp(void)
{
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mPSBlob);
	ReleaseCOM(mVSBlob);
}

bool D3DBoxApp::Init()
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

void D3DBoxApp::OnResize()
{
	D3DApp::OnResize();
	DirectX::XMMATRIX p = DirectX::XMMatrixPerspectiveFovLH(
		0.25f * 3.14,
		AspectRadio(), 
		1.0f, 
		1000.0f);

	XMStoreFloat4x4(&mProj, p);
}

void D3DBoxApp::UpdateScene(float dt)
{
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	DirectX::XMVECTOR pos    = DirectX::XMVectorSet(x, y, z, 1.0f);
	DirectX::XMVECTOR target = DirectX::XMVectorZero();
	DirectX::XMVECTOR up     = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	DirectX::XMMATRIX v      = DirectX::XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, v);
}

void D3DBoxApp::DrawScene()
{
	m_d3dImmediateContext->IASetInputLayout(mInputLayout);
	m_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_d3dImmediateContext->ClearRenderTargetView(
		m_RenderTargetView, 
		reinterpret_cast<const float*>(&DirectX::Colors::LightSteelBlue));
	m_d3dImmediateContext->ClearDepthStencilView(
		m_DepthStencilView,
		D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,
		1.0f, 
		0);

	m_d3dImmediateContext->PSSetShader(mPixelShader, NULL, 0);
	m_d3dImmediateContext->VSSetShader(mVertexShader, NULL, 0);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
	m_d3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

	
	DirectX::XMMATRIX world        = XMLoadFloat4x4(&mWorld);
	DirectX::XMMATRIX view         = XMLoadFloat4x4(&mView);
	DirectX::XMMATRIX proj         = XMLoadFloat4x4(&mProj);
	DirectX::XMMATRIX wordViewProj = XMMatrixTranspose(world * view * proj);

	CBPerspectiveObject cbPerspectiveObject;
	XMStoreFloat4x4(&cbPerspectiveObject.mWorldViewProj, wordViewProj);
	mCBObject.Data = cbPerspectiveObject;
	mCBObject.Apply(m_d3dImmediateContext);

	auto buffer = mCBObject.Buffer();
	m_d3dImmediateContext->VSSetConstantBuffers(0, 1, &buffer);

	m_d3dImmediateContext->DrawIndexed(36, 0, 0);
	HR(m_SwapChain->Present(0, 0), L"SwapChain");
}

void D3DBoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	
}
void D3DBoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	
}
void D3DBoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	
}

void D3DBoxApp::BuildGemetryBuffers()
{
	 Vertex vertices[] =
    {
		{ 
			DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), 
			static_cast<const DirectX::XMFLOAT4>(DirectX::Colors::White)   
		},
		{
			DirectX::XMFLOAT3(-1.0f, +1.0f, -1.0f), 
			static_cast<const DirectX::XMFLOAT4>(DirectX::Colors::Black) 
		},
		{
			DirectX::XMFLOAT3(+1.0f, +1.0f, -1.0f), 
			static_cast<const DirectX::XMFLOAT4>(DirectX::Colors::Red)   
		},
		{
			DirectX::XMFLOAT3(+1.0f, -1.0f, -1.0f),
			static_cast<const DirectX::XMFLOAT4>(DirectX::Colors::Green)	
		},
		{
			DirectX::XMFLOAT3(-1.0f, -1.0f, +1.0f), 
			static_cast<const DirectX::XMFLOAT4>(DirectX::Colors::Blue)	
		},
		{
			DirectX::XMFLOAT3(-1.0f, +1.0f, +1.0f),
			static_cast<const DirectX::XMFLOAT4>(DirectX::Colors::Yellow) 
		},
		{
			DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), 
			static_cast<const DirectX::XMFLOAT4>(DirectX::Colors::Cyan)
		},
		{ 
			DirectX::XMFLOAT3(+1.0f, -1.0f, +1.0f),
			static_cast<const DirectX::XMFLOAT4>(DirectX::Colors::Magenta) 
		}
    };

	 D3D11_BUFFER_DESC vbd;
	 vbd.Usage               = D3D11_USAGE_IMMUTABLE;
	 vbd.ByteWidth           = sizeof(Vertex) * 8;
	 vbd.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	 vbd.CPUAccessFlags      = 0;
	 vbd.MiscFlags           = 0;
	 vbd.StructureByteStride = 0;
	 
	 D3D11_SUBRESOURCE_DATA vinitData;
	 vinitData.pSysMem = vertices;
	 HR(m_d3dDevice->CreateBuffer(
		 &vbd,
		 &vinitData,
		 &mBoxVB),
		 L"Create Vertex Buffer");

	 UINT indices[] = 
	 {
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3, 
		4, 3, 7
	};

	 D3D11_BUFFER_DESC ibd;
	 ibd.Usage               = D3D11_USAGE_IMMUTABLE;
	 ibd.ByteWidth           = sizeof(UINT) * 36;
	 ibd.BindFlags           = D3D11_BIND_INDEX_BUFFER;
	 ibd.CPUAccessFlags      = 0;
	 ibd.MiscFlags           = 0;
	 ibd.StructureByteStride = 0;
	 
	 D3D11_SUBRESOURCE_DATA iinitData;
	 iinitData.pSysMem = indices;
	 HR(m_d3dDevice->CreateBuffer(
		 &ibd,
		 &iinitData,
		 &mBoxIB),
		 L"Create Index Buffer");
}

void D3DBoxApp::BuildVertexLayout()
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

void D3DBoxApp::LoadShader()
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

void D3DBoxApp::BuildRasterState()
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




