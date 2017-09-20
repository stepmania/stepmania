/*
 * Most of these prototypes match up with the D3DX math functions.  Take a
 * function name, replace "Rage" with "D3DX" and look it up in the D3D SDK
 * docs for details.
 */

#include "global.h"
#include "RageLog.h"
#include "RageMath.hpp"
#include "RageMath.h"
#include "RageMatrix.hpp"
#include "RageVector4.hpp"
#include "RageTypes.h"
#include "RageUtil.hpp"
#include <limits>
#include <array>

using std::vector;

void RageVec3ClearBounds( Rage::Vector3 &mins, Rage::Vector3 &maxs )
{
	float const maxFloat = std::numeric_limits<float>::max();
	mins = Rage::Vector3( maxFloat, maxFloat, maxFloat );
	maxs = mins * -1;
}

void RageVec3AddToBounds( const Rage::Vector3 &p, Rage::Vector3 &mins, Rage::Vector3 &maxs )
{
	using std::max;
	using std::min;
	mins.x = min( mins.x, p.x );
	mins.y = min( mins.y, p.y );
	mins.z = min( mins.z, p.z );
	maxs.x = max( maxs.x, p.x );
	maxs.y = max( maxs.y, p.y );
	maxs.z = max( maxs.z, p.z );
}

#define m00 m[0][0]
#define m01 m[0][1]
#define m02 m[0][2]
#define m03 m[0][3]
#define m10 m[1][0]
#define m11 m[1][1]
#define m12 m[1][2]
#define m13 m[1][3]
#define m20 m[2][0]
#define m21 m[2][1]
#define m22 m[2][2]
#define m23 m[2][3]
#define m30 m[3][0]
#define m31 m[3][1]
#define m32 m[3][2]
#define m33 m[3][3]

void RageMatrixMultiply( Rage::Matrix* pOut, const Rage::Matrix* pA, const Rage::Matrix* pB )
{
//#if defined(_WINDOWS)
//	// <30 cycles for theirs versus >100 for ours.
//	D3DXMatrixMultiply( (D3DMATRIX*)pOut, (D3DMATRIX*)pA, (D3DMATRIX*)pB );
//#else
	const Rage::Matrix &a = *pA;
	const Rage::Matrix &b = *pB;

	*pOut = Rage::Matrix(
		b.m00*a.m00+b.m01*a.m10+b.m02*a.m20+b.m03*a.m30,
		b.m00*a.m01+b.m01*a.m11+b.m02*a.m21+b.m03*a.m31,
		b.m00*a.m02+b.m01*a.m12+b.m02*a.m22+b.m03*a.m32,
		b.m00*a.m03+b.m01*a.m13+b.m02*a.m23+b.m03*a.m33,
		b.m10*a.m00+b.m11*a.m10+b.m12*a.m20+b.m13*a.m30,
		b.m10*a.m01+b.m11*a.m11+b.m12*a.m21+b.m13*a.m31,
		b.m10*a.m02+b.m11*a.m12+b.m12*a.m22+b.m13*a.m32,
		b.m10*a.m03+b.m11*a.m13+b.m12*a.m23+b.m13*a.m33,
		b.m20*a.m00+b.m21*a.m10+b.m22*a.m20+b.m23*a.m30,
		b.m20*a.m01+b.m21*a.m11+b.m22*a.m21+b.m23*a.m31,
		b.m20*a.m02+b.m21*a.m12+b.m22*a.m22+b.m23*a.m32,
		b.m20*a.m03+b.m21*a.m13+b.m22*a.m23+b.m23*a.m33,
		b.m30*a.m00+b.m31*a.m10+b.m32*a.m20+b.m33*a.m30,
		b.m30*a.m01+b.m31*a.m11+b.m32*a.m21+b.m33*a.m31,
		b.m30*a.m02+b.m31*a.m12+b.m32*a.m22+b.m33*a.m32,
		b.m30*a.m03+b.m31*a.m13+b.m32*a.m23+b.m33*a.m33
	);
	// phew!
//#endif
}

