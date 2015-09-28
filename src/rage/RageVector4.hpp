#ifndef RAGE_VECTOR_4_HPP_
#define RAGE_VECTOR_4_HPP_

struct RageVector4
{
public:
	RageVector4();
	RageVector4(float a, float b, float c, float d);
	
	// assignment operators
	RageVector4& operator += (const RageVector4& rhs);
	RageVector4& operator -= (const RageVector4& rhs);
	RageVector4& operator *= (float rhs);
	RageVector4& operator /= (float rhs);
	
	float x, y, z, w;
};

#endif

inline bool operator==(RageVector4 const & lhs, RageVector4 const & rhs)
{
	return
		lhs.x == rhs.x &&
		lhs.y == rhs.y &&
		lhs.z == rhs.z &&
		lhs.w == rhs.w;
}

inline bool operator!=(RageVector4 const & lhs, RageVector4 const & rhs)
{
	return !operator==(lhs, rhs);
}

inline RageVector4 operator+(RageVector4 lhs, RageVector4 const & rhs)
{
	lhs += rhs;
	return lhs;
}

inline RageVector4 operator-(RageVector4 lhs, RageVector4 const & rhs)
{
	lhs -= rhs;
	return lhs;
}

inline RageVector4 operator*(RageVector4 lhs, float rhs)
{
	lhs *= rhs;
	return lhs;
}

inline RageVector4 operator/(RageVector4 lhs, float rhs)
{
	lhs /= rhs;
	return lhs;
}
