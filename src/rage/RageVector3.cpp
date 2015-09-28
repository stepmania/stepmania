#include "RageVector3.hpp"

RageVector3::RageVector3() : x(0), y(0), z(0)
{
}

RageVector3::RageVector3(float a, float b, float c) : x(a), y(b), z(c)
{
}

RageVector3 & RageVector3::operator+=(const RageVector3 & rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return *this;
}

RageVector3 & RageVector3::operator-=(const RageVector3 & rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	return *this;
}

RageVector3 & RageVector3::operator*=(float rhs)
{
	x *= rhs;
	y *= rhs;
	z *= rhs;
	return *this;
}

RageVector3 & RageVector3::operator/=(float rhs)
{
	x /= rhs;
	y /= rhs;
	z /= rhs;
	return *this;
}