/*
 * Return:
 *
 * Rage::MatrixMultiply( pOut, &translate, &scale );
 */

void RageMatrixRotationX( Rage::Matrix* pOut, float theta )
{
	theta *= Rage::PI / 180;

	*pOut = Rage::Matrix::GetIdentity();
	pOut->m[1][1] = Rage::FastCos(theta);
	pOut->m[2][2] = pOut->m[1][1];

	pOut->m[2][1] = Rage::FastSin(theta);
	pOut->m[1][2] = -pOut->m[2][1];
}

void RageMatrixRotationY( Rage::Matrix* pOut, float theta )
{
	theta *= Rage::PI / 180;

	*pOut = Rage::Matrix::GetIdentity();
	pOut->m[0][0] = Rage::FastCos(theta);
	pOut->m[2][2] = pOut->m[0][0];

	pOut->m[0][2] = Rage::FastSin(theta);
	pOut->m[2][0] = -pOut->m[0][2];
}

void RageMatrixRotationZ( Rage::Matrix* pOut, float theta )
{
	theta *= Rage::PI / 180;

	*pOut = Rage::Matrix::GetIdentity();
	pOut->m[0][0] = Rage::FastCos(theta);
	pOut->m[1][1] = pOut->m[0][0];

	pOut->m[0][1] = Rage::FastSin(theta);
	pOut->m[1][0] = -pOut->m[0][1];
}

/* Return Rage::MatrixRotationX(rX) * Rage::MatrixRotationY(rY) * Rage::MatrixRotationZ(rZ)
 * quickly (without actually doing two complete matrix multiplies), by removing the
 * parts of the matrix multiplies that we know will be 0. */
void RageMatrixRotationXYZ( Rage::Matrix* pOut, float rX, float rY, float rZ )
{
	rX *= Rage::PI / 180;
	rY *= Rage::PI / 180;
	rZ *= Rage::PI / 180;

	const float cX = Rage::FastCos(rX);
	const float sX = Rage::FastSin(rX);
	const float cY = Rage::FastCos(rY);
	const float sY = Rage::FastSin(rY);
	const float cZ = Rage::FastCos(rZ);
	const float sZ = Rage::FastSin(rZ);

	/*
	 * X*Y:
	 * Rage::Matrix(
	 *	cY,  sY*sX, sY*cX, 0,
	 *	0,   cX,    -sX,   0,
	 *	-sY, cY*sX, cY*cX, 0,
	 *	0, 0, 0, 1
	 * );
	 *
	 * X*Y*Z:
	 *
	 * Rage::Matrix(
	 *	cZ*cY, cZ*sY*sX+sZ*cX, cZ*sY*cX+sZ*(-sX), 0,
	 *	(-sZ)*cY, (-sZ)*sY*sX+cZ*cX, (-sZ)*sY*cX+cZ*(-sX), 0,
	 *	-sY, cY*sX, cY*cX, 0,
	 *	0, 0, 0, 1
	 * );
	 */

	pOut->m00 = cZ*cY;
	pOut->m01 = cZ*sY*sX+sZ*cX;
	pOut->m02 = cZ*sY*cX+sZ*(-sX);
	pOut->m03 = 0;
	pOut->m10 = (-sZ)*cY;
	pOut->m11 = (-sZ)*sY*sX+cZ*cX;
	pOut->m12 = (-sZ)*sY*cX+cZ*(-sX);
	pOut->m13 = 0;
	pOut->m20 = -sY;
	pOut->m21 = cY*sX;
	pOut->m22 = cY*cX;
	pOut->m23 = 0;
	pOut->m30 = 0;
	pOut->m31 = 0;
	pOut->m32 = 0;
	pOut->m33 = 1;
}

