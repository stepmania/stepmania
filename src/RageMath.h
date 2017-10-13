/* RageMath - vector/matrix math utilities. */

#ifndef RAGE_MATH_H
#define RAGE_MATH_H

struct lua_State;

namespace Rage
{
	struct Vector2;
	struct Vector3;
	struct Vector4;
	struct Matrix;
}

void RageVec3ClearBounds( Rage::Vector3 &mins, Rage::Vector3 &maxs );
void RageVec3AddToBounds( const Rage::Vector3 &p, Rage::Vector3 &mins, Rage::Vector3 &maxs );

// pOut = pB * pA
void RageMatrixMultiply( Rage::Matrix* pOut, const Rage::Matrix* pA, const Rage::Matrix* pB );
void RageMatrixRotationX( Rage::Matrix* pOut, float fTheta );
void RageMatrixRotationY( Rage::Matrix* pOut, float fTheta );
void RageMatrixRotationZ( Rage::Matrix* pOut, float fTheta );
void RageMatrixRotationXYZ( Rage::Matrix* pOut, float rX, float rY, float rZ );
void RageAARotate(Rage::Vector3* inret, Rage::Vector3 const* axis, float angle);
void RageQuatFromHPR(Rage::Vector4* pOut, Rage::Vector3 hpr );
void RageQuatFromPRH(Rage::Vector4* pOut, Rage::Vector3 prh );
void RageMatrixFromQuat( Rage::Matrix* pOut, const Rage::Vector4 q );
void RageQuatSlerp(Rage::Vector4 *pOut, const Rage::Vector4 &from, const Rage::Vector4 &to, float t);
Rage::Vector4 RageQuatFromH(float theta);
Rage::Vector4 RageQuatFromP(float theta);
Rage::Vector4 RageQuatFromR(float theta);
void RageQuatMultiply( Rage::Vector4* pOut, const Rage::Vector4 &pA, const Rage::Vector4 &pB );
Rage::Matrix RageLookAt(
	float eyex, float eyey, float eyez,
	float centerx, float centery, float centerz,
	float upx, float upy, float upz );
void RageMatrixAngles( Rage::Matrix* pOut, const Rage::Vector3 &angles );

class RageQuadratic
{
public:
	void SetFromBezier( float fC1, float fC2, float fC3, float fC4 );
	void GetBezier( float &fC1, float &fC2, float &fC3, float &fC4 ) const;

	void SetFromCubic( float fX1, float fX2, float fX3, float fX4 );

	float Evaluate( float fT ) const;
	float GetSlope( float fT ) const;

	/* Equivalent to Evaluate(0.0f) and Evaluate(1.0f), but faster: */
	float GetBezierStart() const { return m_fD; }
	float GetBezierEnd() const { return m_fA + m_fB + m_fC + m_fD; }

	void PushSelf(lua_State* L);
private:
	float m_fA, m_fB, m_fC, m_fD;
};

class RageBezier2D
{
public:
	void SetFromBezier( float fC1X, float fC2X, float fC3X, float fC4X,
			    float fC1Y, float fC2Y, float fC3Y, float fC4Y );

	void Evaluate( float fT, float *pX, float *pY ) const;
	float EvaluateYFromX( float fX ) const;

	RageQuadratic& get_x() { return m_X; }
	RageQuadratic& get_y() { return m_Y; }
	void PushSelf(lua_State* L);
private:
	RageQuadratic m_X;
	RageQuadratic m_Y;
};

bool point_inside_poly(float x, float y, std::vector<Rage::Vector2> const& poly);

#endif

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
