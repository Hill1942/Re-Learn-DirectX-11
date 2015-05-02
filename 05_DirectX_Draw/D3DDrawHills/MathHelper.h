#pragma once

#include <windows.h>
#include <DirectXMath.h>

using namespace DirectX;

class MathHelper
{
public:
	static const float PI;

	static float RandF()
	{
		return float(rand()) / float(RAND_MAX);
	}
	static float RandF(float a, float b)
	{
		return a + RandF() * (b - a);
	}

	template<typename T>
	static T Min(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	template<typename T>
	static T Max(const T&a, const T& b)
	{
		return a > b ? a : b;
	}

	template<typename T>
	static T Lerp(const T& a, const T& b, float t)
	{
		return a + (b - a) * t;
	}

	template<typename T>
	static T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low: (x > high ? high: x);
	}

	static float AngleFromXY(float x, float y)
	{
		float theta = 0.0f;
		if (x >= 0.0f)
		{
			theta = atanf(y / x);
			if (theta < 0.0f)
				theta += 2.0f * PI;
		}
		else
		{
			theta = atanf(y / x) + PI;
		}
	}

	static XMMATRIX InverseTranspose(CXMMATRIX m)
	{
		XMMATRIX a = m;
		a.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		XMVECTOR det = XMMatrixDeterminant(a);

		return XMMatrixTranspose(XMMatrixInverse(&det, a));
	}

	static XMVECTOR RandUnitVec3()
	{
		XMVECTOR one = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		XMVECTOR zero = XMVectorZero();
		while (true)
		{
			XMVECTOR v = XMVectorSet(
				RandF(-1.0f, 1.0f), 
				RandF(-1.0f, 1.0f),
				RandF(-1.0f, 1.0f),
				0.0f);
			if (XMVector3Greater(XMVector3LengthSq(v), one))
				continue;

			return XMVector3Normalize(v);
		}
	}

	static XMVECTOR RandHemisphereUnitVec3(XMVECTOR n)
	{
		XMVECTOR one = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		XMVECTOR zero = XMVectorZero();
		while (true)
		{
			XMVECTOR v = XMVectorSet(
				RandF(-1.0f, 1.0f), 
				RandF(-1.0f, 1.0f),
				RandF(-1.0f, 1.0f),
				0.0f);
			if (XMVector3Greater(XMVector3LengthSq(v), one))
				continue;
			if (XMVector3Less(XMVector3Dot(n, v), zero))
				continue;

			return XMVector3Normalize(v);
		}
	}
};

const float MathHelper::PI = 3.1415926535f;