void RageAARotate(Rage::Vector3* inret, Rage::Vector3 const* axis, float angle)
{
	float ha= angle/2.0f;
	float ca2= Rage::FastCos(ha);
	float sa2= Rage::FastSin(ha);
	Rage::Vector4 quat(axis->x * sa2, axis->y * sa2, axis->z * sa2, ca2);
	Rage::Vector4 quatc(-quat.x, -quat.y, -quat.z, ca2);
	Rage::Vector4 point(inret->x, inret->y, inret->z, 0.0f);
	RageQuatMultiply(&point, quat, point);
	RageQuatMultiply(&point, point, quatc);
	inret->x= point.x;
	inret->y= point.y;
	inret->z= point.z;
}

void RageQuatMultiply( Rage::Vector4* pOut, const Rage::Vector4 &pA, const Rage::Vector4 &pB )
{
	Rage::Vector4 out;
	out.x = pA.w * pB.x + pA.x * pB.w + pA.y * pB.z - pA.z * pB.y;
	out.y = pA.w * pB.y + pA.y * pB.w + pA.z * pB.x - pA.x * pB.z;
	out.z = pA.w * pB.z + pA.z * pB.w + pA.x * pB.y - pA.y * pB.x;
	out.w = pA.w * pB.w - pA.x * pB.x - pA.y * pB.y - pA.z * pB.z;

	float dist, square;

	square = out.x * out.x + out.y * out.y + out.z * out.z + out.w * out.w;

	if (square > 0.0)
    dist = 1.0f / std::sqrt(square);
	else dist = 1;

	out.x *= dist;
	out.y *= dist;
	out.z *= dist;
	out.w *= dist;

	*pOut = out;
}

Rage::Vector4 RageQuatFromH(float theta )
{
	theta *= Rage::PI / 180.0f;
	theta /= 2.0f;
	theta *= -1;
	const float c = Rage::FastCos(theta);
	const float s = Rage::FastSin(theta);

	return Rage::Vector4(0, s, 0, c);
}

Rage::Vector4 RageQuatFromP(float theta )
{
	theta *= Rage::PI / 180.0f;
	theta /= 2.0f;
	theta *= -1;
	const float c = Rage::FastCos(theta);
	const float s = Rage::FastSin(theta);

	return Rage::Vector4(s, 0, 0, c);
}

Rage::Vector4 RageQuatFromR(float theta )
{
	theta *= Rage::PI / 180.0f;
	theta /= 2.0f;
	theta *= -1;
	const float c = Rage::FastCos(theta);
	const float s = Rage::FastSin(theta);

	return Rage::Vector4(0, 0, s, c);
}


/* Math from http://www.gamasutra.com/features/19980703/quaternions_01.htm . */

/* prh.xyz -> heading, pitch, roll */
void RageQuatFromHPR(Rage::Vector4* pOut, Rage::Vector3 hpr )
{
	hpr *= Rage::PI;
	hpr /= 180.0f;
	hpr /= 2.0f;

	const float sX = Rage::FastSin(hpr.x);
	const float cX = Rage::FastCos(hpr.x);
	const float sY = Rage::FastSin(hpr.y);
	const float cY = Rage::FastCos(hpr.y);
	const float sZ = Rage::FastSin(hpr.z);
	const float cZ = Rage::FastCos(hpr.z);

	pOut->w = cX * cY * cZ + sX * sY * sZ;
	pOut->x = sX * cY * cZ - cX * sY * sZ;
	pOut->y = cX * sY * cZ + sX * cY * sZ;
	pOut->z = cX * cY * sZ - sX * sY * cZ;
}

/*
 * Screen orientatoin:  the "floor" is the XZ plane, and Y is height; in other
 * words, the screen is the XY plane and negative Z goes into it.
 */

