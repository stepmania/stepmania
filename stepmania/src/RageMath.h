#ifndef RAGE_MATH_H
#define RAGE_MATH_H
/*
-----------------------------------------------------------------------------
 File: RageMath.h

 Desc: .

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

struct RageVector2;
struct RageVector3;
struct RageVector4;
struct RageMatrix;

//#include "plib-1.6.0/sg.h"

void RageVec2Normalize( RageVector2* pOut, const RageVector2* pV );
void RageVec3TransformCoord( RageVector3* pOut, const RageVector3* pV, const RageMatrix* pM );
void RageVec4TransformCoord( RageVector4* pOut, const RageVector4* pV, const RageMatrix* pM );
void RageMatrixIdentity( RageMatrix* pOut );
void RageMatrixMultiply( RageMatrix* pOut, const RageMatrix* pA, const RageMatrix* pB );
void RageMatrixTranslation( RageMatrix* pOut, float x, float y, float z );
void RageMatrixScaling( RageMatrix* pOut, float x, float y, float z );
void RageMatrixRotationX( RageMatrix* pOut, float fTheta );
void RageMatrixRotationY( RageMatrix* pOut, float fTheta );
void RageMatrixRotationZ( RageMatrix* pOut, float fTheta );

#endif
