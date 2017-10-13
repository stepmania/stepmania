#include "RageVector2.hpp"
#include "RageVector4.hpp"
#include <cmath>

Rage::Vector2::Vector2() : x(0), y(0)
{
}

Rage::Vector2::Vector2(float a, float b) : x(a), y(b)
{
}

Rage::Vector2 & Rage::Vector2::operator+=(Rage::Vector2 const & rhs)
{
	x += rhs.x;
	y += rhs.y;
	
	return *this;
}

Rage::Vector2 & Rage::Vector2::operator-=(Rage::Vector2 const & rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	
	return *this;
}

Rage::Vector2 & Rage::Vector2::operator*=(float rhs)
{
	x *= rhs;
	y *= rhs;

	return *this;
}

Rage::Vector2 & Rage::Vector2::operator/=(float rhs)
{
	x /= rhs;
	y /= rhs;

	return *this;
}

Rage::Vector2 Rage::Vector2::GetNormalized() const
{
	float scale = 1.0f / std::sqrt( x * x + y * y );
	return (*this) * scale;
}

Rage::Vector2 Rage::Vector2::TransformCoords(Rage::Matrix const &mat) const
{
	Rage::Vector4 temp
	{
		x,
		y,
		0.f,
		1.f // translate
	};
	temp = temp.TransformCoords(mat);
	return Rage::Vector2
	{
		temp.x / temp.w,
		temp.y / temp.w
	};
}