/* prh.xyz -> pitch, roll, heading */
void RageQuatFromPRH(Rage::Vector4* pOut, Rage::Vector3 prh )
{
	prh *= Rage::PI;
	prh /= 180.0f;
	prh /= 2.0f;

	/* Set cX to the cosine of the angle we want to rotate on the X axis,
	 * and so on.  Here, hpr.z (roll) rotates on the Z axis, hpr.x (heading)
	 * on Y, and hpr.y (pitch) on X. */
	const float sX = Rage::FastSin(prh.y);
	const float cX = Rage::FastCos(prh.y);
	const float sY = Rage::FastSin(prh.x);
	const float cY = Rage::FastCos(prh.x);
	const float sZ = Rage::FastSin(prh.z);
	const float cZ = Rage::FastCos(prh.z);

	pOut->w = cX * cY * cZ + sX * sY * sZ;
	pOut->x = sX * cY * cZ - cX * sY * sZ;
	pOut->y = cX * sY * cZ + sX * cY * sZ;
	pOut->z = cX * cY * sZ - sX * sY * cZ;
}

void RageMatrixFromQuat( Rage::Matrix* pOut, const Rage::Vector4 q )
{
	float xx = q.x * (q.x + q.x);
	float xy = q.x * (q.y + q.y);
	float xz = q.x * (q.z + q.z);

	float wx = q.w * (q.x + q.x);
	float wy = q.w * (q.y + q.y);
	float wz = q.w * (q.z + q.z);

	float yy = q.y * (q.y + q.y);
	float yz = q.y * (q.z + q.z);

	float zz = q.z * (q.z + q.z);
	// careful.  The param order is row-major, which is the
	// transpose of the order shown in the OpenGL docs.
	*pOut = Rage::Matrix(
		1-(yy+zz), xy+wz,     xz-wy,     0,
		xy-wz,     1-(xx+zz), yz+wx,     0,
		xz+wy,     yz-wx,     1-(xx+yy), 0,
		0,         0,         0,         1 );
}

void RageQuatSlerp(Rage::Vector4 *pOut, const Rage::Vector4 &from, const Rage::Vector4 &to, float t)
{
	float to1[4];

	// calc cosine
	float cosom = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;

	// adjust signs (if necessary)
	if ( cosom < 0 )
	{
		cosom = -cosom;
		to1[0] = - to.x;
		to1[1] = - to.y;
		to1[2] = - to.z;
		to1[3] = - to.w;
	}
	else
	{
		to1[0] = to.x;
		to1[1] = to.y;
		to1[2] = to.z;
		to1[3] = to.w;
	}

	// calculate coefficients
	float scale0, scale1;
	if ( cosom < 0.9999f )
	{
		// standard case (slerp)
		float omega = std::acos(cosom);
		float sinom = Rage::FastSin(omega);
		scale0 = Rage::FastSin((1.0f - t) * omega) / sinom;
		scale1 = Rage::FastSin(t * omega) / sinom;
	}
	else
	{
		// "from" and "to" quaternions are very close
		//  ... so we can do a linear interpolation
		scale0 = 1.0f - t;
		scale1 = t;
	}
	// calculate final values
	pOut->x = scale0 * from.x + scale1 * to1[0];
	pOut->y = scale0 * from.y + scale1 * to1[1];
	pOut->z = scale0 * from.z + scale1 * to1[2];
	pOut->w = scale0 * from.w + scale1 * to1[3];
}

Rage::Matrix RageLookAt(
	float eyex, float eyey, float eyez,
	float centerx, float centery, float centerz,
	float upx, float upy, float upz )
{
	Rage::Vector3 Z(eyex - centerx, eyey - centery, eyez - centerz);
	Z = Z.GetNormalized();

	Rage::Vector3 Y(upx, upy, upz);

	Rage::Vector3 X(
		 Y.y * Z.z - Y.z * Z.y,
		-Y.x * Z.z + Y.z * Z.x,
		 Y.x * Z.y - Y.y * Z.x);

	Y = Rage::Vector3(
		 Z.y * X.z - Z.z * X.y,
		 -Z.x * X.z + Z.z * X.x,
		 Z.x * X.y - Z.y * X.x );

	X = X.GetNormalized();
	Y = Y.GetNormalized();

	Rage::Matrix mat(
		X.x, Y.x, Z.x, 0,
		X.y, Y.y, Z.y, 0,
		X.z, Y.z, Z.z, 0,
		0,    0,    0,    1 );

	auto mat2 = Rage::Matrix::GetTranslation(-eyex, -eyey, -eyez);

	Rage::Matrix ret;
	RageMatrixMultiply(&ret, &mat, &mat2);

	return ret;
}

