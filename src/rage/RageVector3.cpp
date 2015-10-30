#include "RageVector3.hpp"
#include "RageVector4.hpp"
#include <cmath>

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

Rage::Vector3 Rage::Vector3::GetNormalized() const
{
	float scale = 1.0f / std::sqrt( x * x + y * y + z * z );
	return (*this) * scale;
}

Rage::Vector3 Rage::Vector3::TransformCoords(Rage::Matrix const &mat) const
{
	Rage::Vector4 temp
	{
		x,
		y,
		z,
		1.f // translate
	};
	temp = temp.TransformCoords(mat);
	return Rage::Vector3
	{
		temp.x / temp.w,
		temp.y / temp.w,
		temp.z / temp.w
	};
}

Rage::Vector3 Rage::Vector3::TransformNormal(Rage::Matrix const &mat) const
{
	Rage::Vector4 temp
	{
		x,
		y,
		z,
		0.f // don't translate this one.
	};
	temp = temp.TransformCoords(mat);
	return Rage::Vector3
	{
		temp.x,
		temp.y,
		temp.z
	};
}
