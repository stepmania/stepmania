/***
*
*	Copyright (c) 1998, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

#ifndef __MATHLIB__
#define __MATHLIB__

// mathlib.h

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef float vec_t;
typedef vec_t vec3_t[3];	// x,y,z
typedef vec_t vec4_t[4];	// x,y,z,w


#define	Q_PI	3.14159265358979323846

// Use this definition globally
#define	ON_EPSILON		0.01
#define	EQUAL_EPSILON	0.001

int VectorCompare (vec3_t v1, vec3_t v2);

#define DotProduct(x,y) ((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define VectorCopy(a,b) {(b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2];}


vec_t VectorNormalize (vec3_t v);
void VectorInverse (vec3_t v);

void ClearBounds (vec3_t mins, vec3_t maxs);
void AddPointToBounds (vec3_t v, vec3_t mins, vec3_t maxs);

void AngleMatrix (const vec3_t angles, float matrix[3][4] );
void AngleIMatrix (const vec3_t angles, float matrix[3][4] );
void R_ConcatTransforms (const float in1[3][4], const float in2[3][4], float out[3][4]);

void VectorIRotate (const vec3_t in1, const float in2[3][4], vec3_t out);
void VectorRotate (const vec3_t in1, const float in2[3][4], vec3_t out);

void VectorTransform (const vec3_t in1, const float in2[3][4], vec3_t out);

void AngleQuaternion( const vec3_t angles, vec4_t quaternion );
void QuaternionMatrix( const vec4_t quaternion, float (*matrix)[4] );
void QuaternionSlerp( const vec4_t p, vec4_t q, float t, vec4_t qt );


#ifdef __cplusplus
}
#endif

#endif
