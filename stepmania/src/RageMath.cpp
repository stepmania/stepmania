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
#include "D3DX8Math.h"


void RageVec2Normalize( RageVector2* pOut, const RageVector2* pV )
{
	D3DXVec2Normalize( (D3DXVECTOR2*)pOut, (const D3DXVECTOR2*)pV );
}

void RageVec3TransformCoord( RageVector3* pOut, const RageVector3* pV, const RageMatrix* pM )
{
	D3DXVec3TransformCoord( (D3DXVECTOR3*)pOut, (const D3DXVECTOR3*)pV, (const D3DXMATRIX*)pM );
}

void RageMatrixIdentity( RageMatrix* pOut )
{
	D3DXMatrixIdentity( (D3DXMATRIX*)pOut );
}

void RageMatrixMultiply( RageMatrix* pOut, const RageMatrix* pA, const RageMatrix* pB )
{
	D3DXMatrixMultiply( (D3DXMATRIX*)pOut, (const D3DXMATRIX*)pA, (const D3DXMATRIX*)pB );
}

void RageMatrixTranslation( RageMatrix* pOut, float x, float y, float z )
{
	D3DXMatrixTranslation( (D3DXMATRIX*)pOut, x, y, z );
}

void RageMatrixScaling( RageMatrix* pOut, float x, float y, float z )
{
	D3DXMatrixScaling( (D3DXMATRIX*)pOut, x, y, z );
}

void RageMatrixRotationX( RageMatrix* pOut, float theta )
{
	D3DXMatrixRotationX( (D3DXMATRIX*)pOut, theta );
}

void RageMatrixRotationY( RageMatrix* pOut, float theta )
{
	D3DXMatrixRotationY( (D3DXMATRIX*)pOut, theta );
}

void RageMatrixRotationZ( RageMatrix* pOut, float theta )
{
	D3DXMatrixRotationZ( (D3DXMATRIX*)pOut, theta );
}

void RageMatrixOrthoOffCenterLH( RageMatrix* pOut, float l, float r, float b, float t, float zn, float zf )
{
	D3DXMatrixOrthoOffCenterLH( (D3DXMATRIX*)pOut, l, r, b, t, zn, zf );
}

void RageMatrixLookAtLH( RageMatrix* pOut, const RageVector3* pEye, const RageVector3* pAt, const RageVector3* pUp )
{
	D3DXMatrixLookAtLH( (D3DXMATRIX*)pOut, (const D3DXVECTOR3*)pEye, (const D3DXVECTOR3*)pAt, (const D3DXVECTOR3*)pUp );
}

void RageMatrixPerspectiveFovLH( RageMatrix* pOut, float fovy, float Aspect, float zn, float zf )
{
	D3DXMatrixPerspectiveFovLH( (D3DXMATRIX*)pOut, fovy, Aspect, zn, zf );
}


