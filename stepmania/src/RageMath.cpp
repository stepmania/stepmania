#include "global.h"
#include "RageMath.h"
#include "RageTypes.h"
#include "RageUtil.h"
#include "RageDisplay.h"
#include "RageLog.h"
#include "arch/Dialog/Dialog.h"

void RageVec3ClearBounds( RageVector3 &mins, RageVector3 &maxs )
{
	mins = RageVector3( 999999, 999999, 999999 );
	maxs = mins * -1;
}

void RageVec3AddToBounds( const RageVector3 &p, RageVector3 &mins, RageVector3 &maxs )
{
	mins.x = min( mins.x, p.x );
	mins.y = min( mins.y, p.y );
	mins.z = min( mins.z, p.z );
	maxs.x = max( maxs.x, p.x );
	maxs.y = max( maxs.y, p.y );
	maxs.z = max( maxs.z, p.z );
}

void RageVec2Normalize( RageVector2* pOut, const RageVector2* pV )
{
	float scale = 1.0f / sqrtf( pV->x*pV->x + pV->y*pV->y );
	pOut->x = pV->x * scale;
	pOut->y = pV->y * scale;
}

void RageVec3Normalize( RageVector3* pOut, const RageVector3* pV )
{
	float scale = 1.0f / sqrtf( pV->x*pV->x + pV->y*pV->y + pV->z*pV->z );
	pOut->x = pV->x * scale;
	pOut->y = pV->y * scale;
	pOut->z = pV->z * scale;
}

void RageVec3TransformCoord( RageVector3* pOut, const RageVector3* pV, const RageMatrix* pM )
{
	RageVector4 temp( pV->x, pV->y, pV->z, 1.0f );	// translate
	RageVec4TransformCoord( &temp, &temp, pM );
	*pOut = RageVector3( temp.x/temp.w, temp.y/temp.w, temp.z/temp.w );
}

