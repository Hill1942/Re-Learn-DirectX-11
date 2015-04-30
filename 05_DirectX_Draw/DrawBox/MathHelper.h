#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class MathHelper
{
public:
	static const float PI;

	template<typename T>
	static T Clamp(const T& x, const T& low, const T& high);
};

const float MathHelper::PI = 3.1415926535f;

template <typename T>
T MathHelper::Clamp(const T& x, const T& low, const T& high)
{
	return x < low ? low: (x > high ? high: x);
}
