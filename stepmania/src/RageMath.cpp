#include "stdafx.h"
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

#pragma comment(lib, "plib-1.6.0/sg.lib")
#pragma comment(lib, "plib-1.6.0/ul.lib")

void RageVec2Normalize( RageVector2* pOut, const RageVector2* pV )
{
	sgNormalizeVec2( (float*)pOut, (const float*)pV );
}

void RageVec3TransformCoord( RageVector3* pOut, const RageVector3* pV, const RageMatrix* pM )
{
	sgFullXformPnt3 ( (float*)pOut, (float*)pV, pM->m );
}

void RageMatrixIdentity( RageMatrix* pOut )
{
	sgMakeIdentMat4( pOut->m );
}

void RageMatrixMultiply( RageMatrix* pOut, const RageMatrix* pA, const RageMatrix* pB )
{
	sgMultMat4( pOut->m, pB->m, pA->m );
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