void RageMatrixAngles( Rage::Matrix* pOut, const Rage::Vector3 &angles )
{
	const Rage::Vector3 angles_radians( angles * 2 * Rage::PI / 360 );

	const float sy = Rage::FastSin( angles_radians.z );
	const float cy = Rage::FastCos( angles_radians.z );
	const float sp = Rage::FastSin( angles_radians.y );
	const float cp = Rage::FastCos( angles_radians.y );
	const float sr = Rage::FastSin( angles_radians.x );
	const float cr = Rage::FastCos( angles_radians.x );

	*pOut = Rage::Matrix::GetIdentity();


	// matrix = (Z * Y) * X
	pOut->m[0][0] = cp*cy;
	pOut->m[0][1] = cp*sy;
	pOut->m[0][2] = -sp;
	pOut->m[1][0] = sr*sp*cy+cr*-sy;
	pOut->m[1][1] = sr*sp*sy+cr*cy;
	pOut->m[1][2] = sr*cp;
	pOut->m[2][0] = (cr*sp*cy+-sr*-sy);
	pOut->m[2][1] = (cr*sp*sy+-sr*cy);
	pOut->m[2][2] = cr*cp;
}

void RageMatrixTranspose( Rage::Matrix* pOut, const Rage::Matrix* pIn )
{
	for( int i=0; i<4; i++)
		for( int j=0; j<4; j++)
			pOut->m[j][i] = pIn->m[i][j];
}

static const unsigned int sine_table_size= 1024;
static const unsigned int sine_index_mod= sine_table_size * 2;
static const double sine_table_index_mult= static_cast<double>(sine_index_mod) / (Rage::PI * 2);
static float sine_table[sine_table_size];
struct sine_initter
{
	sine_initter()
	{
		for(unsigned int i= 0; i < sine_table_size; ++i)
		{
			float angle= Rage::scale(i + 0.f, 0.f, sine_table_size + 0.f, 0.f, Rage::PI);
			sine_table[i]= sinf(angle);
		}
	}
};
static sine_initter sinner;

