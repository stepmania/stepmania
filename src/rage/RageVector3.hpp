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

	float& operator[](int i)
	{
		switch(i)
		{
			case 0: return x;
			case 1: return y;
			case 2: return z;
			default: return x;
		}
		return x;
	}

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

inline void avg_vec3(Vector3 const& lhs, Vector3 const& rhs, Vector3& res)
{
	res.x= (lhs.x + rhs.x) * .5;
	res.y= (lhs.y + rhs.y) * .5;
	res.z= (lhs.z + rhs.z) * .5;
}

// Little known fact: There are two ways to multiply two vectors:
// Cross product, and dot product.  Because they are both vector
// multiplication, neither of them should use the * operator.
// And combining with an assignment operator is more wrong because it
// forces more operations because the person doing the multiplication
// always wants to put the result in a different vector. -Kyz
inline Vector3 CrossProduct(Vector3 const& lhs, Vector3 const & rhs)
{
	return Rage::Vector3(
		(lhs.y * rhs.z) - (lhs.z * rhs.y),
		(lhs.z * rhs.x) - (lhs.x * rhs.z),
		(lhs.x * rhs.y) - (lhs.y * rhs.x));
}

struct transform // robot in disguise
{
	Vector3 pos;
	Vector3 rot;
	Vector3 zoom;
	// Because I happen to need alpha and glow at the same time as the other
	// parts often. -Kyz
	float alpha;
	float glow;
};

}

#endif
