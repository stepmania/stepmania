#ifndef RAGE_VECTOR_4_HPP_
#define RAGE_VECTOR_4_HPP_

#include "RageMatrix.hpp"

namespace Rage
{
struct Vector4
{
public:
	Vector4();
	Vector4(float a, float b, float c, float d);
	
	// assignment operators
	Vector4& operator += (Vector4 const & rhs);
	Vector4& operator -= (Vector4 const & rhs);
	Vector4& operator *= (float rhs);
	Vector4& operator /= (float rhs);

	/** @brief Transform the coordinates into a new vector. */
	Vector4 TransformCoords(Matrix const &mat) const;
	
	float x, y, z, w;
};

inline bool operator==(Vector4 const & lhs, Vector4 const & rhs)
{
	return
		lhs.x == rhs.x &&
		lhs.y == rhs.y &&
		lhs.z == rhs.z &&
		lhs.w == rhs.w;
}

inline bool operator!=(Vector4 const & lhs, Vector4 const & rhs)
{
	return !operator==(lhs, rhs);
}

inline Vector4 operator+(Vector4 lhs, Vector4 const & rhs)
{
	lhs += rhs;
	return lhs;
}

inline Vector4 operator-(Vector4 lhs, Vector4 const & rhs)
{
	lhs -= rhs;
	return lhs;
}

inline Vector4 operator*(Vector4 lhs, float rhs)
{
	lhs *= rhs;
	return lhs;
}

inline Vector4 operator/(Vector4 lhs, float rhs)
{
	lhs /= rhs;
	return lhs;
}
}

#endif
