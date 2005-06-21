/*
 * Most of these prototypes match up with the D3DX math functions.  Take a
 * function name, replace "Rage" with "D3DX" and look it up in the D3D SDK
 * docs for details.
 */

#include "global.h"
#include "RageMath.h"
#include "RageTypes.h"
#include "RageUtil.h"
#include "RageDisplay.h"
#include "RageLog.h"
#include "arch/Dialog/Dialog.h"
#include <float.h>

#if defined(__MACOSX__)
# include <vecLib/vBLAS.h>
#endif

void RageVec3ClearBounds( RageVector3 &mins, RageVector3 &maxs )
{
	mins = RageVector3( FLT_MAX, FLT_MAX, FLT_MAX );
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
#if defined(__MACOSX__)
	// (M^t * v)^t = v^t * M 
	cblas_sgemv(CblasRowMajor, CblasTrans, 4, 4, 1, &pM->m00, 4, &pV->x, 1,
				0, &pOut->x, 1);
#else
    const RageMatrix &a = *pM;
    const RageVector4 &v = *pV;
	*pOut = RageVector4(
		a.m00*v.x+a.m10*v.y+a.m20*v.z+a.m30*v.w,
		a.m01*v.x+a.m11*v.y+a.m21*v.z+a.m31*v.w,
		a.m02*v.x+a.m12*v.y+a.m22*v.z+a.m32*v.w,
		a.m03*v.x+a.m13*v.y+a.m23*v.z+a.m33*v.w );
#endif
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
	static float identity[16] = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
	memcpy(&pOut->m00, identity, sizeof(identity));
/*	*pOut = RageMatrix(
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1 );
*/
}

RageMatrix RageMatrix::GetTranspose() const
{
	return RageMatrix(m00,m10,m20,m30,m01,m11,m21,m31,m02,m12,m22,m32,m03,m13,m23,m33);
}

void RageMatrixMultiply( RageMatrix* pOut, const RageMatrix* pA, const RageMatrix* pB )
{
#if defined(__MACOSX__)
	cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 4, 4, 4, 1,
				&pB->m00, 4, &pA->m00, 4, 0, &pOut->m00, 4);
#else
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
#endif
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

void RageMatrixSkewX( RageMatrix* pOut, float fAmount )
{
	RageMatrixIdentity(pOut);
	pOut->m[1][0] = fAmount;
}

/*
 * Return:
 *
 * RageMatrix translate;
 * RageMatrixTranslation( &translate, fTransX, fTransY, fTransZ );
 * RageMatrix scale;
 * RageMatrixScaling( &scale, fScaleX, float fScaleY, float fScaleZ );
 * RageMatrixMultiply( pOut, &translate, &scale );
 */
void RageMatrixTranslateAndScale( RageMatrix* pOut,
			float fTransX, float fTransY, float fTransZ,
			float fScaleX, float fScaleY, float fScaleZ )
{
	pOut->m00 = fScaleX;
	pOut->m01 = 0;
	pOut->m02 = 0;
	pOut->m03 = 0;

	pOut->m10 = 0;
	pOut->m11 = fScaleY;
	pOut->m12 = 0;
	pOut->m13 = 0;

	pOut->m20 = 0;
	pOut->m21 = 0;
	pOut->m22 = fScaleZ;
	pOut->m23 = 0;

	pOut->m30 = fTransX;
	pOut->m31 = fTransY;
	pOut->m32 = fTransZ;
	pOut->m33 = 1;
}

void RageMatrixRotationX( RageMatrix* pOut, float theta )
{
	theta *= PI/180;

	RageMatrixIdentity(pOut);
	pOut->m[1][1] = RageFastCos(theta);
	pOut->m[2][2] = pOut->m[1][1];

	pOut->m[2][1] = RageFastSin(theta);
	pOut->m[1][2] = -pOut->m[2][1];
}

void RageMatrixRotationY( RageMatrix* pOut, float theta )
{
	theta *= PI/180;

	RageMatrixIdentity(pOut);
	pOut->m[0][0] = RageFastCos(theta);
	pOut->m[2][2] = pOut->m[0][0];

	pOut->m[0][2] = RageFastSin(theta);
	pOut->m[2][0] = -pOut->m[0][2];
}

void RageMatrixRotationZ( RageMatrix* pOut, float theta )
{
	theta *= PI/180;

	RageMatrixIdentity(pOut);
	pOut->m[0][0] = RageFastCos(theta);
	pOut->m[1][1] = pOut->m[0][0];

	pOut->m[0][1] = RageFastSin(theta);
	pOut->m[1][0] = -pOut->m[0][1];
}

/* Return RageMatrixRotationX(rX) * RageMatrixRotationY(rY) * RageMatrixRotationZ(rZ)
 * quickly (without actually doing two complete matrix multiplies), by removing the
 * parts of the matrix multiplies that we know will be 0. */
