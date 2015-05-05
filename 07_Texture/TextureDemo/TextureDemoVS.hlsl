cbuffer CBObjectVS : register(b0)
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float3 PosW    : POSITION;
	float3 NormalW : NORMAL;
	float2 Tex     : TEXCOORD;
};

VertexOut main(VertexIn vin)
{
	//transform to homogenous clip space 
	VertexOut vout;
	vout.PosW    = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
	vout.PosH    = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.Tex     = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	return vout;
}