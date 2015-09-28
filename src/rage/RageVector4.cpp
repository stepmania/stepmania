#include "RageVector4.hpp"

RageVector4::RageVector4() : x(0), y(0), z(0), w(0)
{
}

RageVector4::RageVector4(float a, float b, float c, float d) : x(a), y(b), z(c), w(d)
{
}

RageVector4 & RageVector4::operator+=(const RageVector4 & rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	w += rhs.w;
	return *this;
}

RageVector4 & RageVector4::operator-=(const RageVector4 & rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	w -= rhs.w;
	return *this;
}

RageVector4 & RageVector4::operator*=(float rhs)
{
	x *= rhs;
	y *= rhs;
	z *= rhs;
	w *= rhs;
	return *this;
}

RageVector4 & RageVector4::operator/=(float rhs)
{
	x /= rhs;
	y /= rhs;
	z /= rhs;
	w /= rhs;
	return *this;
}
