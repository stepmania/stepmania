#include "RageVector4.hpp"

Rage::Vector4::Vector4() : x(0), y(0), z(0), w(0)
{
}

Rage::Vector4::Vector4(float a, float b, float c, float d) : x(a), y(b), z(c), w(d)
{
}

Rage::Vector4 & Rage::Vector4::operator+=(const Rage::Vector4 & rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	w += rhs.w;
	return *this;
}

Rage::Vector4 & Rage::Vector4::operator-=(const Rage::Vector4 & rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	w -= rhs.w;
	return *this;
}

Rage::Vector4 & Rage::Vector4::operator*=(float rhs)
{
	x *= rhs;
	y *= rhs;
	z *= rhs;
	w *= rhs;
	return *this;
}

Rage::Vector4 & Rage::Vector4::operator/=(float rhs)
{
	x /= rhs;
	y /= rhs;
	z /= rhs;
	w /= rhs;
	return *this;
}

Rage::Vector4 Rage::Vector4::TransformCoords(Rage::Matrix const &mat) const
{
	return Rage::Vector4
	{
		mat.m[0][0] * x + mat.m[1][0] * y + mat.m[2][0] * z + mat.m[3][0] * w,
		mat.m[0][1] * x + mat.m[1][1] * y + mat.m[2][1] * z + mat.m[3][1] * w,
		mat.m[0][2] * x + mat.m[1][2] * y + mat.m[2][2] * z + mat.m[3][2] * w,
		mat.m[0][3] * x + mat.m[1][3] * y + mat.m[2][3] * z + mat.m[3][3] * w
	};
}
