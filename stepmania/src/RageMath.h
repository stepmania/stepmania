#ifndef RAGE_MATH_H
#define RAGE_MATH_H
/*
-----------------------------------------------------------------------------
 File: RageMath.h

 Desc: Math utility functions.  Most of these prototypes 
	match up with the D3DX math functions.  Take a function name, 
	replace "Rage" with "D3DX" and look it up in the D3D SDK docs 
	for details.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#define PI		((float)3.1415926535897932384626433832795)
#define DegreeToRadian( degree ) ((degree) * (PI / 180.0f))
#define RadianToDegree( radian ) ((radian) * (180.0f / PI))


struct RageVector2;
struct RageVector3;
struct RageVector4;
struct RageMatrix;

void RageVec3ClearBounds( RageVector3 &mins, RageVector3 &maxs );
void RageVec3AddToBounds( const RageVector3 &p, RageVector3 &mins, RageVector3 &maxs );

void RageVec2Normalize( RageVector2* pOut, const RageVector2* pV );
void RageVec3Normalize( RageVector3* pOut, const RageVector3* pV );
void RageVec3TransformCoord( RageVector3* pOut, const RageVector3* pV, const RageMatrix* pM );
void RageVec3TransformNormal( RageVector3* pOut, const RageVector3* pV, const RageMatrix* pM );
void RageVec4TransformCoord( RageVector4* pOut, const RageVector4* pV, const RageMatrix* pM );
void RageMatrixIdentity( RageMatrix* pOut );
RageMatrix RageMatrixIdentity();
void RageMatrixMultiply( RageMatrix* pOut, const RageMatrix* pA, const RageMatrix* pB );
void RageMatrixTranslation( RageMatrix* pOut, float x, float y, float z );
void RageMatrixScaling( RageMatrix* pOut, float x, float y, float z );
void RageMatrixRotationX( RageMatrix* pOut, float fTheta );
void RageMatrixRotationY( RageMatrix* pOut, float fTheta );
void RageMatrixRotationZ( RageMatrix* pOut, float fTheta );
RageMatrix RageMatrixRotationX( float fTheta );
RageMatrix RageMatrixRotationY( float fTheta );
RageMatrix RageMatrixRotationZ( float fTheta );
void RageMatrixCommand( CString sCommandString, RageMatrix &mat );
void RageQuatFromHPR(RageVector4* pOut, RageVector3 hpr );
void RageQuatFromPRH(RageVector4* pOut, RageVector3 prh );
void RageMatrixFromQuat( RageMatrix* pOut, const RageVector4 q );
void RageQuatSlerp(RageVector4 *pOut, const RageVector4 &from, const RageVector4 &to, float t);
RageVector4 RageQuatFromH(float theta);
RageVector4 RageQuatFromP(float theta);
RageVector4 RageQuatFromR(float theta);
void RageQuatMultiply( RageVector4* pOut, const RageVector4 &pA, const RageVector4 &pB );
RageMatrix RageLookAt(
	float eyex, float eyey, float eyez,
	float centerx, float centery, float centerz,
	float upx, float upy, float upz );
void RageMatrixAngles( RageMatrix* pOut, const RageVector3 &angles );
void RageMatrixTranspose( RageMatrix* pOut, const RageMatrix* pIn );

#endif
