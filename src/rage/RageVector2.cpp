#include "RageVector2.hpp"

RageVector2::RageVector2() : x(0), y(0)
{
}

RageVector2::RageVector2(float a, float b) : x(a), y(b)
{
}

RageVector2 & RageVector2::operator+=(RageVector2 const & rhs)
{
	x += rhs.x;
	y += rhs.y;
	
	return *this;
}

RageVector2 & RageVector2::operator-=(RageVector2 const & rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	
	return *this;
}

RageVector2 & RageVector2::operator*=(float rhs)
{
	x *= rhs;
	y *= rhs;

	return *this;
}

RageVector2 & RageVector2::operator/=(float rhs)
{
	x /= rhs;
	y /= rhs;

	return *this;
}
