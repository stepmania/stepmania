#ifndef RageMath_H
#define RageMath_H
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
struct RageMatrix;


void RageVec2Normalize( RageVector2* pOut, const RageVector2* pV );
void RageVec3TransformCoord( RageVector3* pOut, const RageVector3* pV, const RageMatrix* pM );
void RageMatrixIdentity( RageMatrix* pOut );
void RageMatrixMultiply( RageMatrix* pOut, const RageMatrix* pA, const RageMatrix* pB );
void RageMatrixTranslation( RageMatrix* pOut, float x, float y, float z );
void RageMatrixScaling( RageMatrix* pOut, float x, float y, float z );
void RageMatrixRotationX( RageMatrix* pOut, float fTheta );
void RageMatrixRotationY( RageMatrix* pOut, float fTheta );
void RageMatrixRotationZ( RageMatrix* pOut, float fTheta );
void RageMatrixOrthoOffCenterLH( RageMatrix* pOut, float l, float r, float b, float t, float zn, float zf );
void RageMatrixLookAtLH( RageMatrix* pOut, const RageVector3* pEye, const RageVector3* pAt, const RageVector3* pUp ); 
void RageMatrixPerspectiveFovLH( RageMatrix* pOut, float fovy, float Aspect, float zn, float zf );

#endif