void RageMatrixRotationXYZ( RageMatrix* pOut, float rX, float rY, float rZ )
{
	rX *= PI/180;
	rY *= PI/180;
	rZ *= PI/180;

	const float cX = RageFastCos(rX);
	const float sX = RageFastSin(rX);
	const float cY = RageFastCos(rY);
	const float sY = RageFastSin(rY);
	const float cZ = RageFastCos(rZ);
	const float sZ = RageFastSin(rZ);

	/*
	 * X*Y:
	 * RageMatrix(
	 *	cY,  sY*sX, sY*cX, 0,
	 *	0,   cX,    -sX,   0,
	 *	-sY, cY*sX, cY*cX, 0,
	 *	0, 0, 0, 1
	 * );
	 *
	 * X*Y*Z:
	 *
	 * RageMatrix(
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
	const float c = RageFastCos(theta);
	const float s = RageFastSin(theta);

	return RageVector4(0, s, 0, c);
}

RageVector4 RageQuatFromP(float theta )
{
	theta *= PI/180.0f;
	theta /= 2.0f;
	theta *= -1;
	const float c = RageFastCos(theta);
	const float s = RageFastSin(theta);

	return RageVector4(s, 0, 0, c);
}

RageVector4 RageQuatFromR(float theta )
{
	theta *= PI/180.0f;
	theta /= 2.0f;
	theta *= -1;
	const float c = RageFastCos(theta);
	const float s = RageFastSin(theta);

	return RageVector4(0, 0, s, c);
}


/* Math from http://www.gamasutra.com/features/19980703/quaternions_01.htm . */

/* prh.xyz -> heading, pitch, roll */
void RageQuatFromHPR(RageVector4* pOut, RageVector3 hpr )
{
	hpr *= PI;
	hpr /= 180.0f;
	hpr /= 2.0f;

	const float sX = RageFastSin(hpr.x);
	const float cX = RageFastCos(hpr.x);
	const float sY = RageFastSin(hpr.y);
	const float cY = RageFastCos(hpr.y);
	const float sZ = RageFastSin(hpr.z);
	const float cZ = RageFastCos(hpr.z);

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
	const float sX = RageFastSin(prh.y);
	const float cX = RageFastCos(prh.y);
	const float sY = RageFastSin(prh.x);
	const float cY = RageFastCos(prh.x);
	const float sZ = RageFastSin(prh.z);
	const float cZ = RageFastCos(prh.z);

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
		float omega = acosf(cosom);
		float sinom = RageFastSin(omega);
		scale0 = RageFastSin((1.0f - t) * omega) / sinom;
		scale1 = RageFastSin(t * omega) / sinom;
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

void RageMatrixAngles( RageMatrix* pOut, const RageVector3 &angles )
{
	const RageVector3 angles_radians( angles * 2*PI / 360 );
	
	const float sy = RageFastSin( angles_radians[2] );
	const float cy = RageFastCos( angles_radians[2] );
	const float sp = RageFastSin( angles_radians[1] );
	const float cp = RageFastCos( angles_radians[1] );
	const float sr = RageFastSin( angles_radians[0] );
	const float cr = RageFastCos( angles_radians[0] );

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

float RageFastSin( float x )
{
	// from 0 to PI
	// sizeof(table) == 4096 == one page of memory in Windows
	static float table[1024];

	static bool bInited = false;
	if( !bInited )
	{
		bInited = true;
		for( unsigned i=0; i<ARRAYSIZE(table); i++ )
		{
			float x = SCALE(i,0,ARRAYSIZE(table),0.0f,PI);
			table[i] = sinf(x);
		}
	}

	// optimization
	if( x == 0 )
		return 0;

	float fIndex = SCALE( x, 0.0f, PI*2, 0, ARRAYSIZE(table)*2 );

	// lerp using samples from the table
	int iSampleIndex[2];
	iSampleIndex[0] = (int)floorf(fIndex);
	iSampleIndex[1] = iSampleIndex[0]+1;

	float fRemainder = fIndex - iSampleIndex[0];
	for( unsigned i=0; i<ARRAYSIZE(iSampleIndex); i++ )
        iSampleIndex[i] %= ARRAYSIZE(table) * 2;

	DEBUG_ASSERT( fRemainder>=0 && fRemainder<=1 );

	float fValue[ARRAYSIZE(iSampleIndex)];
	for( unsigned i=0; i<ARRAYSIZE(iSampleIndex); i++ )
	{
		int &iSample = iSampleIndex[i];
		float &fVal = fValue[i];

		if( iSample >= int(ARRAYSIZE(table)) )	// PI <= iSample < 2*PI
		{
			// sin(x) == -sin(PI+x)
			iSample -= ARRAYSIZE(table);
			DEBUG_ASSERT( iSample>=0 && iSample<ARRAYSIZE(table) );
			fVal = -table[iSample];
		}
		else
		{
			fVal = table[iSample];
		}
	}

	return SCALE( fRemainder, 0.0f, 1.0f, fValue[0], fValue[1] );
}

float RageFastCos( float x )
{
	return RageFastSin( x + 0.5f*PI );
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
