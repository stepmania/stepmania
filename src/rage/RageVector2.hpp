#ifndef RAGE_VECTOR_2_HPP_
#define RAGE_VECTOR_2_HPP_

struct RageVector2
{
public:
	RageVector2();
	RageVector2(float x1, float y1);

	// assignment operators
	RageVector2& operator += (RageVector2 const & rhs);
	RageVector2& operator -= (RageVector2 const & rhs);
	RageVector2& operator *= (float rhs);
	RageVector2& operator /= (float rhs);

	float x, y;
};

inline bool operator==(RageVector2 const & lhs, RageVector2 const & rhs)
{
	return
		lhs.x == rhs.x &&
		lhs.y == rhs.y;
}

inline bool operator!=(RageVector2 const & lhs, RageVector2 const & rhs)
{
	return !operator==(lhs, rhs);
}

inline RageVector2 operator+(RageVector2 lhs, RageVector2 const & rhs)
{
	lhs += rhs;
	return lhs;
}

inline RageVector2 operator-(RageVector2 lhs, RageVector2 const & rhs)
{
	lhs -= rhs;
	return lhs;
}

inline RageVector2 operator*(RageVector2 lhs, float rhs)
{
	lhs *= rhs;
	return lhs;
}

inline RageVector2 operator/(RageVector2 lhs, float rhs)
{
	lhs /= rhs;
	return lhs;
}

#endif
