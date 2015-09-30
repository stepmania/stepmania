#include "RageVector3.hpp"

Rage::Vector3::Vector3() : x(0), y(0), z(0)
{
}

Rage::Vector3::Vector3(float a, float b, float c) : x(a), y(b), z(c)
{
}

Rage::Vector3 & Rage::Vector3::operator+=(const Rage::Vector3 & rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return *this;
}

Rage::Vector3 & Rage::Vector3::operator-=(const Rage::Vector3 & rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	return *this;
}

Rage::Vector3 & Rage::Vector3::operator*=(float rhs)
{
	x *= rhs;
	y *= rhs;
	z *= rhs;
	return *this;
}

Rage::Vector3 & Rage::Vector3::operator/=(float rhs)
{
	x /= rhs;
	y /= rhs;
	z /= rhs;
	return *this;
}
