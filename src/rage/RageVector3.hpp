#ifndef RAGE_VECTOR_3_HPP_
#define RAGE_VECTOR_3_HPP_

#include "RageMatrix.hpp"

namespace Rage
{
struct Vector3
{
public:
	Vector3();
	Vector3(float a, float b, float c);
		
	// assignment operators
	Vector3& operator += (Vector3 const & rhs);
	Vector3& operator -= (Vector3 const & rhs);
	Vector3& operator *= (float rhs);
	Vector3& operator /= (float rhs);

	/** @brief Get the cross product. */
	Vector3& operator *= (Vector3 const & rhs);
	
	/** @brief Get a normalized version of the vector. */
	Vector3 GetNormalized() const;

	/** @brief Get the transformed coordinate version of the vector. */
	Vector3 TransformCoords(Matrix const &mat) const;

	/** @brief Get the transformed normal version of the vector. */
	Vector3 TransformNormal(Matrix const &mat) const;

	float x, y, z;
};

inline bool operator==(Vector3 const & lhs, Vector3 const & rhs)
{
	return
		lhs.x == rhs.x &&
		lhs.y == rhs.y &&
		lhs.z == rhs.z;
}

inline bool operator!=(Vector3 const & lhs, Vector3 const & rhs)
{
	return !operator==(lhs, rhs);
}

inline Vector3 operator+(Vector3 lhs, Vector3 const & rhs)
{
	lhs += rhs;
	return lhs;
}

inline Vector3 operator-(Vector3 lhs, Vector3 const & rhs)
{
	lhs -= rhs;
	return lhs;
}

inline Vector3 operator*(Vector3 lhs, float rhs)
{
	lhs *= rhs;
	return lhs;
}

inline Vector3 operator/(Vector3 lhs, float rhs)
{
	lhs /= rhs;
	return lhs;
}

inline Vector3 operator*(Vector3 lhs, Vector3 const &rhs)
{
	lhs *= rhs;
	return lhs;
}

}

#endif
