#ifndef RAGE_VECTOR_3_HPP_
#define RAGE_VECTOR_3_HPP_

namespace Rage
{
struct Vector3
{
public:
	Vector3();
	Vector3(float a, float b, float c);
		
	// assignment operators
	Vector3& operator += (const Vector3& rhs);
	Vector3& operator -= (const Vector3& rhs);
	Vector3& operator *= (float rhs);
	Vector3& operator /= (float rhs);
	
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
}

#endif