void RageVec3TransformNormal( RageVector3* pOut, const RageVector3* pV, const RageMatrix* pM )
{
	RageVector4 temp( pV->x, pV->y, pV->z, 0.0f );	// don't translate
	RageVec4TransformCoord( &temp, &temp, pM );
	*pOut = RageVector3( temp.x, temp.y, temp.z );
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

void RageVec4TransformCoord( RageVector4* pOut, const RageVector4* pV, const RageMatrix* pM )
{
    const RageMatrix &a = *pM;
    const RageVector4 &v = *pV;
	*pOut = RageVector4(
		a.m00*v.x+a.m10*v.y+a.m20*v.z+a.m30*v.w,
		a.m01*v.x+a.m11*v.y+a.m21*v.z+a.m31*v.w,
		a.m02*v.x+a.m12*v.y+a.m22*v.z+a.m32*v.w,
		a.m03*v.x+a.m13*v.y+a.m23*v.z+a.m33*v.w );
}

RageMatrix::RageMatrix( float v00, float v01, float v02, float v03,
						float v10, float v11, float v12, float v13,
						float v20, float v21, float v22, float v23,
						float v30, float v31, float v32, float v33 )
{
	m00=v00; m01=v01; m02=v02; m03=v03;
	m10=v10; m11=v11; m12=v12; m13=v13;
	m20=v20; m21=v21; m22=v22; m23=v23;
	m30=v30; m31=v31; m32=v32; m33=v33;
}

void RageMatrixIdentity( RageMatrix* pOut )
{
	*pOut = RageMatrix(
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1 );
}

RageMatrix RageMatrix::GetTranspose() const
{
	return RageMatrix(m00,m10,m20,m30,m01,m11,m21,m31,m02,m12,m22,m32,m03,m13,m23,m33);
}

void RageMatrixMultiply( RageMatrix* pOut, const RageMatrix* pA, const RageMatrix* pB )
{
//#if defined(_WINDOWS) || defined(_XBOX)
//	// <30 cycles for theirs versus >100 for ours.
//	D3DXMatrixMultiply( (D3DMATRIX*)pOut, (D3DMATRIX*)pA, (D3DMATRIX*)pB );
//#else
    const RageMatrix &a = *pA;
    const RageMatrix &b = *pB;

	*pOut = RageMatrix(
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
{
	theta *= PI/180;

	RageMatrixIdentity(pOut);
	pOut->m[1][1] = cosf(theta);
	pOut->m[2][2] = pOut->m[1][1];

	pOut->m[2][1] = sinf(theta);
	pOut->m[1][2] = -pOut->m[2][1];
}

void RageMatrixRotationY( RageMatrix* pOut, float theta )
{
	theta *= PI/180;

	RageMatrixIdentity(pOut);
	pOut->m[0][0] = cosf(theta);
	pOut->m[2][2] = pOut->m[0][0];

	pOut->m[0][2] = sinf(theta);
	pOut->m[2][0] = -pOut->m[0][2];
}

void RageMatrixRotationZ( RageMatrix* pOut, float theta )
{
	theta *= PI/180;

	RageMatrixIdentity(pOut);
	pOut->m[0][0] = cosf(theta);
	pOut->m[1][1] = pOut->m[0][0];

	pOut->m[0][1] = sinf(theta);
	pOut->m[1][0] = -pOut->m[0][1];
}

RageMatrix RageMatrixRotationX( float theta )
{
	RageMatrix m;
	RageMatrixRotationX( &m, theta );
	return m;
}
RageMatrix RageMatrixRotationY( float theta )
{
	RageMatrix m;
	RageMatrixRotationY( &m, theta );
	return m;
}
RageMatrix RageMatrixRotationZ( float theta )
{
	RageMatrix m;
	RageMatrixRotationZ( &m, theta );
	return m;
}


/* This is similar in style to Actor::Command.  However, Actors don't store
 * matrix stacks; they only store offsets and scales, and compound them into
 * a single transformations at once.  This makes some things easy, but it's not
 * convenient for generic 3d transforms.  For that, we have this, which has the
 * small subset of the actor commands that applies to raw matrices, and we apply
 * commands in the order given.  "scale,2;x,1;" is very different from
 * "x,1;scale,2;". */
static CString GetParam( const CStringArray& sParams, int iIndex, int& iMaxIndexAccessed )
{
	iMaxIndexAccessed = max( iIndex, iMaxIndexAccessed );
	if( iIndex < int(sParams.size()) )
		return sParams[iIndex];
	else
		return "";
}

void RageMatrixCommand( CString sCommandString, RageMatrix &mat )
{
	CStringArray asCommands;
	split( sCommandString, ";", asCommands, true );
	
	for( unsigned c=0; c<asCommands.size(); c++ )
	{
		CStringArray asTokens;
		split( asCommands[c], ",", asTokens, true );

		int iMaxIndexAccessed = 0;

#define sParam(i) (GetParam(asTokens,i,iMaxIndexAccessed))
#define fParam(i) (strtof(sParam(i),NULL))
#define iParam(i) (atoi(sParam(i)))
#define bParam(i) (iParam(i)!=0)

		CString& sName = asTokens[0];
		sName.MakeLower();

		RageMatrix b;
		// Act on command
		if( sName=="x" )					RageMatrixTranslation( &b, fParam(1),0,0 );
		else if( sName=="y" )				RageMatrixTranslation( &b, 0,fParam(1),0 );
		else if( sName=="z" )				RageMatrixTranslation( &b, 0,0,fParam(1) );
		else if( sName=="zoomx" )			RageMatrixScaling(&b, fParam(1),1,1 );
		else if( sName=="zoomy" )			RageMatrixScaling(&b, 1,fParam(1),1 );
		else if( sName=="zoomz" )			RageMatrixScaling(&b, 1,1,fParam(1) );
		else if( sName=="rotationx" )		RageMatrixRotationX( &b, fParam(1) );
		else if( sName=="rotationy" )		RageMatrixRotationY( &b, fParam(1) );
		else if( sName=="rotationz" )		RageMatrixRotationZ( &b, fParam(1) );
		else
		{
			CString sError = ssprintf( "MatrixCommand:  Unrecognized matrix command name '%s' in command string '%s'.", sName.c_str(), sCommandString.c_str() );
			LOG->Warn( sError );
			Dialog::OK( sError );
			continue;
		}


		if( iMaxIndexAccessed != (int)asTokens.size()-1 )
		{
			CString sError = ssprintf( "MatrixCommand:  Wrong number of parameters in command '%s'.  Expected %d but there are %d.", join(",",asTokens).c_str(), iMaxIndexAccessed+1, (int)asTokens.size() );
			LOG->Warn( sError );
			Dialog::OK( sError );
			continue;
		}

		RageMatrix a(mat);
		RageMatrixMultiply(&mat, &a, &b);
	}
}


void RageQuatMultiply( RageVector4* pOut, const RageVector4 &pA, const RageVector4 &pB )
{
	RageVector4 out;
	out.x = pA.w * pB.x + pA.x * pB.w + pA.y * pB.z - pA.z * pB.y;
	out.y = pA.w * pB.y + pA.y * pB.w + pA.z * pB.x - pA.x * pB.z;
	out.z = pA.w * pB.z + pA.z * pB.w + pA.x * pB.y - pA.y * pB.x;
	out.w = pA.w * pB.w - pA.x * pB.x - pA.y * pB.y - pA.z * pB.z;

    float dist, square;

	square = out.x * out.x + out.y * out.y + out.z * out.z + out.w * out.w;
	
	if (square > 0.0)
		dist = 1.0f / sqrtf(square);
	else dist = 1;

    out.x *= dist;
    out.y *= dist;
    out.z *= dist;
    out.w *= dist;

	*pOut = out;
}

RageVector4 RageQuatFromH(float theta )
{
	theta *= PI/180.0f;
	theta /= 2.0f;
	theta *= -1;
	const float c = cosf(theta);
	const float s = sinf(theta);

	return RageVector4(0, s, 0, c);
}

RageVector4 RageQuatFromP(float theta )
{
	theta *= PI/180.0f;
	theta /= 2.0f;
	theta *= -1;
	const float c = cosf(theta);
	const float s = sinf(theta);

	return RageVector4(s, 0, 0, c);
}

RageVector4 RageQuatFromR(float theta )
{
	theta *= PI/180.0f;
	theta /= 2.0f;
	theta *= -1;
	const float c = cosf(theta);
	const float s = sinf(theta);

	return RageVector4(0, 0, s, c);
}


/* Math from http://www.gamasutra.com/features/19980703/quaternions_01.htm . */

/* prh.xyz -> heading, pitch, roll */
void RageQuatFromHPR(RageVector4* pOut, RageVector3 hpr )
{
	hpr *= PI;
	hpr /= 180.0f;
	hpr /= 2.0f;

	const float sX = sinf(hpr.x);
	const float cX = cosf(hpr.x);
	const float sY = sinf(hpr.y);
	const float cY = cosf(hpr.y);
	const float sZ = sinf(hpr.z);
	const float cZ = cosf(hpr.z);

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
void RageQuatFromPRH(RageVector4* pOut, RageVector3 prh )
{
	prh *= PI;
	prh /= 180.0f;
	prh /= 2.0f;

	/* Set cX to the cosine of the angle we want to rotate on the X axis,
	 * and so on.  Here, hpr.z (roll) rotates on the Z axis, hpr.x (heading)
	 * on Y, and hpr.y (pitch) on X. */
	const float sX = sinf(prh.y);
	const float cX = cosf(prh.y);
	const float sY = sinf(prh.x);
	const float cY = cosf(prh.x);
	const float sZ = sinf(prh.z);
	const float cZ = cosf(prh.z);

	pOut->w = cX * cY * cZ + sX * sY * sZ;
	pOut->x = sX * cY * cZ - cX * sY * sZ;
	pOut->y = cX * sY * cZ + sX * cY * sZ;
	pOut->z = cX * cY * sZ - sX * sY * cZ;
}

void RageMatrixFromQuat( RageMatrix* pOut, const RageVector4 q )
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
	*pOut = RageMatrix(
		1-(yy+zz), xy+wz,     xz-wy,     0,
		xy-wz,     1-(xx+zz), yz+wx,     0,
		xz+wy,     yz-wx,     1-(xx+yy), 0,
		0,         0,         0,         1 );
}

void RageQuatSlerp(RageVector4 *pOut, const RageVector4 &from, const RageVector4 &to, float t)
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
	} else  {
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
		float omega = acosf(cosom);
		float sinom = sinf(omega);
		scale0 = sinf((1.0f - t) * omega) / sinom;
		scale1 = sinf(t * omega) / sinom;
	} else {        
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

RageMatrix RageLookAt(
	float eyex, float eyey, float eyez,
	float centerx, float centery, float centerz,
	float upx, float upy, float upz )
{
	RageVector3 Z(eyex - centerx, eyey - centery, eyez - centerz);
	RageVec3Normalize(&Z, &Z);

	RageVector3 Y(upx, upy, upz);

	RageVector3 X(
		 Y[1] * Z[2] - Y[2] * Z[1],
		-Y[0] * Z[2] + Y[2] * Z[0],
		 Y[0] * Z[1] - Y[1] * Z[0]);

	Y = RageVector3(
		 Z[1] * X[2] - Z[2] * X[1],
        -Z[0] * X[2] + Z[2] * X[0],
         Z[0] * X[1] - Z[1] * X[0] );

	RageVec3Normalize(&X, &X);
	RageVec3Normalize(&Y, &Y);

	RageMatrix mat(
		X[0], Y[0], Z[0], 0,
		X[1], Y[1], Z[1], 0,
		X[2], Y[2], Z[2], 0,
		   0,    0,    0, 1 );

	RageMatrix mat2;
	RageMatrixTranslation(&mat2, -eyex, -eyey, -eyez);

	RageMatrix ret;
	RageMatrixMultiply(&ret, &mat, &mat2);

	return ret;
}

RageMatrix RageMatrixIdentity()
{
	RageMatrix m;
	RageMatrixIdentity( &m );
	return m;
}

void RageMatrixAngles( RageMatrix* pOut, const RageVector3 &angles )
{
	const RageVector3 angles_radians( angles * 2*PI / 360 );
	
	const float sy = sinf( angles_radians[2] );
	const float cy = cosf( angles_radians[2] );
	const float sp = sinf( angles_radians[1] );
	const float cp = cosf( angles_radians[1] );
	const float sr = sinf( angles_radians[0] );
	const float cr = cosf( angles_radians[0] );

	RageMatrixIdentity( pOut );


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

void RageMatrixTranspose( RageMatrix* pOut, const RageMatrix* pIn )
{
	for( int i=0; i<4; i++)
		for( int j=0; j<4; j++)
			pOut->m[j][i] = pIn->m[i][j];
}

/*
 * Copyright (c) 2001-2003 Chris Danford
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
