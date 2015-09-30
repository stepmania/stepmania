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
