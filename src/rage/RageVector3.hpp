#ifndef RAGE_VECTOR_3_HPP_
#define RAGE_VECTOR_3_HPP_

struct RageVector3
{
public:
	RageVector3();
	RageVector3(float a, float b, float c);
		
	// assignment operators
	RageVector3& operator += (const RageVector3& rhs);
	RageVector3& operator -= (const RageVector3& rhs);
	RageVector3& operator *= (float rhs);
	RageVector3& operator /= (float rhs);
	
	float x, y, z;
};

#endif

inline bool operator==(RageVector3 const & lhs, RageVector3 const & rhs)
{
	return
		lhs.x == rhs.x &&
		lhs.y == rhs.y &&
		lhs.z == rhs.z;
}

inline bool operator!=(RageVector3 const & lhs, RageVector3 const & rhs)
{
	return !operator==(lhs, rhs);
}

inline RageVector3 operator+(RageVector3 lhs, RageVector3 const & rhs)
{
	lhs += rhs;
	return lhs;
}

inline RageVector3 operator-(RageVector3 lhs, RageVector3 const & rhs)
{
	lhs -= rhs;
	return lhs;
}

inline RageVector3 operator*(RageVector3 lhs, float rhs)
{
	lhs *= rhs;
	return lhs;
}

inline RageVector3 operator/(RageVector3 lhs, float rhs)
{
	lhs /= rhs;
	return lhs;
}
