#include "global.h"
/*
-----------------------------------------------------------------------------
 File: RageMath

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Peter S. May (GetHashForString implementation)
-----------------------------------------------------------------------------
*/

#include "RageMath.h"
#include "RageTypes.h"
#include <math.h>

void RageVec2Normalize( RageVector2* pOut, const RageVector2* pV )
{
	float scale = 1.0f / sqrtf( pV->x*pV->x + pV->y*pV->y );
	pOut->x = pV->x * scale;
	pOut->y = pV->y * scale;
}

void RageVec3TransformCoord( RageVector3* pOut, const RageVector3* pV, const RageMatrix* pM )
{
	RageVector4 temp( pV->x, pV->y, pV->z, 1.0f );
	RageVec4TransformCoord( &temp, &temp, pM );
	*pOut = RageVector3( temp.x/temp.w, temp.y/temp.w, temp.z/temp.w );
}

void RageVec4TransformCoord( RageVector4* pOut, const RageVector4* pV, const RageMatrix* pM )
{
    const RageMatrix &a = *pM;
    const RageVector4 &v = *pOut;
	*pOut = RageVector4(
		a.m00*v.x+a.m10*v.y+a.m20*v.z+a.m30*v.w,
		a.m01*v.x+a.m11*v.y+a.m21*v.z+a.m31*v.w,
		a.m02*v.x+a.m12*v.y+a.m22*v.z+a.m32*v.w,
		a.m03*v.x+a.m13*v.y+a.m23*v.z+a.m33*v.w );
}

void RageMatrixIdentity( RageMatrix* pOut )
{
	*pOut = RageMatrix(
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1 );
}

void RageMatrixMultiply( RageMatrix* pOut, const RageMatrix* pA, const RageMatrix* pB )
{
    const RageMatrix &a = *pA;
    const RageMatrix &b = *pB;

	*pOut = RageMatrix(
		a.m00*b.m00+a.m01*b.m10+a.m02*b.m20+a.m03*b.m30,
		a.m00*b.m01+a.m01*b.m11+a.m02*b.m21+a.m03*b.m31,
		a.m00*b.m02+a.m01*b.m12+a.m02*b.m22+a.m03*b.m32,
		a.m00*b.m03+a.m01*b.m13+a.m02*b.m23+a.m03*b.m33,
		a.m10*b.m00+a.m11*b.m10+a.m12*b.m20+a.m13*b.m30,
		a.m10*b.m01+a.m11*b.m11+a.m12*b.m21+a.m13*b.m31,
		a.m10*b.m02+a.m11*b.m12+a.m12*b.m22+a.m13*b.m32,
		a.m10*b.m03+a.m11*b.m13+a.m12*b.m23+a.m13*b.m33,
		a.m20*b.m00+a.m21*b.m10+a.m22*b.m20+a.m23*b.m30,
		a.m20*b.m01+a.m21*b.m11+a.m22*b.m21+a.m23*b.m31,
		a.m20*b.m02+a.m21*b.m12+a.m22*b.m22+a.m23*b.m32,
		a.m20*b.m03+a.m21*b.m13+a.m22*b.m23+a.m23*b.m33,
		a.m30*b.m00+a.m31*b.m10+a.m32*b.m20+a.m33*b.m30,
		a.m30*b.m01+a.m31*b.m11+a.m32*b.m21+a.m33*b.m31,
		a.m30*b.m02+a.m31*b.m12+a.m32*b.m22+a.m33*b.m32,
		a.m30*b.m03+a.m31*b.m13+a.m32*b.m23+a.m33*b.m33 
	);
	// phew!
}

void RageMatrixTranslation( RageMatrix* pOut, float x, float y, float z )
{
	RageMatrixIdentity(pOut);
	pOut->m[3][0] = x;
	pOut->m[3][1] = y;
	pOut->m[3][2] = z;
}

void RageMatrixScaling( RageMatrix* pOut, float x, float y, float z )
{
	RageMatrixIdentity(pOut);
	pOut->m[0][0] = x;
	pOut->m[1][1] = y;
	pOut->m[2][2] = z;
}

void RageMatrixRotationX( RageMatrix* pOut, float theta )
{ /* UNTESTED */
	RageMatrixIdentity(pOut);
	pOut->m[1][1] = cosf(theta);
	pOut->m[2][2] = pOut->m[1][1];

	pOut->m[2][1] = sinf(theta);
	pOut->m[1][2] = -pOut->m[2][1];
}

void RageMatrixRotationY( RageMatrix* pOut, float theta )
{ /* UNTESTED */
	RageMatrixIdentity(pOut);
	pOut->m[0][0] = cosf(theta);
	pOut->m[2][2] = pOut->m[0][0];

	pOut->m[0][2] = sinf(theta);
	pOut->m[2][0] = -pOut->m[0][2];
}

void RageMatrixRotationZ( RageMatrix* pOut, float theta )
{
	RageMatrixIdentity(pOut);
	pOut->m[0][0] = cosf(theta);
	pOut->m[1][1] = pOut->m[0][0];

	pOut->m[0][1] = sinf(theta);
	pOut->m[1][0] = -pOut->m[0][1];
}