float RageFastSin(float angle)
{
	if(angle == 0) { return 0; }
	float index= angle * sine_table_index_mult;
	int first_index= static_cast<int>(index);
	int second_index= (first_index + 1) % sine_index_mod;
	float remainder= index - first_index;
	first_index%= sine_index_mod;
	float first= 0.0f;
	float second= 0.0f;
#define SET_SAMPLE(sample) \
	if(sample##_index >= sine_table_size) \
	{ \
		sample= -sine_table[sample##_index - sine_table_size]; \
	} \
	else \
	{ \
		sample= sine_table[sample##_index]; \
	}
	SET_SAMPLE(first);
	SET_SAMPLE(second);
#undef SET_SAMPLE
	float result= Rage::lerp(remainder, first, second);
	return result;
}

float RageFastCos( float x )
{
	return RageFastSin( x + 0.5f * Rage::PI );
}

float RageQuadratic::Evaluate( float fT ) const
{
	// optimized (m_fA * fT*fT*fT) + (m_fB * fT*fT) + (m_fC * fT) + m_fD;
	return ((m_fA*fT + m_fB)*fT + m_fC)*fT + m_fD;
}

void RageQuadratic::SetFromBezier( float fX1, float fX2, float fX3, float fX4 )
{
	m_fD = fX1;
	m_fC = 3.0f * (fX2 - fX1);
	m_fB = 3.0f * (fX3 - fX2) - m_fC;
	m_fA = fX4 - fX1 - m_fC - m_fB;
}

void RageQuadratic::GetBezier( float &fX1, float &fX2, float &fX3, float &fX4 ) const
{
	fX1 = m_fD;
	fX2 = m_fD + m_fC/3.0f;
	fX3 = m_fD + 2*m_fC/3.0f + m_fB/3.0f;
	fX4 = m_fD + m_fC + m_fB + m_fA;
}

/* Cubic polynomial interpolation.  SetFromCubicPoly(-1, 0, 1, 2); Evaluate(p) will
 * interpolate between 0 and 1. */
void RageQuadratic::SetFromCubic( float fX1, float fX2, float fX3, float fX4 )
{
	m_fA = -1.0f/6.0f*fX1 + +3.0f/6.0f*fX2 + -3.0f/6.0f*fX3 + 1.0f/6.0f*fX4;
	m_fB =  3.0f/6.0f*fX1 + -6.0f/6.0f*fX2 +  3.0f/6.0f*fX3;
	m_fC = -2.0f/6.0f*fX1 + -3.0f/6.0f*fX2 +            fX3 + -1.0f/6.0f*fX4;
	m_fD =                             fX2;
}

float RageQuadratic::GetSlope( float fT ) const
{
	return 3*m_fA*fT*fT + 2*m_fB*fT + m_fC;
}

void RageBezier2D::Evaluate( float fT, float *pX, float *pY ) const
{
	*pX = m_X.Evaluate( fT );
	*pY = m_Y.Evaluate( fT );
}

float RageBezier2D::EvaluateYFromX( float fX ) const
{
	/* Quickly approximate T using Newton-Raphelson successive optimization (see
	 * http://www.tinaja.com/text/bezmath.html).  This usually finds T within an
	 * acceptable error margin in a few steps. */
	float fT = Rage::scale( fX, m_X.GetBezierStart(), m_X.GetBezierEnd(), 0.f, 1.f );
	// Don't try more than 100 times, the curve might be a bit nonsensical. -Kyz
	for(int i= 0; i < 100; ++i)
	{
		float fGuessedX = m_X.Evaluate( fT );
		float fError = fX-fGuessedX;

		/* If our guess is good enough, evaluate the result Y and return. */
		if( unlikely(fabsf(fError) < 0.0001f) )
			return m_Y.Evaluate( fT );

		float fSlope = m_X.GetSlope( fT );
		fT += fError/fSlope;
	}
	return m_Y.Evaluate( fT );
}

void RageBezier2D::SetFromBezier(
		float fC1X, float fC1Y, float fC2X, float fC2Y,
		float fC3X, float fC3Y, float fC4X, float fC4Y )
{
	m_X.SetFromBezier( fC1X, fC2X, fC3X, fC4X );
	m_Y.SetFromBezier( fC1Y, fC2Y, fC3Y, fC4Y );
}

// A line equation of the form "Ax + By = C".
// Start and end points are irrelevant to what it will be used for. -Kyz
struct abc_line
{
	float a;
	float b;
	float c;
};

struct abc_poly
{
	vector<abc_line> edges;
	float centroid_x;
	float centroid_y;
};

void points_to_abc_line(Rage::Vector2 const& pa, Rage::Vector2 const& pb, abc_line& line)
{
	Rage::Vector2 v= (pb - pa).GetNormalized();
	if(std::isnan(v.x) || !std::isfinite(v.x))
	{
		line.a= 0;
		line.b= 0;
		line.c= 0;
		return;
	}
	line.a= v.y;
	line.b= v.x;
	line.c= (v.y * pa.x) + (v.x * pa.y);
}

bool point_below_abc_line(float x, float y, abc_line const& line)
{
	float point_c= (x * line.a) + (y * line.b);
	return point_c > line.c;
}

void points_to_abc_poly(vector<Rage::Vector2> const& points, abc_poly& poly)
{
	poly.centroid_x= points[0].x;
	poly.centroid_y= points[0].y;
	size_t num_points= points.size();
	poly.edges.resize(num_points);
	for(size_t p= 1; p < num_points; ++p)
	{
		poly.centroid_x+= points[p].x;
		poly.centroid_y+= points[p].y;
		points_to_abc_line(points[p-1], points[p], poly.edges[p-1]);
	}
	points_to_abc_line(points[num_points-1], points[0], poly.edges[num_points-1]);
	float npr= 1.f / static_cast<float>(num_points);
	poly.centroid_x*= npr;
	poly.centroid_y*= npr;
}

bool point_inside_poly(float x, float y, std::vector<Rage::Vector2> const& poly)
{
	abc_poly conv_poly;
	points_to_abc_poly(poly, conv_poly);
	for(auto&& edge : conv_poly.edges)
	{
		if(point_below_abc_line(x, y, edge) != point_below_abc_line(conv_poly.centroid_x, conv_poly.centroid_y, edge))
		{
			return false;
		}
	}
	return true;
}

#include "LuaBinding.h"

struct LunaRageQuadratic : Luna<RageQuadratic>
{
	static int evaluate(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->Evaluate(FArg(1)));
		return 1;
	}
	static int get_bezier(T* p, lua_State* L)
	{
		float a, b, c, d;
		p->GetBezier(a, b, c, d);
		lua_pushnumber(L, a);
		lua_pushnumber(L, b);
		lua_pushnumber(L, c);
		lua_pushnumber(L, d);
		return 4;
	}
	static int get_bezier_end(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->GetBezierEnd());
		return 1;
	}
	static int get_bezier_start(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->GetBezierStart());
		return 1;
	}
	static int get_slope(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->GetSlope(FArg(1)));
		return 1;
	}
	static int set_from_bezier(T* p, lua_State* L)
	{
		p->SetFromBezier(FArg(1), FArg(2), FArg(3), FArg(4));
		COMMON_RETURN_SELF;
	}
	static int set_from_cubic(T* p, lua_State* L)
	{
		p->SetFromCubic(FArg(1), FArg(2), FArg(3), FArg(4));
		COMMON_RETURN_SELF;
	}
	LunaRageQuadratic()
	{
		ADD_METHOD(evaluate);
		ADD_METHOD(get_bezier);
		ADD_METHOD(get_bezier_end);
		ADD_METHOD(get_bezier_start);
		ADD_METHOD(get_slope);
		ADD_METHOD(set_from_bezier);
		ADD_METHOD(set_from_cubic);
	}
};
LUA_REGISTER_CLASS(RageQuadratic);

