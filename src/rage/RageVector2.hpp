#ifndef RAGE_VECTOR_2_HPP_
#define RAGE_VECTOR_2_HPP_

namespace Rage
{
struct Vector2
{
public:
	Vector2();
	Vector2(float x1, float y1);

	// assignment operators
	Vector2& operator += (Vector2 const & rhs);
	Vector2& operator -= (Vector2 const & rhs);
	Vector2& operator *= (float rhs);
	Vector2& operator /= (float rhs);

	/** @brief Get a normalized version of the vector. 
	 *
	 * For documentation support, look into D3DXVec2Normalize. */
	Vector2 GetNormalized() const;
	
	float x, y;
};

inline bool operator==(Vector2 const & lhs, Vector2 const & rhs)
{
	return
		lhs.x == rhs.x &&
		lhs.y == rhs.y;
}

inline bool operator!=(Vector2 const & lhs, Vector2 const & rhs)
{
	return !operator==(lhs, rhs);
}

inline Vector2 operator+(Vector2 lhs, Vector2 const & rhs)
{
	lhs += rhs;
	return lhs;
}

inline Vector2 operator-(Vector2 lhs, Vector2 const & rhs)
{
	lhs -= rhs;
	return lhs;
}

inline Vector2 operator*(Vector2 lhs, float rhs)
{
	lhs *= rhs;
	return lhs;
}

inline Vector2 operator/(Vector2 lhs, float rhs)
{
	lhs /= rhs;
	return lhs;
}
}

#endif