struct LunaRageBezier2D : Luna<RageBezier2D>
{
	static int evaluate(T* p, lua_State* L)
	{
		float x, y;
		p->Evaluate(FArg(1), &x, &y);
		lua_pushnumber(L, x);
		lua_pushnumber(L, y);
		return 2;
	}
	static int evaluate_y_from_x(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->EvaluateYFromX(FArg(1)));
		return 1;
	}
	static int get_x(T* p, lua_State* L)
	{
		p->get_x().PushSelf(L);
		return 1;
	}
	static int get_y(T* p, lua_State* L)
	{
		p->get_y().PushSelf(L);
		return 1;
	}
	static int set_from_bezier(T* p, lua_State* L)
	{
		p->SetFromBezier(FArg(1), FArg(2), FArg(3), FArg(4), FArg(5), FArg(6), FArg(7), FArg(8));
		COMMON_RETURN_SELF;
	}
	static int destroy(T* p, lua_State* L)
	{
		Rage::safe_delete(p);
		return 0;
	}
	LunaRageBezier2D()
	{
		ADD_METHOD(destroy);
		ADD_METHOD(evaluate);
		ADD_METHOD(evaluate_y_from_x);
		ADD_METHOD(get_x);
		ADD_METHOD(get_y);
		ADD_METHOD(set_from_bezier);
	}
};
LUA_REGISTER_CLASS(RageBezier2D);

int LuaFunc_create_bezier(lua_State* L);
int LuaFunc_create_bezier(lua_State* L)
{
	RageBezier2D* bezier= new RageBezier2D;
	bezier->PushSelf(L);
	return 1;
}
LUAFUNC_REGISTER_COMMON(create_bezier);

/*
 * Copyright (c) 2001-2006 Chris Danford, Glenn Maynard
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
