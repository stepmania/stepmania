/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net

     $Id$
*/


#ifndef SG_H
#define SG_H  1

#include "ul.h"

#define sgFloat float
#define SGfloat float

#define SG_ZERO  0.0f
#define SG_HALF  0.5f
#define SG_ONE   1.0f
#define SG_TWO   2.0f
#define SG_THREE 3.0f
#define SG_45    45.0f
#define SG_60    60.0f
#define SG_90    90.0f
#define SG_180   180.0f
#define SG_MAX   FLT_MAX

#define SG_X	0
#define SG_Y	1
#define SG_Z	2
#define SG_W	3

#ifndef M_PI
#define SG_PI  3.1415926535f
#else
#define SG_PI  ((SGfloat) M_PI)
#endif

#define SG_DEGREES_TO_RADIANS  (SG_PI/SG_180)
#define SG_RADIANS_TO_DEGREES  (SG_180/SG_PI)

/*
  These are just convenient redefinitions of standard
  math library functions to stop float/double warnings.
*/
   
inline SGfloat sgSqrt   ( const SGfloat x ) { return (SGfloat) sqrt ( x ) ; }
inline SGfloat sgSquare ( const SGfloat x ) { return x * x ; }
inline SGfloat sgAbs    ( const SGfloat a ) { return (a<SG_ZERO) ? -a : a ; }

/* 
  Type-casted sin/cos/tan/asin/acos/atan2 ANGLES IN DEGREES
*/

inline SGfloat sgASin ( SGfloat s )
                { return (SGfloat) asin ( s ) * SG_RADIANS_TO_DEGREES ; }
inline SGfloat sgACos ( SGfloat s )
                { return (SGfloat) acos ( s ) * SG_RADIANS_TO_DEGREES ; }
inline SGfloat sgATan ( SGfloat s )
                { return (SGfloat) atan ( s ) * SG_RADIANS_TO_DEGREES ; }
inline SGfloat sgATan2 ( SGfloat y, SGfloat x )
                { return (SGfloat) atan2 ( y,x ) * SG_RADIANS_TO_DEGREES ; }
inline SGfloat sgSin ( SGfloat s )
                { return (SGfloat) sin ( s * SG_DEGREES_TO_RADIANS ) ; }
inline SGfloat sgCos ( SGfloat s )
                { return (SGfloat) cos ( s * SG_DEGREES_TO_RADIANS ) ; }
inline SGfloat sgTan ( SGfloat s )
                { return (SGfloat) tan ( s * SG_DEGREES_TO_RADIANS ) ; }


inline int sgCompareFloat ( const SGfloat a, const SGfloat b, const SGfloat tol )
{
  if ( ( a + tol ) < b ) return -1 ;
  if ( ( b + tol ) < a ) return  1 ;
  return 0 ;
}


/*
  Types used in SG.
*/

typedef SGfloat sgVec2 [ 2 ] ;
typedef SGfloat sgVec3 [ 3 ] ;
typedef SGfloat sgVec4 [ 4 ] ;

typedef sgVec4 sgQuat ;

typedef SGfloat sgMat3  [3][3] ;
typedef SGfloat sgMat4  [4][4] ;

struct sgCoord
{
  sgVec3 xyz ;
  sgVec3 hpr ;
} ;

class sgSphere ;
class sgBox ;
class sgFrustum ;

/*
  Some handy constants
*/

#define SG_OUTSIDE  FALSE
#define SG_INSIDE    TRUE
#define SG_STRADDLE     2

inline SGfloat sgHeightOfPlaneVec2 ( const sgVec4 plane, const sgVec2 pnt )
{
  if ( plane[2] == SG_ZERO )
    return SG_ZERO ;
  else
    return -( plane[0] * pnt[0] + plane[1] * pnt[1] + plane[3] ) / plane[2] ;
}

/*
  Convert a direction vector into a set of euler angles,
  (with zero roll)
*/

extern void sgHPRfromVec3 ( sgVec3 hpr, const sgVec3 src ) ;

extern void sgMakeCoordMat4 ( sgMat4 dst, const SGfloat x, const SGfloat y, const SGfloat z,
                                          const SGfloat h, const SGfloat p, const SGfloat r ) ;

inline void sgMakeCoordMat4( sgMat4 dst, const sgVec3 xyz, const sgVec3 hpr )
{
  sgMakeCoordMat4 ( dst, xyz[0], xyz[1], xyz[2],
                         hpr[0], hpr[1], hpr[2] ) ;
}

inline void sgMakeCoordMat4( sgMat4 dst, const sgCoord *src )
{
  sgMakeCoordMat4 ( dst, src->xyz, src->hpr ) ;
}

extern void sgMakeLookAtMat4 ( sgMat4 dst,
        const sgVec3 eye, const sgVec3 center, const sgVec3 up ) ;

extern void sgMakeRotMat4   ( sgMat4 dst, const SGfloat angle, const sgVec3 axis ) ;

inline void sgMakeRotMat4   ( sgMat4 dst, const sgVec3 hpr )
{
  sgMakeCoordMat4 ( dst, SG_ZERO, SG_ZERO, SG_ZERO, hpr[0], hpr[1], hpr[2] ) ;
}

inline void sgMakeRotMat4   ( sgMat4 dst,const SGfloat h, const SGfloat p, const SGfloat r )
{
  sgMakeCoordMat4 ( dst, SG_ZERO, SG_ZERO, SG_ZERO, h, p, r ) ;
}

extern void sgMakeTransMat4 ( sgMat4 dst, const sgVec3 xyz ) ;
extern void sgMakeTransMat4 ( sgMat4 dst, const SGfloat x, const SGfloat y, const SGfloat z ) ;


extern void sgSetCoord      ( sgCoord *coord, const sgMat4 src ) ;

extern void sgMultMat4      ( sgMat4 dst, const sgMat4 a, const sgMat4 b ) ;
extern void sgPostMultMat4  ( sgMat4 dst, const sgMat4 a ) ;
extern void sgPreMultMat4   ( sgMat4 dst, const sgMat4 a ) ;

extern void sgTransposeNegateMat4 ( sgMat4 dst ) ;
extern void sgTransposeNegateMat4 ( sgMat4 dst, const sgMat4 src ) ;
extern void sgInvertMat4 ( sgMat4 dst, const sgMat4 src ) ;
inline void sgInvertMat4 ( sgMat4 dst ) { sgInvertMat4 ( dst, dst ) ; }

extern void sgXformVec3     ( sgVec3 dst, const sgVec3 src, const sgMat4 mat ) ;
extern void sgXformPnt3     ( sgVec3 dst, const sgVec3 src, const sgMat4 mat ) ;
extern void sgXformVec4     ( sgVec4 dst, const sgVec4 src, const sgMat4 mat ) ;
extern void sgXformPnt4     ( sgVec4 dst, const sgVec4 src, const sgMat4 mat ) ;
extern void sgFullXformPnt3 ( sgVec3 dst, const sgVec3 src, const sgMat4 mat ) ;
extern void sgFullXformPnt4 ( sgVec4 dst, const sgVec4 src, const sgMat4 mat ) ;

inline void sgXformVec3     ( sgVec3 dst, const sgMat4 mat ) { sgXformVec3 ( dst, dst, mat ) ; }
inline void sgXformPnt3     ( sgVec3 dst, const sgMat4 mat ) { sgXformPnt3 ( dst, dst, mat ) ; }
inline void sgXformVec4     ( sgVec4 dst, const sgMat4 mat ) { sgXformVec4 ( dst, dst, mat ) ; }
inline void sgXformPnt4     ( sgVec4 dst, const sgMat4 mat ) { sgXformPnt4 ( dst, dst, mat ) ; }
inline void sgFullXformPnt3 ( sgVec3 dst, const sgMat4 mat ) { sgFullXformPnt3 ( dst, dst, mat ) ; }
inline void sgFullXformPnt4 ( sgVec4 dst, const sgMat4 mat ) { sgFullXformPnt4 ( dst, dst, mat ) ; }


/* Bits returned by sgClassifyMat4 */

#define SG_IDENTITY        0x00   // for clarity
#define SG_ROTATION        0x01   // includes a rotational component
#define SG_MIRROR          0x02   // changes handedness (det < 0)
#define SG_SCALE           0x04   // uniform scaling
#define SG_NONORTHO        0x10   // 3x3 not orthogonal
#define SG_TRANSLATION     0x20   // translates
#define SG_PROJECTION      0x40   // forth column not 0,0,0,1

/* Are these needed? sgClassifyMat4() does set the general scale bit for some matrices,
 * but it is not easily defined. Use SG_NONORTHO instead (which is also set). */
#define SG_UNIFORM_SCALE   SG_SCALE
#define SG_GENERAL_SCALE   0x08   // x, y and z scaled differently

extern int sgClassifyMat4 ( const sgMat4 mat ) ;



/*
  Basic low-level vector functions.
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  For each of Vec2, Vec3 and Vec4, we provide inlines for

  Zero      - set all elements to zero.
  Set       - set each element individually.
  Add       - add vectors element by element.
  Sub       - subtract vectors element by element.
  Scale     - multiply each element of a vector by a variable.
  AddScaled - multiply second vector by a constant and add the
              result to the first vector.
  Negate    - negate each element of a vector.
  Compare   - compare vectors element by element with optional tolerance.
              (return TRUE if vectors are equal - within tolerances)
  Equal     - return TRUE if vectors are exactly equal.
  Length    - compute length of a vector.
  Distance  - compute distance between two points.
  ScalarProduct - scalar (dot) product.
  VectorProduct - vector (cross) product (3-element vectors ONLY!).
  Normalise/Normalize - make vector be one unit long.
*/

inline void sgZeroVec2 ( sgVec2 dst ) { dst[0]=dst[1]=SG_ZERO ; }
inline void sgZeroVec3 ( sgVec3 dst ) { dst[0]=dst[1]=dst[2]=SG_ZERO ; }
inline void sgZeroVec4 ( sgVec4 dst ) { dst[0]=dst[1]=dst[2]=dst[3]=SG_ZERO ; }


inline void sgSetVec2 ( sgVec2 dst, const SGfloat x, const SGfloat y )
{
  dst [ 0 ] = x ;
  dst [ 1 ] = y ;
}

inline void sgSetVec3 ( sgVec3 dst, const SGfloat x, const SGfloat y, const SGfloat z )
{
  dst [ 0 ] = x ;
  dst [ 1 ] = y ;
  dst [ 2 ] = z ;
}

inline void sgSetVec4 ( sgVec4 dst, const SGfloat x, const SGfloat y, const SGfloat z, const SGfloat w )
{
  dst [ 0 ] = x ;
  dst [ 1 ] = y ;
  dst [ 2 ] = z ;
  dst [ 3 ] = w ;
}


inline void sgCopyVec2 ( sgVec2 dst, const sgVec2 src )
{
  dst [ 0 ] = src [ 0 ] ;
  dst [ 1 ] = src [ 1 ] ;
}

inline void sgCopyVec3 ( sgVec3 dst, const sgVec3 src )
{
  dst [ 0 ] = src [ 0 ] ;
  dst [ 1 ] = src [ 1 ] ;
  dst [ 2 ] = src [ 2 ] ;
}

inline void sgCopyVec4 ( sgVec4 dst, const sgVec4 src )
{
  dst [ 0 ] = src [ 0 ] ;
  dst [ 1 ] = src [ 1 ] ;
  dst [ 2 ] = src [ 2 ] ;
  dst [ 3 ] = src [ 3 ] ;
}


inline void sgAddVec2 ( sgVec2 dst, const sgVec2 src )
{
  dst [ 0 ] += src [ 0 ] ;
  dst [ 1 ] += src [ 1 ] ;
}

inline void sgAddVec3 ( sgVec3 dst, const sgVec3 src )
{
  dst [ 0 ] += src [ 0 ] ;
  dst [ 1 ] += src [ 1 ] ;
  dst [ 2 ] += src [ 2 ] ;
}

inline void sgAddVec4 ( sgVec4 dst, const sgVec4 src )
{
  dst [ 0 ] += src [ 0 ] ;
  dst [ 1 ] += src [ 1 ] ;
  dst [ 2 ] += src [ 2 ] ;
  dst [ 3 ] += src [ 3 ] ;
}


inline void sgAddVec2 ( sgVec2 dst, const sgVec2 src1, const sgVec2 src2 )
{
  dst [ 0 ] = src1 [ 0 ] + src2 [ 0 ] ;
  dst [ 1 ] = src1 [ 1 ] + src2 [ 1 ] ;
}

inline void sgAddVec3 ( sgVec3 dst, const sgVec3 src1, const sgVec3 src2 )
{
  dst [ 0 ] = src1 [ 0 ] + src2 [ 0 ] ;
  dst [ 1 ] = src1 [ 1 ] + src2 [ 1 ] ;
  dst [ 2 ] = src1 [ 2 ] + src2 [ 2 ] ;
}

inline void sgAddVec4 ( sgVec4 dst, const sgVec4 src1, const sgVec4 src2 )
{
  dst [ 0 ] = src1 [ 0 ] + src2 [ 0 ] ;
  dst [ 1 ] = src1 [ 1 ] + src2 [ 1 ] ;
  dst [ 2 ] = src1 [ 2 ] + src2 [ 2 ] ;
  dst [ 3 ] = src1 [ 3 ] + src2 [ 3 ] ;
}


inline void sgSubVec2 ( sgVec2 dst, const sgVec2 src )
{
  dst [ 0 ] -= src [ 0 ] ;
  dst [ 1 ] -= src [ 1 ] ;
}

inline void sgSubVec3 ( sgVec3 dst, const sgVec3 src )
{
  dst [ 0 ] -= src [ 0 ] ;
  dst [ 1 ] -= src [ 1 ] ;
  dst [ 2 ] -= src [ 2 ] ;
}

inline void sgSubVec4 ( sgVec4 dst, const sgVec4 src )
{
  dst [ 0 ] -= src [ 0 ] ;
  dst [ 1 ] -= src [ 1 ] ;
  dst [ 2 ] -= src [ 2 ] ;
  dst [ 3 ] -= src [ 3 ] ;
}

inline void sgSubVec2 ( sgVec2 dst, const sgVec2 src1, const sgVec2 src2 )
{
  dst [ 0 ] = src1 [ 0 ] - src2 [ 0 ] ;
  dst [ 1 ] = src1 [ 1 ] - src2 [ 1 ] ;
}

inline void sgSubVec3 ( sgVec3 dst, const sgVec3 src1, const sgVec3 src2 )
{
  dst [ 0 ] = src1 [ 0 ] - src2 [ 0 ] ;
  dst [ 1 ] = src1 [ 1 ] - src2 [ 1 ] ;
  dst [ 2 ] = src1 [ 2 ] - src2 [ 2 ] ;
}

inline void sgSubVec4 ( sgVec4 dst, const sgVec4 src1, const sgVec4 src2 )
{
  dst [ 0 ] = src1 [ 0 ] - src2 [ 0 ] ;
  dst [ 1 ] = src1 [ 1 ] - src2 [ 1 ] ;
  dst [ 2 ] = src1 [ 2 ] - src2 [ 2 ] ;
  dst [ 3 ] = src1 [ 3 ] - src2 [ 3 ] ;
}


inline void sgNegateVec2 ( sgVec2 dst )
{
  dst [ 0 ] = -dst [ 0 ] ;
  dst [ 1 ] = -dst [ 1 ] ;
}

inline void sgNegateVec3 ( sgVec3 dst )
{
  dst [ 0 ] = -dst [ 0 ] ;
  dst [ 1 ] = -dst [ 1 ] ;
  dst [ 2 ] = -dst [ 2 ] ;
}

inline void sgNegateVec4 ( sgVec4 dst )
{
  dst [ 0 ] = -dst [ 0 ] ;
  dst [ 1 ] = -dst [ 1 ] ;
  dst [ 2 ] = -dst [ 2 ] ;
  dst [ 3 ] = -dst [ 3 ] ;
}


inline void sgNegateVec2 ( sgVec2 dst, const sgVec2 src )
{
  dst [ 0 ] = -src [ 0 ] ;
  dst [ 1 ] = -src [ 1 ] ;
}

inline void sgNegateVec3 ( sgVec3 dst, const sgVec3 src )
{
  dst [ 0 ] = -src [ 0 ] ;
  dst [ 1 ] = -src [ 1 ] ;
  dst [ 2 ] = -src [ 2 ] ;
}

inline void sgNegateVec4 ( sgVec4 dst, const sgVec4 src )
{
  dst [ 0 ] = -src [ 0 ] ;
  dst [ 1 ] = -src [ 1 ] ;
  dst [ 2 ] = -src [ 2 ] ;
  dst [ 3 ] = -src [ 3 ] ;
}


inline void sgScaleVec2 ( sgVec2 dst, const SGfloat s )
{
  dst [ 0 ] *= s ;
  dst [ 1 ] *= s ;
}

inline void sgScaleVec3 ( sgVec3 dst, const SGfloat s )
{
  dst [ 0 ] *= s ;
  dst [ 1 ] *= s ;
  dst [ 2 ] *= s ;
}

inline void sgScaleVec4 ( sgVec4 dst, const SGfloat s )
{
  dst [ 0 ] *= s ;
  dst [ 1 ] *= s ;
  dst [ 2 ] *= s ;
  dst [ 3 ] *= s ;
}

inline void sgScaleVec2 ( sgVec2 dst, const sgVec2 src, const SGfloat s )
{
  dst [ 0 ] = src [ 0 ] * s ;
  dst [ 1 ] = src [ 1 ] * s ;
}

inline void sgScaleVec3 ( sgVec3 dst, const sgVec3 src, const SGfloat s )
{
  dst [ 0 ] = src [ 0 ] * s ;
  dst [ 1 ] = src [ 1 ] * s ;
  dst [ 2 ] = src [ 2 ] * s ;
}

inline void sgScaleVec4 ( sgVec4 dst, const sgVec4 src, const SGfloat s )
{
  dst [ 0 ] = src [ 0 ] * s ;
  dst [ 1 ] = src [ 1 ] * s ;
  dst [ 2 ] = src [ 2 ] * s ;
  dst [ 3 ] = src [ 3 ] * s ;
}


inline void sgAddScaledVec2 ( sgVec2 dst, const sgVec2 src, const SGfloat s )
{
  dst [ 0 ] += src [ 0 ] * s ;
  dst [ 1 ] += src [ 1 ] * s ;
}

inline void sgAddScaledVec3 ( sgVec3 dst, const sgVec3 src, const SGfloat s )
{
  dst [ 0 ] += src [ 0 ] * s ;
  dst [ 1 ] += src [ 1 ] * s ;
  dst [ 2 ] += src [ 2 ] * s ;
}

inline void sgAddScaledVec4 ( sgVec4 dst, const sgVec4 src, const SGfloat s )
{
  dst [ 0 ] += src [ 0 ] * s ;
  dst [ 1 ] += src [ 1 ] * s ;
  dst [ 2 ] += src [ 2 ] * s ;
  dst [ 3 ] += src [ 3 ] * s ;
}


inline void sgAddScaledVec2 ( sgVec2 dst, const sgVec2 src1, const sgVec2 src2, const SGfloat s )
{
  dst [ 0 ] = src1 [ 0 ] + src2 [ 0 ] * s ;
  dst [ 1 ] = src1 [ 1 ] + src2 [ 1 ] * s ;
}

inline void sgAddScaledVec3 ( sgVec3 dst, const sgVec3 src1, const sgVec3 src2, const SGfloat s )
{
  dst [ 0 ] = src1 [ 0 ] + src2 [ 0 ] * s ;
  dst [ 1 ] = src1 [ 1 ] + src2 [ 1 ] * s ;
  dst [ 2 ] = src1 [ 2 ] + src2 [ 2 ] * s ;
}

inline void sgAddScaledVec4 ( sgVec4 dst, const sgVec4 src1, const sgVec4 src2, const SGfloat s )
{
  dst [ 0 ] = src1 [ 0 ] + src2 [ 0 ] * s ;
  dst [ 1 ] = src1 [ 1 ] + src2 [ 1 ] * s ;
  dst [ 2 ] = src1 [ 2 ] + src2 [ 2 ] * s ;
  dst [ 3 ] = src1 [ 3 ] + src2 [ 3 ] * s ;
}


inline int sgCompareVec2 ( const sgVec2 a, const sgVec2 b, const SGfloat tol )
{
  if ( sgCompareFloat( a[0], b[0], tol ) != 0 ) return FALSE ;
  if ( sgCompareFloat( a[1], b[1], tol ) != 0 ) return FALSE ;

  return TRUE ;
}

inline int sgCompareVec3 ( const sgVec3 a, const sgVec3 b, const SGfloat tol )
{
  if ( sgCompareFloat( a[0], b[0], tol ) != 0 ) return FALSE ;
  if ( sgCompareFloat( a[1], b[1], tol ) != 0 ) return FALSE ;
  if ( sgCompareFloat( a[2], b[2], tol ) != 0 ) return FALSE ;

  return TRUE ;
}

inline int sgCompareVec4 ( const sgVec4 a, const sgVec4 b, const SGfloat tol )
{
  if ( sgCompareFloat( a[0], b[0], tol ) != 0 ) return FALSE ;
  if ( sgCompareFloat( a[1], b[1], tol ) != 0 ) return FALSE ;
  if ( sgCompareFloat( a[2], b[2], tol ) != 0 ) return FALSE ;
  if ( sgCompareFloat( a[3], b[3], tol ) != 0 ) return FALSE ;

  return TRUE ;
}


inline int sgEqualVec2 ( const sgVec2 a, const sgVec2 b )
{
  return a[0] == b[0] &&
         a[1] == b[1] ;
}

inline int sgEqualVec3 ( const sgVec3 a, const sgVec3 b )
{
  return a[0] == b[0] &&
         a[1] == b[1] &&
         a[2] == b[2] ;
}

inline int sgEqualVec4 ( const sgVec4 a, const sgVec4 b )
{
  return a[0] == b[0] &&
         a[1] == b[1] &&
         a[2] == b[2] &&
         a[3] == b[3] ;
}


inline SGfloat sgScalarProductVec2 ( const sgVec2 a, const sgVec2 b )
{
  return a[0]*b[0] + a[1]*b[1] ;
}

inline SGfloat sgScalarProductVec3 ( const sgVec3 a, const sgVec3 b )
{
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] ;
}

inline SGfloat sgScalarProductVec4 ( const sgVec4 a, const sgVec4 b )
{
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3] ;
}


extern void sgVectorProductVec3 ( sgVec3 dst, const sgVec3 a, const sgVec3 b ) ;

inline SGfloat sgLerp ( const SGfloat a, const SGfloat b, const SGfloat f )
{
  return a + f * ( b - a ) ;
}

inline void sgLerpVec4 ( sgVec4 dst, const sgVec4 a, const sgVec4 b, const SGfloat f )
{
  dst[0] = a[0] + f * ( b[0] - a[0] ) ;
  dst[1] = a[1] + f * ( b[1] - a[1] ) ;
  dst[2] = a[2] + f * ( b[2] - a[2] ) ;
  dst[3] = a[3] + f * ( b[3] - a[3] ) ;
}


inline void sgLerpVec3 ( sgVec3 dst, const sgVec3 a, const sgVec3 b, const SGfloat f )
{
  dst[0] = a[0] + f * ( b[0] - a[0] ) ;
  dst[1] = a[1] + f * ( b[1] - a[1] ) ;
  dst[2] = a[2] + f * ( b[2] - a[2] ) ;
}


inline void sgLerpVec2 ( sgVec2 dst, const sgVec2 a, const sgVec2 b, const SGfloat f )
{
  dst[0] = a[0] + f * ( b[0] - a[0] ) ;
  dst[1] = a[1] + f * ( b[1] - a[1] ) ;
}


inline void sgLerpAnglesVec3 ( sgVec3 dst, const sgVec3 a,
                                           const sgVec3 b,
                                           const SGfloat f )
{
  sgVec3 tmp ;
 
  if ( b[0] - a[0] >  180.0f ) tmp[0] = a[0] + 360.0f ; else
  if ( b[0] - a[0] < -180.0f ) tmp[0] = a[0] - 360.0f ; else tmp[0] = a[0] ;
 
  if ( b[1] - a[1] >  180.0f ) tmp[1] = a[1] + 360.0f ; else
  if ( b[1] - a[1] < -180.0f ) tmp[1] = a[1] - 360.0f ; else tmp[1] = a[1] ;
 
  if ( b[2] - a[2] >  180.0f ) tmp[2] = a[2] + 360.0f ; else
  if ( b[2] - a[2] < -180.0f ) tmp[2] = a[2] - 360.0f ; else tmp[2] = a[2] ;
 
  dst[0] = tmp[0] + f * ( b[0] - tmp[0] ) ;
  dst[1] = tmp[1] + f * ( b[1] - tmp[1] ) ;
  dst[2] = tmp[2] + f * ( b[2] - tmp[2] ) ;
}                                                                               



inline SGfloat sgDistanceSquaredVec2 ( const sgVec2 a, const sgVec2 b )
{
  return sgSquare ( a[0]-b[0] ) + sgSquare ( a[1]-b[1] ) ;
}

inline SGfloat sgDistanceSquaredVec3 ( const sgVec3 a, const sgVec3 b )
{
  return sgSquare ( a[0]-b[0] ) + sgSquare ( a[1]-b[1] ) +
         sgSquare ( a[2]-b[2] ) ;
}

inline SGfloat sgDistanceSquaredVec4 ( const sgVec4 a, const sgVec4 b )
{
  return sgSquare ( a[0]-b[0] ) + sgSquare ( a[1]-b[1] ) +
         sgSquare ( a[2]-b[2] ) + sgSquare ( a[3]-b[3] ) ;
}

inline SGfloat sgDistanceVec2 ( const sgVec2 a, const sgVec2 b )
{
  return sgSqrt ( sgSquare ( a[0]-b[0] ) + sgSquare ( a[1]-b[1] ) ) ;
}

inline SGfloat sgDistanceVec3 ( const sgVec3 a, const sgVec3 b )
{
  return sgSqrt ( sgSquare ( a[0]-b[0] ) + sgSquare ( a[1]-b[1] ) +
                  sgSquare ( a[2]-b[2] ) ) ;
}

inline SGfloat sgDistanceVec4 ( const sgVec4 a, const sgVec4 b )
{
  return sgSqrt ( sgSquare ( a[0]-b[0] ) + sgSquare ( a[1]-b[1] ) +
                  sgSquare ( a[2]-b[2] ) + sgSquare ( a[3]-b[3] ) ) ;
}


inline SGfloat sgLengthVec2 ( const sgVec2 src )
{
  return sgSqrt ( sgScalarProductVec2 ( src, src ) ) ;
}

inline SGfloat sgLengthVec3 ( const sgVec3 src )
{
  return sgSqrt ( sgScalarProductVec3 ( src, src ) ) ;
}

inline SGfloat sgLengthVec4 ( const sgVec4 src )
{
  return sgSqrt ( sgScalarProductVec4 ( src, src ) ) ;
}

inline SGfloat sgLengthSquaredVec2 ( sgVec2 const src )
{
  return sgScalarProductVec2 ( src, src ) ;
}

inline SGfloat sgLengthSquaredVec3 ( sgVec3 const src )
{
  return sgScalarProductVec3 ( src, src ) ;
}

inline SGfloat sgLengthSquaredVec4 ( sgVec4 const src )
{
  return sgScalarProductVec4 ( src, src ) ;
}


/* Anglo-US spelling issues.  <sigh> */
#define sgNormalizeVec2 sgNormaliseVec2
#define sgNormalizeVec3 sgNormaliseVec3
#define sgNormalizeVec4 sgNormaliseVec4
#define sgNormalizeQuat sgNormaliseQuat

inline void sgNormaliseVec2 ( sgVec2 dst )
{
  sgScaleVec2 ( dst, SG_ONE / sgLengthVec2 ( dst ) ) ;
}

inline void sgNormaliseVec3 ( sgVec3 dst )
{
  sgScaleVec3 ( dst, SG_ONE / sgLengthVec3 ( dst ) ) ;
}

inline void sgNormaliseVec4 ( sgVec4 dst )
{
  sgScaleVec4 ( dst, SG_ONE / sgLengthVec4 ( dst ) ) ;
}

inline void sgNormaliseVec2 ( sgVec2 dst, const sgVec2 src )
{
  sgScaleVec2 ( dst, src, SG_ONE / sgLengthVec2 ( src ) ) ;
}

inline void sgNormaliseVec3 ( sgVec3 dst, const sgVec3 src )
{
  sgScaleVec3 ( dst, src, SG_ONE / sgLengthVec3 ( src ) ) ;
}

inline void sgNormaliseVec4 ( sgVec4 dst, const sgVec4 src )
{
  sgScaleVec4 ( dst, src, SG_ONE / sgLengthVec4 ( src ) ) ;
}


inline void sgZeroCoord ( sgCoord *dst )
{
  sgSetVec3 ( dst->xyz, SG_ZERO, SG_ZERO, SG_ZERO ) ;
  sgSetVec3 ( dst->hpr, SG_ZERO, SG_ZERO, SG_ZERO ) ;
}

inline void sgSetCoord ( sgCoord *dst, const SGfloat x, const SGfloat y, const SGfloat z,
                                       const SGfloat h, const SGfloat p, const SGfloat r )
{
  sgSetVec3 ( dst->xyz, x, y, z ) ;
  sgSetVec3 ( dst->hpr, h, p, r ) ;
}

inline void sgSetCoord ( sgCoord *dst, const sgVec3 xyz, const sgVec3 hpr )
{
  sgCopyVec3 ( dst->xyz, xyz ) ;
  sgCopyVec3 ( dst->hpr, hpr ) ;
}

inline void sgCopyCoord ( sgCoord *dst, const sgCoord *src )
{
  sgCopyVec3 ( dst->xyz, src->xyz ) ;
  sgCopyVec3 ( dst->hpr, src->hpr ) ;
}



inline void sgCopyMat4 ( sgMat4 dst, const sgMat4 src )
{
  sgCopyVec4 ( dst[ 0 ], src[ 0 ] ) ;
  sgCopyVec4 ( dst[ 1 ], src[ 1 ] ) ;
  sgCopyVec4 ( dst[ 2 ], src[ 2 ] ) ;
  sgCopyVec4 ( dst[ 3 ], src[ 3 ] ) ;
}


inline void sgScaleMat4 ( sgMat4 dst, const sgMat4 src, const SGfloat scale )
{
  sgScaleVec4 ( dst[0], src[0], scale ) ;
  sgScaleVec4 ( dst[1], src[1], scale ) ;
  sgScaleVec4 ( dst[2], src[2], scale ) ;
  sgScaleVec4 ( dst[3], src[3], scale ) ;
}
 

inline void sgMakeIdentMat4 ( sgMat4 dst )
{
  sgSetVec4 ( dst[0], SG_ONE , SG_ZERO, SG_ZERO, SG_ZERO ) ;
  sgSetVec4 ( dst[1], SG_ZERO, SG_ONE , SG_ZERO, SG_ZERO ) ;
  sgSetVec4 ( dst[2], SG_ZERO, SG_ZERO, SG_ONE , SG_ZERO ) ;
  sgSetVec4 ( dst[3], SG_ZERO, SG_ZERO, SG_ZERO, SG_ONE  ) ;
}


extern void sgMakePickMatrix( sgMat4 mat, sgFloat x, sgFloat y,
                    sgFloat width, sgFloat height, sgVec4 viewport ) ;

extern int  sgCompare3DSqdDist ( const sgVec3 a, const sgVec3 b, const SGfloat sqd_dist ) ;

inline SGfloat sgDistToLineVec2 ( const sgVec3 line, const sgVec2 pnt )
{
  return sgScalarProductVec2 ( line, pnt ) + line[2] ;
}
 

struct sgLineSegment3   /* Bounded line segment */
{
  sgVec3 a ;
  sgVec3 b ;
} ;

struct sgLine3    /* Infinite line */
{
  sgVec3 point_on_line ;
  sgVec3 direction_vector ;  /* Should be a unit vector */
} ;


inline void sgLineSegment3ToLine3 ( sgLine3 *line,
                                   const sgLineSegment3 lineseg )
{
  sgCopyVec3      ( line->point_on_line   , lineseg.a ) ;
  sgSubVec3       ( line->direction_vector, lineseg.b, lineseg.a ) ;
  sgNormaliseVec3 ( line->direction_vector ) ;
}


SGfloat sgDistSquaredToLineVec3        ( const sgLine3 line,
                                         const sgVec3 pnt ) ;
SGfloat sgDistSquaredToLineSegmentVec3 ( const sgLineSegment3 line,
                                         const sgVec3 pnt ) ;


inline SGfloat sgDistToLineVec3 ( const sgLine3 line,
                                  const sgVec3 pnt )
{
  return sgSqrt ( sgDistSquaredToLineVec3 ( line, pnt ) );
}


inline SGfloat sgDistToLineSegmentVec3 ( const sgLineSegment3 line,
                                         const sgVec3 pnt )
{
  return sgSqrt ( sgDistSquaredToLineSegmentVec3(line,pnt) ) ;
}


inline SGfloat sgDistToPlaneVec3 ( const sgVec4 plane, const sgVec3 pnt )
{
  return sgScalarProductVec3 ( plane, pnt ) + plane[3] ;
}


inline SGfloat sgHeightAbovePlaneVec3 ( const sgVec4 plane, const sgVec3 pnt )
{
  return pnt[2] - sgHeightOfPlaneVec2 ( plane, pnt ) ;
}

extern void sgReflectInPlaneVec3 ( sgVec3 dst, const sgVec3 src, const sgVec4 plane ) ;

inline void sgReflectInPlaneVec3 ( sgVec3 dst, const sgVec4 plane ) 
{
  sgReflectInPlaneVec3 ( dst, dst, plane ) ;
}

extern void sgMakeNormal    ( sgVec3 dst, const sgVec3 a, const sgVec3 b, const sgVec3 c ) ;


inline void sgMake2DLine ( sgVec3 dst, const sgVec2 a, const sgVec2 b )
{
  dst[0] =   b[1]-a[1]  ;
  dst[1] = -(b[0]-a[0]) ;
  sgNormalizeVec2 ( dst ) ;
  dst[2] = - ( dst[0]*a[0] + dst[1]*a[1] ) ;
}

inline void sgMakePlane ( sgVec4 dst, const sgVec3 normal, const sgVec3 pnt )
{
  sgCopyVec3 ( dst, normal ) ;
  dst [ 3 ] = - sgScalarProductVec3 ( normal, pnt ) ;
}

inline void sgMakePlane ( sgVec4 dst, const sgVec3 a, const sgVec3 b, const sgVec3 c )
{
  /*
    Ax + By + Cz + D == 0 ;
    D = - ( Ax + By + Cz )
      = - ( A*a[0] + B*a[1] + C*a[2] )
      = - sgScalarProductVec3 ( normal, a ) ;
  */

  sgMakeNormal ( dst, a, b, c ) ;

  dst [ 3 ] = - sgScalarProductVec3 ( dst, a ) ;
}

float sgTriArea( sgVec3 p0, sgVec3 p1, sgVec3 p2 );


// Fast code. Result is in the range  0..pi:
inline SGfloat sgAngleBetweenNormalizedVec3 ( sgVec3 v1, sgVec3 v2 )
{
  float f = sgScalarProductVec3 ( v1, v2 ) ;
  
  return (float)(acos((f>=1.0f)?1.0f:(f<=-1.0f)?-1.0f:f)*SG_RADIANS_TO_DEGREES) ;
}

// Fast code. Result is in the range  0..pi:
SGfloat sgAngleBetweenVec3 ( sgVec3 v1, sgVec3 v2 );

// all three have to be normalized. Slow code. Result is in the range  0..2*pi:
SGfloat sgAngleBetweenNormalizedVec3 (sgVec3 first, sgVec3 second, sgVec3 normal);

// normal has to be normalized. Slow code. Result is in the range  0..2*pi:
SGfloat sgAngleBetweenVec3 ( sgVec3 v1, sgVec3 v2, sgVec3 normal );

class sgSphere
{
  sgVec3  center ;
  SGfloat radius ;
public:
  sgSphere () { empty () ; }

  const SGfloat *getCenter (void) const { return center ; }

  void setCenter ( const sgVec3 c )
  {
    sgCopyVec3 ( center, c ) ;
  }

  void setCenter ( const SGfloat x, const SGfloat y, const SGfloat z )
  {
    sgSetVec3 ( center, x, y, z ) ;
  }

  SGfloat getRadius (void) const        { return radius ; }
  void    setRadius ( const SGfloat r ) { radius = r ; }

  int isEmpty (void) const { return radius <  SG_ZERO ; }
  void empty  (void)	   { radius = - SG_ONE ; }

  void orthoXform ( const sgMat4 m )
  {
    sgXformPnt3 ( center, center, m ) ;
    // radius *= sgLengthVec3 ( m[0] ) ;  -- degrades performance for non-scaled matrices ...
  }

  void extend ( const sgSphere *s ) ;
  void extend ( const sgBox    *b ) ;
  void extend ( const sgVec3    v ) ;

  int intersects ( const sgSphere *s ) const 
  {
    return sgCompare3DSqdDist ( center, s->getCenter(),
                    sgSquare ( radius + s->getRadius() ) ) <= 0 ;
  }

  int intersects ( const sgVec4 plane ) const 
  {
    return sgAbs ( sgDistToPlaneVec3 ( plane, center ) ) <= radius ;
  }

  int intersects ( const sgBox *b ) const ;
} ;


class sgBox
{
  sgVec3 min ;
  sgVec3 max ;

public:
  sgBox () { empty () ; }

  const SGfloat *getMin (void) const { return min ; }
  const SGfloat *getMax (void) const { return max ; }

  void setMin ( const SGfloat x, const SGfloat y, const SGfloat z )
  { 
    sgSetVec3 ( min, x, y, z ) ;
  }

  void setMin ( const sgVec3 src )
  {
    sgCopyVec3 ( min, src ) ;
  }

  void setMax ( const SGfloat x, const SGfloat y, const SGfloat z )
  { 
    sgSetVec3 ( max, x, y, z ) ;
  }

  void setMax ( const sgVec3 src )
  { 
    sgCopyVec3 ( max, src ) ;
  }

  int isEmpty(void) const 
  {
    return ( min[0] > max[0] ||
             min[1] > max[1] ||
             min[2] > max[2] ) ;
  }

  void empty (void) 
  {
    sgSetVec3 ( min,  SG_MAX,  SG_MAX,  SG_MAX ) ;
    sgSetVec3 ( max, -SG_MAX, -SG_MAX, -SG_MAX ) ;
  }

  void extend  ( const sgSphere *s ) ;
  void extend  ( const sgBox    *b ) ;
  void extend  ( const sgVec3    v ) ;

  int intersects ( const sgSphere *s ) const 
  {
    return s -> intersects ( this ) ;
  }

  int intersects ( const sgBox *b ) const 
  {
    return min[0] <= b->getMax()[0] && max[0] >= b->getMin()[0] &&
           min[1] <= b->getMax()[1] && max[1] >= b->getMin()[1] &&
           min[2] <= b->getMax()[2] && max[2] >= b->getMin()[2] ;
  }

  int intersects ( const sgVec4 plane ) const ;
} ;

#define SG_NEAR       0x10
#define SG_FAR        0x20
#define SG_TOP        0x30
#define SG_BOT        0x40
#define SG_LEFT       0x50
#define SG_RIGHT      0x60

class sgFrustum
{
  /* The parameters for a glFrustum or pfMakePerspFrust */
  
  SGfloat left, right, top, bot, nnear, ffar ;

  /* The A,B,C terms of the plane equations of the four sloping planes */

  sgVec3 top_plane, bot_plane, left_plane, right_plane ;

  /* A GL/PF-style perspective matrix for this frustum */

  sgMat4 mat ;

  /* These two are only valid for simple frusta */

  SGfloat hfov ;    /* Horizontal Field of View */
  SGfloat vfov ;    /* Vertical   Field of View */

  void update (void) ;
  int getOutcode ( const sgVec4 src ) const ;

public:

  sgFrustum (void)
  {
    nnear = SG_ONE ;
    ffar  = 1000000.0f ;
    hfov  = SG_45 ;
    vfov  = SG_45 ;
    update () ;
  }

  void setFrustum ( const SGfloat l, const SGfloat r,
                    const SGfloat b, const SGfloat t,
                    const SGfloat n, const SGfloat f )
  {
    left  = l ; right = r ;
    top   = t ; bot   = b ;
    nnear = n ; ffar  = f ;
    hfov = vfov = SG_ZERO ;
    update () ;
  } 

  void     getMat4 ( sgMat4 dst ) { sgCopyMat4 ( dst, mat ) ; }

  SGfloat  getHFOV (void) const { return hfov  ; }
  SGfloat  getVFOV (void) const { return vfov  ; }
  SGfloat  getNear (void) const { return nnear ; }
  SGfloat  getFar  (void) const { return ffar  ; }
  SGfloat  getLeft (void) const { return left  ; }
  SGfloat  getRight(void) const { return right ; }
  SGfloat  getTop  (void) const { return top   ; }
  SGfloat  getBot  (void) const { return bot   ; }

  void getFOV ( SGfloat *h, SGfloat *v ) const 
  {
    if ( h != (SGfloat *) 0 ) *h = hfov ;
    if ( v != (SGfloat *) 0 ) *v = vfov ;
  }

  void setFOV ( const SGfloat h, const SGfloat v )
  {
    hfov = ( h <= 0 ) ? ( v * SG_THREE / SG_TWO ) : h ;
    vfov = ( v <= 0 ) ? ( h * SG_TWO / SG_THREE ) : v ;
    update () ;
  }

  void getNearFar ( SGfloat *n, SGfloat *f ) const 
  {
    if ( n != (SGfloat *) 0 ) *n = nnear ;
    if ( f != (SGfloat *) 0 ) *f = ffar  ;
  }

  void setNearFar ( const SGfloat n, const SGfloat f )
  {
    nnear = n ;
    ffar  = f ;
    update () ;
  }

  int  contains ( const sgVec3 p ) const ;
  int  contains ( const sgSphere *s ) const ;
} ;


/*
  Quaternion routines are Copyright (C) 1999
  Kevin B. Thompson <kevinbthompson@yahoo.com>
  Modified by Sylvan W. Clebsch <sylvan@stanford.edu>
  Largely rewritten by "Negative0" <negative0@earthlink.net>
*/

/*
  Quaternion structure  w = real, (x, y, z) = vector
  CHANGED sqQuat to float array so that syntax matches 
  vector and matrix routines
*/


inline void sgMakeIdentQuat ( sgQuat dst )
{
  sgSetVec4 ( dst, SG_ZERO, SG_ZERO, SG_ZERO, SG_ONE ) ;
}


inline void sgSetQuat ( sgQuat dst,
                        const SGfloat w, const SGfloat x,
                        const SGfloat y, const SGfloat z )
{
  sgSetVec4 ( dst, x, y, z, w ) ;
}

inline void sgCopyQuat ( sgQuat dst, const sgQuat src )
{
  sgCopyVec4 ( dst, src ) ;
}


/* Construct a unit quaternion (length==1) */

inline void sgNormaliseQuat ( sgQuat dst, const sgQuat src )
{
  SGfloat d = sgScalarProductVec4 ( src, src ) ;

  d = (d > SG_ZERO) ? (SG_ONE / sgSqrt ( d )) : SG_ONE ;

  sgScaleVec4 ( dst, src, d ) ;
}



inline void sgNormaliseQuat ( sgQuat dst ) { sgNormaliseQuat ( dst, dst ) ; }


inline void sgInvertQuat ( sgQuat dst, const sgQuat src )
{
  SGfloat d = sgScalarProductVec4 ( src, src ) ;

  d = ( d == SG_ZERO ) ? SG_ONE : ( SG_ONE / d ) ;

  dst[SG_W] =  src[SG_W] * d ;
  dst[SG_X] = -src[SG_X] * d ;
  dst[SG_Y] = -src[SG_Y] * d ;
  dst[SG_Z] = -src[SG_Z] * d ;
}

inline void sgInvertQuat ( sgQuat dst ) { sgInvertQuat ( dst, dst ) ; }


/* Make an angle and axis of rotation from a Quaternion. */

void sgQuatToAngleAxis ( SGfloat *angle, sgVec3 axis, const sgQuat src ) ;
void sgQuatToAngleAxis ( SGfloat *angle,
                         SGfloat *x, SGfloat *y, SGfloat *z,
                         const sgQuat src ) ;

/* Make a quaternion from a given angle and axis of rotation */

void sgAngleAxisToQuat ( sgQuat dst, const SGfloat angle, const sgVec3 axis ) ;
void sgAngleAxisToQuat ( sgQuat dst,
                         const SGfloat angle,
                         const SGfloat x, const SGfloat y, const SGfloat z ) ;

/* Convert a matrix to/from a quat */

void sgMatrixToQuat ( sgQuat quat, const sgMat4 m ) ;
void sgQuatToMatrix ( sgMat4 m, const sgQuat quat ) ;

/* Convert a set of eulers to/from a quat */

void sgQuatToEuler( sgVec3 hpr, const sgQuat quat ) ;
void sgEulerToQuat( sgQuat quat, const sgVec3 hpr ) ;

inline void sgEulerToQuat( sgQuat dst,
                           SGfloat h, SGfloat p, SGfloat r )
{
  sgVec3 hpr ;

  sgSetVec3 ( hpr, h, p, r ) ;

  sgEulerToQuat( dst, hpr ) ;
}

inline void sgHPRToQuat ( sgQuat dst, SGfloat h, SGfloat p, SGfloat r )
{
  sgVec3 hpr;

  hpr[0] = h * SG_DEGREES_TO_RADIANS ;
  hpr[1] = p * SG_DEGREES_TO_RADIANS ;
  hpr[2] = r * SG_DEGREES_TO_RADIANS ;

  sgEulerToQuat( dst, hpr ) ;
}

inline void sgHPRToQuat ( sgQuat dst, const sgVec3 hpr )
{
  sgVec3 tmp ;

  sgScaleVec3 ( tmp, hpr, SG_DEGREES_TO_RADIANS ) ;

  sgEulerToQuat ( dst, tmp ) ;
}

/* Multiply quaternions together (concatenate rotations) */

void sgMultQuat ( sgQuat dst, const sgQuat a, const sgQuat b ) ;

inline void sgPostMultQuat ( sgQuat dst, const sgQuat q )
{
  sgQuat r ;

  sgCopyQuat ( r, dst ) ;
  sgMultQuat ( dst, r, q ) ;
}

inline void sgPreMultQuat ( sgQuat dst, const sgQuat q )
{
  sgQuat r ;

  sgCopyQuat ( r, dst ) ;
  sgMultQuat ( dst, q, r ) ;
}


/* Rotate a quaternion by a given angle and axis (convenience function) */


inline void sgPreRotQuat ( sgQuat dst, const SGfloat angle, const sgVec3 axis )
{
  sgQuat q ;
  sgAngleAxisToQuat ( q, angle, axis ) ;
  sgPreMultQuat ( dst, q ) ;
  sgNormaliseQuat ( dst ) ;
}


inline void sgPostRotQuat ( sgQuat dst, const SGfloat angle, const sgVec3 axis )
{
  sgQuat q ;
  sgAngleAxisToQuat ( q, angle, axis ) ;
  sgPostMultQuat ( dst, q ) ;
  sgNormaliseQuat ( dst ) ;
}


inline void sgPreRotQuat ( sgQuat dst,
                        const SGfloat angle,
                        const SGfloat x, const SGfloat y, const SGfloat z )
{
  sgVec3 axis ;

  sgSetVec3 ( axis, x, y, z ) ;
  sgPreRotQuat ( dst, angle, axis ) ;
}

inline void sgPostRotQuat ( sgQuat dst,
                        const SGfloat angle,
                        const SGfloat x, const SGfloat y, const SGfloat z )
{
  sgVec3 axis ;

  sgSetVec3 ( axis, x, y, z ) ;
  sgPostRotQuat ( dst, angle, axis ) ;
}


/* more meaningful names */
#define sgWorldRotQuat  sgPostRotQuat
#define sgLocalRotQuat  sgPreRotQuat

/* for backwards compatibility */
#define sgRotQuat       sgPostRotQuat


/* SWC - Interpolate between to quaternions */

extern void sgSlerpQuat ( sgQuat dst,
                          const sgQuat from, const sgQuat to,
                          const SGfloat t ) ;



/*
  Intersection testing.
*/

int sgIsectPlanePlane       ( sgVec3 point, sgVec3 dir,
                              sgVec4 plane1, sgVec4 plane2 ) ;
int sgIsectInfLinePlane     ( sgVec3 dst,
                              sgVec3 l_org, sgVec3 l_vec,
                              sgVec4 plane ) ;
int sgIsectInfLineInfLine   ( sgVec3 dst,
                              sgVec3 l1_org, sgVec3 l1_vec,
                              sgVec3 l2_org, sgVec3 l2_vec ) ;
SGfloat sgIsectLinesegPlane ( sgVec3 dst,
                              sgVec3 v1, sgVec3 v2,
                              sgVec4 plane ) ;
bool sgPointInTriangle      ( sgVec3 point, sgVec3 tri[3] );




/**********************************************************************/

#define sgdFloat double
#define SGDfloat double

#define SGD_ZERO  0.0
#define SGD_HALF  0.5
#define SGD_ONE   1.0
#define SGD_TWO   2.0
#define SGD_THREE 3.0
#define SGD_45    45.0
#define SGD_60    60.0
#define SGD_90    90.0
#define SGD_180   180.0
#define SGD_MAX   DBL_MAX


#define SGD_X	0
#define SGD_Y	1
#define SGD_Z	2
#define SGD_W	3

#ifndef M_PI
#define SGD_PI 3.14159265358979323846   /* From M_PI under Linux/X86 */
#else
#define SGD_PI M_PI
#endif

#define SGD_DEGREES_TO_RADIANS  (SGD_PI/SGD_180)
#define SGD_RADIANS_TO_DEGREES  (SGD_180/SGD_PI)

/*
  These are just convenient redefinitions of standard
  math library functions to stop float/double warnings.
*/
   
inline SGDfloat sgdSqrt   ( const SGDfloat x ) { return sqrt ( x ) ; }
inline SGDfloat sgdSquare ( const SGDfloat x ) { return x * x ; }
inline SGDfloat sgdAbs    ( const SGDfloat a ) { return ( a < SGD_ZERO ) ? -a : a ; }

inline SGDfloat sgdASin ( SGDfloat s )
                { return (SGDfloat) asin ( s ) * SGD_RADIANS_TO_DEGREES ; }
inline SGDfloat sgdACos ( SGDfloat s )
                { return (SGDfloat) acos ( s ) * SGD_RADIANS_TO_DEGREES ; }
inline SGDfloat sgdATan ( SGDfloat s )
                { return (SGDfloat) atan ( s ) * SGD_RADIANS_TO_DEGREES ; }
inline SGDfloat sgdATan2 ( SGDfloat y, SGDfloat x )
                { return (SGDfloat) atan2 ( y,x ) * SGD_RADIANS_TO_DEGREES ; }
inline SGDfloat sgdSin ( SGDfloat s )
                { return (SGDfloat) sin ( s * SGD_DEGREES_TO_RADIANS ) ; }
inline SGDfloat sgdCos ( SGDfloat s )
                { return (SGDfloat) cos ( s * SGD_DEGREES_TO_RADIANS ) ; }
inline SGDfloat sgdTan ( SGDfloat s )
                { return (SGDfloat) tan ( s * SGD_DEGREES_TO_RADIANS ) ; }

inline int sgdCompareFloat ( const SGDfloat a, const SGDfloat b, const SGDfloat tol )
{
  if ( ( a + tol ) < b ) return -1 ;
  if ( ( b + tol ) < a ) return  1 ;
  return 0 ;
}


/*
  Types used in SGD.
*/

typedef SGDfloat sgdVec2 [ 2 ] ;
typedef SGDfloat sgdVec3 [ 3 ] ;
typedef SGDfloat sgdVec4 [ 4 ] ;

typedef sgdVec4 sgdQuat ;

typedef SGDfloat sgdMat3  [3][3] ;
typedef SGDfloat sgdMat4  [4][4] ;

struct sgdCoord
{
  sgdVec3 xyz ;
  sgdVec3 hpr ;
} ;

class sgdSphere ;
class sgdBox ;
class sgdFrustum ;

/*
  Some handy constants
*/

#define SGD_OUTSIDE  FALSE
#define SGD_INSIDE    TRUE
#define SGD_STRADDLE     2

inline SGDfloat sgdHeightOfPlaneVec2 ( const sgdVec4 plane, const sgdVec2 pnt )
{
  if ( plane[2] == SGD_ZERO )
    return SGD_ZERO ;
  else
    return -( plane[0] * pnt[0] + plane[1] * pnt[1] + plane[3] ) / plane[2] ;
}

/*
  Convert a direction vector into a set of euler angles,
  (with zero roll)
*/

extern void sgdHPRfromVec3 ( sgdVec3 hpr, const sgdVec3 src ) ;

extern void sgdMakeCoordMat4 ( sgdMat4 dst, const SGDfloat x, const SGDfloat y, const SGDfloat z,
                                            const SGDfloat h, const SGDfloat p, const SGDfloat r ) ;

inline void sgdMakeCoordMat4( sgdMat4 dst, const sgdVec3 xyz, const sgdVec3 hpr )
{
  sgdMakeCoordMat4 ( dst, xyz[0], xyz[1], xyz[2],
                         hpr[0], hpr[1], hpr[2] ) ;
}

inline void sgdMakeCoordMat4( sgdMat4 dst, const sgdCoord *src )
{
  sgdMakeCoordMat4 ( dst, src->xyz, src->hpr ) ;
}

extern void sgdMakeRotMat4   ( sgdMat4 dst, const SGDfloat angle, const sgdVec3 axis ) ;

inline void sgdMakeRotMat4   ( sgdMat4 dst, const sgdVec3 hpr )
{
  sgdMakeCoordMat4 ( dst, SGD_ZERO, SGD_ZERO, SGD_ZERO, hpr[0], hpr[1], hpr[2] ) ;
}

inline void sgdMakeRotMat4   ( sgdMat4 dst, const SGDfloat h, const SGDfloat p, const SGDfloat r )
{
  sgdMakeCoordMat4 ( dst, SGD_ZERO, SGD_ZERO, SGD_ZERO, h, p, r ) ;
}

extern void sgdMakeTransMat4 ( sgdMat4 dst, const sgdVec3 xyz ) ;
extern void sgdMakeTransMat4 ( sgdMat4 dst, const SGDfloat x, const SGDfloat y, const SGDfloat z ) ;


extern void sgdSetCoord      ( sgdCoord *coord, const sgdMat4 src ) ;

extern void sgdMultMat4      ( sgdMat4 dst, const sgdMat4 a, const sgdMat4 b ) ;
extern void sgdPostMultMat4  ( sgdMat4 dst, const sgdMat4 a ) ;
extern void sgdPreMultMat4   ( sgdMat4 dst, const sgdMat4 a ) ;

extern void sgdTransposeNegateMat4 ( sgdMat4 dst ) ;
extern void sgdTransposeNegateMat4 ( sgdMat4 dst, const sgdMat4 src ) ;

extern void sgdInvertMat4 ( sgdMat4 dst, const sgdMat4 src ) ;
inline void sgdInvertMat4 ( sgdMat4 dst ) { sgdInvertMat4 ( dst, dst ) ; }

extern void sgdXformVec3     ( sgdVec3 dst, const sgdVec3 src, const sgdMat4 mat ) ;
extern void sgdXformPnt3     ( sgdVec3 dst, const sgdVec3 src, const sgdMat4 mat ) ;
extern void sgdXformVec4     ( sgdVec4 dst, const sgdVec4 src, const sgdMat4 mat ) ;
extern void sgdXformPnt4     ( sgdVec4 dst, const sgdVec4 src, const sgdMat4 mat ) ;
extern void sgdFullXformPnt3 ( sgdVec3 dst, const sgdVec3 src, const sgdMat4 mat ) ;
extern void sgdFullXformPnt4 ( sgdVec4 dst, const sgdVec4 src, const sgdMat4 mat ) ;

inline void sgdXformVec3     ( sgdVec3 dst, const sgdMat4 mat ) { sgdXformVec3 ( dst, dst, mat ) ; }
inline void sgdXformPnt3     ( sgdVec3 dst, const sgdMat4 mat ) { sgdXformPnt3 ( dst, dst, mat ) ; }
inline void sgdXformVec4     ( sgdVec4 dst, const sgdMat4 mat ) { sgdXformVec4 ( dst, dst, mat ) ; }
inline void sgdXformPnt4     ( sgdVec4 dst, const sgdMat4 mat ) { sgdXformPnt4 ( dst, dst, mat ) ; }
inline void sgdFullXformPnt3 ( sgdVec3 dst, const sgdMat4 mat ) { sgdFullXformPnt3 ( dst, dst, mat ) ; }
inline void sgdFullXformPnt4 ( sgdVec4 dst, const sgdMat4 mat ) { sgdFullXformPnt4 ( dst, dst, mat ) ; }

/*
  Basic low-level vector functions.
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  For each of Vec2, Vec3 and Vec4, we provide inlines for

  Zero      - set all elements to zero.
  Set       - set each element individually.
  Add       - add vectors element by element.
  Sub       - subtract vectors element by element.
  Scale     - multiply each element of a vector by a variable.
  Negate    - negate each element of a vector.
  Compare   - compare vectors element by element with optional tolerance.
              (return TRUE if vectors are equal - within tolerances)
  Equal     - return TRUE if vectors are exactly equal.
  Length    - compute length of a vector.
  Distance  - compute distance between two points.
  LengthSquared   - compute the square of the length of a vector.
  DistanceSquared - compute the square of the distance between two points.
  ScalarProduct - scalar (dot) product.
  VectorProduct - vector (cross) product (3-element vectors ONLY!).
  Normalise/Normalize - make vector be one unit long.
  Lerp      - linear interpolation by a fraction 'f'.
*/

inline void sgdZeroVec2 ( sgdVec2 dst ) { dst[0]=dst[1]=SGD_ZERO ; }
inline void sgdZeroVec3 ( sgdVec3 dst ) { dst[0]=dst[1]=dst[2]=SGD_ZERO ; }
inline void sgdZeroVec4 ( sgdVec4 dst ) { dst[0]=dst[1]=dst[2]=dst[3]=SGD_ZERO ; }


inline void sgdSetVec2 ( sgdVec2 dst, const SGDfloat x, const SGDfloat y )
{
  dst [ 0 ] = x ;
  dst [ 1 ] = y ;
}

inline void sgdSetVec3 ( sgdVec3 dst, const SGDfloat x, const SGDfloat y, const SGDfloat z )
{
  dst [ 0 ] = x ;
  dst [ 1 ] = y ;
  dst [ 2 ] = z ;
}

inline void sgdSetVec4 ( sgdVec4 dst, const SGDfloat x, const SGDfloat y, const SGDfloat z, const SGDfloat w )
{
  dst [ 0 ] = x ;
  dst [ 1 ] = y ;
  dst [ 2 ] = z ;
  dst [ 3 ] = w ;
}


inline void sgdCopyVec2 ( sgdVec2 dst, const sgdVec2 src )
{
  dst [ 0 ] = src [ 0 ] ;
  dst [ 1 ] = src [ 1 ] ;
}

inline void sgdCopyVec3 ( sgdVec3 dst, const sgdVec3 src )
{
  dst [ 0 ] = src [ 0 ] ;
  dst [ 1 ] = src [ 1 ] ;
  dst [ 2 ] = src [ 2 ] ;
}

inline void sgdCopyVec4 ( sgdVec4 dst, const sgdVec4 src )
{
  dst [ 0 ] = src [ 0 ] ;
  dst [ 1 ] = src [ 1 ] ;
  dst [ 2 ] = src [ 2 ] ;
  dst [ 3 ] = src [ 3 ] ;
}


inline void sgdAddVec2 ( sgdVec2 dst, const sgdVec2 src )
{
  dst [ 0 ] += src [ 0 ] ;
  dst [ 1 ] += src [ 1 ] ;
}

inline void sgdAddVec3 ( sgdVec3 dst, const sgdVec3 src )
{
  dst [ 0 ] += src [ 0 ] ;
  dst [ 1 ] += src [ 1 ] ;
  dst [ 2 ] += src [ 2 ] ;
}

inline void sgdAddVec4 ( sgdVec4 dst, const sgdVec4 src )
{
  dst [ 0 ] += src [ 0 ] ;
  dst [ 1 ] += src [ 1 ] ;
  dst [ 2 ] += src [ 2 ] ;
  dst [ 3 ] += src [ 3 ] ;
}


inline void sgdAddVec2 ( sgdVec2 dst, const sgdVec2 src1, const sgdVec2 src2 )
{
  dst [ 0 ] = src1 [ 0 ] + src2 [ 0 ] ;
  dst [ 1 ] = src1 [ 1 ] + src2 [ 1 ] ;
}

inline void sgdAddVec3 ( sgdVec3 dst, const sgdVec3 src1, const sgdVec3 src2 )
{
  dst [ 0 ] = src1 [ 0 ] + src2 [ 0 ] ;
  dst [ 1 ] = src1 [ 1 ] + src2 [ 1 ] ;
  dst [ 2 ] = src1 [ 2 ] + src2 [ 2 ] ;
}

inline void sgdAddVec4 ( sgdVec4 dst, const sgdVec4 src1, const sgdVec4 src2 )
{
  dst [ 0 ] = src1 [ 0 ] + src2 [ 0 ] ;
  dst [ 1 ] = src1 [ 1 ] + src2 [ 1 ] ;
  dst [ 2 ] = src1 [ 2 ] + src2 [ 2 ] ;
  dst [ 3 ] = src1 [ 3 ] + src2 [ 3 ] ;
}


inline void sgdSubVec2 ( sgdVec2 dst, const sgdVec2 src )
{
  dst [ 0 ] -= src [ 0 ] ;
  dst [ 1 ] -= src [ 1 ] ;
}

inline void sgdSubVec3 ( sgdVec3 dst, const sgdVec3 src )
{
  dst [ 0 ] -= src [ 0 ] ;
  dst [ 1 ] -= src [ 1 ] ;
  dst [ 2 ] -= src [ 2 ] ;
}

inline void sgdSubVec4 ( sgdVec4 dst, const sgdVec4 src )
{
  dst [ 0 ] -= src [ 0 ] ;
  dst [ 1 ] -= src [ 1 ] ;
  dst [ 2 ] -= src [ 2 ] ;
  dst [ 3 ] -= src [ 3 ] ;
}

inline void sgdSubVec2 ( sgdVec2 dst, const sgdVec2 src1, const sgdVec2 src2 )
{
  dst [ 0 ] = src1 [ 0 ] - src2 [ 0 ] ;
  dst [ 1 ] = src1 [ 1 ] - src2 [ 1 ] ;
}

inline void sgdSubVec3 ( sgdVec3 dst, const sgdVec3 src1, const sgdVec3 src2 )
{
  dst [ 0 ] = src1 [ 0 ] - src2 [ 0 ] ;
  dst [ 1 ] = src1 [ 1 ] - src2 [ 1 ] ;
  dst [ 2 ] = src1 [ 2 ] - src2 [ 2 ] ;
}

inline void sgdSubVec4 ( sgdVec4 dst, const sgdVec4 src1, const sgdVec4 src2 )
{
  dst [ 0 ] = src1 [ 0 ] - src2 [ 0 ] ;
  dst [ 1 ] = src1 [ 1 ] - src2 [ 1 ] ;
  dst [ 2 ] = src1 [ 2 ] - src2 [ 2 ] ;
  dst [ 3 ] = src1 [ 3 ] - src2 [ 3 ] ;
}


inline void sgdNegateVec2 ( sgdVec2 dst )
{
  dst [ 0 ] = -dst [ 0 ] ;
  dst [ 1 ] = -dst [ 1 ] ;
}

inline void sgdNegateVec3 ( sgdVec3 dst )
{
  dst [ 0 ] = -dst [ 0 ] ;
  dst [ 1 ] = -dst [ 1 ] ;
  dst [ 2 ] = -dst [ 2 ] ;
}

inline void sgdNegateVec4 ( sgdVec4 dst )
{
  dst [ 0 ] = -dst [ 0 ] ;
  dst [ 1 ] = -dst [ 1 ] ;
  dst [ 2 ] = -dst [ 2 ] ;
  dst [ 3 ] = -dst [ 3 ] ;
}


inline void sgdNegateVec2 ( sgdVec2 dst, const sgdVec2 src )
{
  dst [ 0 ] = -src [ 0 ] ;
  dst [ 1 ] = -src [ 1 ] ;
}

inline void sgdNegateVec3 ( sgdVec3 dst, const sgdVec3 src )
{
  dst [ 0 ] = -src [ 0 ] ;
  dst [ 1 ] = -src [ 1 ] ;
  dst [ 2 ] = -src [ 2 ] ;
}

inline void sgdNegateVec4 ( sgdVec4 dst, const sgdVec4 src )
{
  dst [ 0 ] = -src [ 0 ] ;
  dst [ 1 ] = -src [ 1 ] ;
  dst [ 2 ] = -src [ 2 ] ;
  dst [ 3 ] = -src [ 3 ] ;
}


inline void sgdScaleVec2 ( sgdVec2 dst, const SGDfloat s )
{
  dst [ 0 ] *= s ;
  dst [ 1 ] *= s ;
}

inline void sgdScaleVec3 ( sgdVec3 dst, const SGDfloat s )
{
  dst [ 0 ] *= s ;
  dst [ 1 ] *= s ;
  dst [ 2 ] *= s ;
}

inline void sgdScaleVec4 ( sgdVec4 dst, const SGDfloat s )
{
  dst [ 0 ] *= s ;
  dst [ 1 ] *= s ;
  dst [ 2 ] *= s ;
  dst [ 3 ] *= s ;
}

inline void sgdScaleVec2 ( sgdVec2 dst, const sgdVec2 src, const SGDfloat s )
{
  dst [ 0 ] = src [ 0 ] * s ;
  dst [ 1 ] = src [ 1 ] * s ;
}

inline void sgdScaleVec3 ( sgdVec3 dst, const sgdVec3 src, const SGDfloat s )
{
  dst [ 0 ] = src [ 0 ] * s ;
  dst [ 1 ] = src [ 1 ] * s ;
  dst [ 2 ] = src [ 2 ] * s ;
}

inline void sgdScaleVec4 ( sgdVec4 dst, const sgdVec4 src, const SGDfloat s )
{
  dst [ 0 ] = src [ 0 ] * s ;
  dst [ 1 ] = src [ 1 ] * s ;
  dst [ 2 ] = src [ 2 ] * s ;
  dst [ 3 ] = src [ 3 ] * s ;
}


inline void sgdAddScaledVec2 ( sgdVec2 dst, const sgdVec2 src, const SGDfloat s )
{
  dst [ 0 ] += src [ 0 ] * s ;
  dst [ 1 ] += src [ 1 ] * s ;
}

inline void sgdAddScaledVec3 ( sgdVec3 dst, const sgdVec3 src, const SGDfloat s )
{
  dst [ 0 ] += src [ 0 ] * s ;
  dst [ 1 ] += src [ 1 ] * s ;
  dst [ 2 ] += src [ 2 ] * s ;
}

inline void sgdAddScaledVec4 ( sgdVec4 dst, const sgdVec4 src, const SGDfloat s )
{
  dst [ 0 ] += src [ 0 ] * s ;
  dst [ 1 ] += src [ 1 ] * s ;
  dst [ 2 ] += src [ 2 ] * s ;
  dst [ 3 ] += src [ 3 ] * s ;
}


inline void sgdAddScaledVec2 ( sgdVec2 dst, const sgdVec2 src1, const sgdVec2 src2, const SGDfloat s )
{
  dst [ 0 ] = src1 [ 0 ] + src2 [ 0 ] * s ;
  dst [ 1 ] = src1 [ 1 ] + src2 [ 1 ] * s ;
}

inline void sgdAddScaledVec3 ( sgdVec3 dst, const sgdVec3 src1, const sgdVec3 src2, const SGDfloat s )
{
  dst [ 0 ] = src1 [ 0 ] + src2 [ 0 ] * s ;
  dst [ 1 ] = src1 [ 1 ] + src2 [ 1 ] * s ;
  dst [ 2 ] = src1 [ 2 ] + src2 [ 2 ] * s ;
}

inline void sgdAddScaledVec4 ( sgdVec4 dst, const sgdVec4 src1, const sgdVec4 src2, const SGDfloat s )
{
  dst [ 0 ] = src1 [ 0 ] + src2 [ 0 ] * s ;
  dst [ 1 ] = src1 [ 1 ] + src2 [ 1 ] * s ;
  dst [ 2 ] = src1 [ 2 ] + src2 [ 2 ] * s ;
  dst [ 3 ] = src1 [ 3 ] + src2 [ 3 ] * s ;
}



inline int sgdCompareVec2 ( const sgdVec2 a, const sgdVec2 b, const SGDfloat tol )
{
  int val = 0 ;

  if ( ( val = sgdCompareFloat( a[0], b[0], tol ) ) != 0 ) return val ;
  if ( ( val = sgdCompareFloat( a[1], b[1], tol ) ) != 0 ) return val ;

  return 0 ;
}

inline int sgdCompareVec3 ( const sgdVec3 a, const sgdVec3 b, const SGDfloat tol )
{
  int val = 0 ;

  if ( ( val = sgdCompareFloat( a[0], b[0], tol ) ) != 0 ) return val ;
  if ( ( val = sgdCompareFloat( a[1], b[1], tol ) ) != 0 ) return val ;
  if ( ( val = sgdCompareFloat( a[2], b[2], tol ) ) != 0 ) return val ;

  return 0 ;
}

inline int sgdCompareVec4 ( const sgdVec4 a, const sgdVec4 b, const SGDfloat tol )
{
  int val = 0 ;

  if ( ( val = sgdCompareFloat( a[0], b[0], tol ) ) != 0 ) return val ;
  if ( ( val = sgdCompareFloat( a[1], b[1], tol ) ) != 0 ) return val ;
  if ( ( val = sgdCompareFloat( a[2], b[2], tol ) ) != 0 ) return val ;
  if ( ( val = sgdCompareFloat( a[3], b[3], tol ) ) != 0 ) return val ;

  return 0 ;
}


inline int sgdEqualVec2 ( const sgdVec2 a, const sgdVec2 b )
{
  return a[0] == b[0] &&
         a[1] == b[1] ;
}

inline int sgdEqualVec3 ( const sgdVec3 a, const sgdVec3 b )
{
  return a[0] == b[0] &&
         a[1] == b[1] &&
         a[2] == b[2] ;
}

inline int sgdEqualVec4 ( const sgdVec4 a, const sgdVec4 b )
{
  return a[0] == b[0] &&
         a[1] == b[1] &&
         a[2] == b[2] &&
         a[3] == b[3] ;
}


inline SGDfloat sgdScalarProductVec2 ( const sgdVec2 a, const sgdVec2 b )
{
  return a[0]*b[0] + a[1]*b[1] ;
}

inline SGDfloat sgdScalarProductVec3 ( const sgdVec3 a, const sgdVec3 b )
{
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] ;
}

inline SGDfloat sgdScalarProductVec4 ( const sgdVec4 a, const sgdVec4 b )
{
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3] ;
}


extern void sgdVectorProductVec3 ( sgdVec3 dst, const sgdVec3 a, const sgdVec3 b ) ;


inline void sgdLerpVec4 ( sgdVec4 dst, const sgdVec4 a, const sgdVec4 b, const SGDfloat f )
{
  dst[0] = a[0] + f * ( b[0] - a[0] ) ;
  dst[1] = a[1] + f * ( b[1] - a[1] ) ;
  dst[2] = a[2] + f * ( b[2] - a[2] ) ;
  dst[3] = a[3] + f * ( b[3] - a[3] ) ;
}

inline void sgdLerpVec3 ( sgdVec3 dst, const sgdVec3 a, const sgdVec3 b, const SGDfloat f )
{
  dst[0] = a[0] + f * ( b[0] - a[0] ) ;
  dst[1] = a[1] + f * ( b[1] - a[1] ) ;
  dst[2] = a[2] + f * ( b[2] - a[2] ) ;
}

inline void sgdLerpVec2 ( sgdVec2 dst, const sgdVec2 a, const sgdVec2 b, const SGDfloat f )
{
  dst[0] = a[0] + f * ( b[0] - a[0] ) ;
  dst[1] = a[1] + f * ( b[1] - a[1] ) ;
}

inline void sgdLerpAnglesVec3 ( sgdVec3 dst, const sgdVec3 a,
                                             const sgdVec3 b,
                                             const SGDfloat f )
{
  sgdVec3 tmp ;
 
  if ( b[0] - a[0] >  180.0 ) tmp[0] = a[0] + 360.0 ; else
  if ( b[0] - a[0] < -180.0 ) tmp[0] = a[0] - 360.0 ; else tmp[0] = a[0] ;
 
  if ( b[1] - a[1] >  180.0 ) tmp[1] = a[1] + 360.0 ; else
  if ( b[1] - a[1] < -180.0 ) tmp[1] = a[1] - 360.0 ; else tmp[1] = a[1] ;
 
  if ( b[2] - a[2] >  180.0 ) tmp[2] = a[2] + 360.0 ; else
  if ( b[2] - a[2] < -180.0 ) tmp[2] = a[2] - 360.0 ; else tmp[2] = a[2] ;
 
  dst[0] = tmp[0] + f * ( b[0] - tmp[0] ) ;
  dst[1] = tmp[1] + f * ( b[1] - tmp[1] ) ;
  dst[2] = tmp[2] + f * ( b[2] - tmp[2] ) ;
}                                                                               




inline SGDfloat sgdDistanceSquaredVec2 ( const sgdVec2 a, const sgdVec2 b )
{
  return sgdSquare ( a[0]-b[0] ) + sgdSquare ( a[1]-b[1] ) ;
}

inline SGDfloat sgdDistanceSquaredVec3 ( const sgdVec3 a, const sgdVec3 b )
{
  return sgdSquare ( a[0]-b[0] ) + sgdSquare ( a[1]-b[1] ) +
         sgdSquare ( a[2]-b[2] ) ;
}

inline SGDfloat sgdDistanceSquaredVec4 ( const sgdVec4 a, const sgdVec4 b )
{
  return sgdSquare ( a[0]-b[0] ) + sgdSquare ( a[1]-b[1] ) +
         sgdSquare ( a[2]-b[2] ) + sgdSquare ( a[3]-b[3] ) ;
}


inline SGDfloat sgdDistanceVec2 ( const sgdVec2 a, const sgdVec2 b )
{
  return sgdSqrt ( sgdSquare ( a[0]-b[0] ) + sgdSquare ( a[1]-b[1] ) ) ;
}

inline SGDfloat sgdDistanceVec3 ( const sgdVec3 a, const sgdVec3 b )
{
  return sgdSqrt ( sgdSquare ( a[0]-b[0] ) + sgdSquare ( a[1]-b[1] ) +
                  sgdSquare ( a[2]-b[2] ) ) ;
}

inline SGDfloat sgdDistanceVec4 ( const sgdVec4 a, const sgdVec4 b )
{
  return sgdSqrt ( sgdSquare ( a[0]-b[0] ) + sgdSquare ( a[1]-b[1] ) +
                  sgdSquare ( a[2]-b[2] ) + sgdSquare ( a[3]-b[3] ) ) ;
}


inline SGDfloat sgdLengthVec2 ( sgdVec2 const src )
{
  return sgdSqrt ( sgdScalarProductVec2 ( src, src ) ) ;
}

inline SGDfloat sgdLengthVec3 ( sgdVec3 const src )
{
  return sgdSqrt ( sgdScalarProductVec3 ( src, src ) ) ;
}

inline SGDfloat sgdLengthVec4 ( sgdVec4 const src )
{
  return sgdSqrt ( sgdScalarProductVec4 ( src, src ) ) ;
}

inline SGDfloat sgdLengthSquaredVec2 ( sgdVec2 const src )
{
  return sgdScalarProductVec2 ( src, src ) ;
}

inline SGDfloat sgdLengthSquaredVec3 ( sgdVec3 const src )
{
  return sgdScalarProductVec3 ( src, src ) ;
}

inline SGDfloat sgdLengthSquaredVec4 ( sgdVec4 const src )
{
  return sgdScalarProductVec4 ( src, src ) ;
}

/* Anglo-US spelling issues.  <sigh> */
#define sgdNormalizeVec2 sgdNormaliseVec2
#define sgdNormalizeVec3 sgdNormaliseVec3
#define sgdNormalizeVec4 sgdNormaliseVec4
#define sgdNormalizeQuat sgdNormaliseQuat

inline void sgdNormaliseVec2 ( sgdVec2 dst )
{
  sgdScaleVec2 ( dst, SGD_ONE / sgdLengthVec2 ( dst ) ) ;
}

inline void sgdNormaliseVec3 ( sgdVec3 dst )
{
  sgdScaleVec3 ( dst, SGD_ONE / sgdLengthVec3 ( dst ) ) ;
}

inline void sgdNormaliseVec4 ( sgdVec4 dst )
{
  sgdScaleVec4 ( dst, SGD_ONE / sgdLengthVec4 ( dst ) ) ;
}

inline void sgdNormaliseVec2 ( sgdVec2 dst, const sgdVec2 src )
{
  sgdScaleVec2 ( dst, src, SGD_ONE / sgdLengthVec2 ( src ) ) ;
}

inline void sgdNormaliseVec3 ( sgdVec3 dst, const sgdVec3 src )
{
  sgdScaleVec3 ( dst, src, SGD_ONE / sgdLengthVec3 ( src ) ) ;
}

inline void sgdNormaliseVec4 ( sgdVec4 dst, const sgdVec4 src )
{
  sgdScaleVec4 ( dst, src, SGD_ONE / sgdLengthVec4 ( src ) ) ;
}


inline void sgdZeroCoord ( sgdCoord *dst )
{
  sgdSetVec3 ( dst->xyz, SGD_ZERO, SGD_ZERO, SGD_ZERO ) ;
  sgdSetVec3 ( dst->hpr, SGD_ZERO, SGD_ZERO, SGD_ZERO ) ;
}

inline void sgdSetCoord ( sgdCoord *dst, const SGDfloat x, const SGDfloat y, const SGDfloat z,
                                         const SGDfloat h, const SGDfloat p, const SGDfloat r )
{
  sgdSetVec3 ( dst->xyz, x, y, z ) ;
  sgdSetVec3 ( dst->hpr, h, p, r ) ;
}

inline void sgdSetCoord ( sgdCoord *dst, const sgdVec3 xyz, const sgdVec3 hpr )
{
  sgdCopyVec3 ( dst->xyz, xyz ) ;
  sgdCopyVec3 ( dst->hpr, hpr ) ;
}

inline void sgdCopyCoord ( sgdCoord *dst, const sgdCoord *src )
{
  sgdCopyVec3 ( dst->xyz, src->xyz ) ;
  sgdCopyVec3 ( dst->hpr, src->hpr ) ;
}



inline void sgdCopyMat4 ( sgdMat4 dst, const sgdMat4 src )
{
  sgdCopyVec4 ( dst[ 0 ], src[ 0 ] ) ;
  sgdCopyVec4 ( dst[ 1 ], src[ 1 ] ) ;
  sgdCopyVec4 ( dst[ 2 ], src[ 2 ] ) ;
  sgdCopyVec4 ( dst[ 3 ], src[ 3 ] ) ;
}


inline void sgdScaleMat4 ( sgdMat4 dst, const sgdMat4 src, const SGDfloat scale )
{
  sgdScaleVec4 ( dst[0], src[0], scale ) ;
  sgdScaleVec4 ( dst[1], src[1], scale ) ;
  sgdScaleVec4 ( dst[2], src[2], scale ) ;
  sgdScaleVec4 ( dst[3], src[3], scale ) ;
}
 

inline void sgdMakeIdentMat4 ( sgdMat4 dst )
{
  sgdSetVec4 ( dst[0], SGD_ONE , SGD_ZERO, SGD_ZERO, SGD_ZERO ) ;
  sgdSetVec4 ( dst[1], SGD_ZERO, SGD_ONE , SGD_ZERO, SGD_ZERO ) ;
  sgdSetVec4 ( dst[2], SGD_ZERO, SGD_ZERO, SGD_ONE , SGD_ZERO ) ;
  sgdSetVec4 ( dst[3], SGD_ZERO, SGD_ZERO, SGD_ZERO, SGD_ONE  ) ;
}

extern int  sgdCompare3DSqdDist ( const sgdVec3 a, const sgdVec3 b, const SGDfloat sqd_dist ) ;
extern void sgdMakeTransMat4 ( sgdMat4 m, const SGDfloat x, const SGDfloat y, const SGDfloat z ) ;
extern void sgdMakeTransMat4 ( sgdMat4 m, const sgdVec3 xyz ) ;
extern void sgdMakeCoordMat4 ( sgdMat4 m, const SGDfloat x, const SGDfloat y, const SGDfloat z,
                                          const SGDfloat h, const SGDfloat p, const SGDfloat r ) ;
extern void sgdMakeCoordMat4 ( sgdMat4 m, const sgdCoord *c ) ;

inline SGDfloat sgdDistToLineVec2 ( const sgdVec3 line, const sgdVec2 pnt )
{
  return sgdScalarProductVec2 ( line, pnt ) + line[2] ;
}
 
inline SGDfloat sgdDistToPlaneVec3 ( const sgdVec4 plane, const sgdVec3 pnt )
{
  return sgdScalarProductVec3 ( plane, pnt ) + plane[3] ;
}
 
inline SGDfloat sgdHeightAbovePlaneVec3 ( const sgdVec4 plane, const sgdVec3 pnt )
{
  return pnt[2] - sgdHeightOfPlaneVec2 ( plane, pnt ) ;
}

extern void sgdReflectInPlaneVec3 ( sgdVec3 dst, const sgdVec3 src, const sgdVec4 plane ) ;
 
inline void sgdReflectInPlaneVec3 ( sgdVec3 dst, const sgdVec4 plane ) 
{
  sgdReflectInPlaneVec3 ( dst, dst, plane ) ;
}

extern void sgdMakeNormal    ( sgdVec3 dst, const sgdVec3 a, const sgdVec3 b, const sgdVec3 c ) ;

inline void sgdMake2DLine ( sgdVec3 dst, const sgdVec2 a, const sgdVec2 b )
{
  dst[0] =  b[1]-a[1] ;
  dst[1] = -b[0]-a[0] ;
  sgdNormalizeVec2 ( dst ) ;
  dst[2] = - ( dst[0]*a[0] + dst[1]*a[1] ) ;
}

inline void sgdMakePlane ( sgdVec4 dst, const sgdVec3 normal, const sgdVec3 pnt )
{
  sgdCopyVec3 ( dst, normal ) ;
  dst [ 3 ] = - sgdScalarProductVec3 ( normal, pnt ) ;
}

inline void sgdMakePlane ( sgdVec4 dst, const sgdVec3 a, const sgdVec3 b, const sgdVec3 c )
{
  /*
    Ax + By + Cz + D == 0 ;
    D = - ( Ax + By + Cz )
      = - ( A*a[0] + B*a[1] + C*a[2] )
      = - sgdScalarProductVec3 ( normal, a ) ;
  */

  sgdMakeNormal ( dst, a, b, c ) ;

  dst [ 3 ] = - sgdScalarProductVec3 ( dst, a ) ;
}




class sgdSphere
{
  sgdVec3  center ;
  SGDfloat radius ;
public:

  const SGDfloat *getCenter (void) const { return center ; }

  void setCenter ( const sgdVec3 c )
  {
    sgdCopyVec3 ( center, c ) ;
  }

  void setCenter ( const SGDfloat x, const SGDfloat y, const SGDfloat z )
  {
    sgdSetVec3 ( center, x, y, z ) ;
  }

  SGDfloat getRadius (void) const         { return radius ; }
  void     setRadius ( const SGDfloat r ) { radius = r ; }

  int isEmpty (void) const { return radius <  SGD_ZERO ; }
  void empty  (void)       { radius = - SGD_ONE ; }

  void orthoXform ( const sgdMat4 m )
  {
    sgdXformPnt3 ( center, center, m ) ;
    // radius *= sgdLengthVec3 ( m[0] ) ;
  }

  void extend ( const sgdSphere *s ) ;
  void extend ( const sgdBox    *b ) ;
  void extend ( const sgdVec3    v ) ;

  int intersects ( const sgdSphere *s ) const 
  {
    return sgdCompare3DSqdDist ( center, s->getCenter(),
                    sgdSquare ( radius + s->getRadius() ) ) <= 0 ;
  }

  int intersects ( const sgdVec4 plane ) const 
  {
    return sgdAbs ( sgdDistToPlaneVec3 ( plane, center ) ) <= radius ;
  }

  int intersects ( const sgdBox *b ) const ;
} ;


class sgdBox
{
  sgdVec3 min ;
  sgdVec3 max ;

public:

  const SGDfloat *getMin (void) const { return min ; }
  const SGDfloat *getMax (void) const { return max ; }

  void setMin ( const SGDfloat x, const SGDfloat y, const SGDfloat z )
  { 
    sgdSetVec3 ( min, x, y, z ) ;
  }

  void setMin ( const sgdVec3 src )
  { 
    sgdCopyVec3 ( min, src ) ;
  }

  void setMax ( const SGDfloat x, const SGDfloat y, const SGDfloat z )
  { 
    sgdSetVec3 ( max, x, y, z ) ;
  }

  void setMax ( const sgdVec3 src )
  { 
    sgdCopyVec3 ( max, src ) ;
  }

  int isEmpty(void) const 
  {
    return ( min[0] > max[0] ||
             min[1] > max[1] ||
             min[2] > max[2] ) ;
  }

  void empty (void)
  {
    sgdSetVec3 ( min,  SGD_MAX,  SGD_MAX,  SGD_MAX ) ;
    sgdSetVec3 ( max, -SGD_MAX, -SGD_MAX, -SGD_MAX ) ;
  }

  void extend  ( const sgdSphere *s ) ;
  void extend  ( const sgdBox    *b ) ;
  void extend  ( const sgdVec3    v ) ;

  int intersects ( const sgdSphere *s ) const 
  {
    return s -> intersects ( this ) ;
  }

  int intersects ( const sgdBox *b ) const 
  {
    return min[0] <= b->getMax()[0] && max[0] >= b->getMin()[0] &&
           min[1] <= b->getMax()[1] && max[1] >= b->getMin()[1] &&
           min[2] <= b->getMax()[2] && max[2] >= b->getMin()[2] ;
  }

  int intersects ( const sgdVec4 plane ) const ;
} ;

#define SGD_NEAR       0x10
#define SGD_FAR        0x20
#define SGD_TOP        0x30
#define SGD_BOT        0x40
#define SGD_LEFT       0x50
#define SGD_RIGHT      0x60

class sgdFrustum
{
  /* The parameters for a glFrustum or pfMakePerspFrust */
  
  SGDfloat left, right, top, bot, nnear, ffar ;

  /* The A,B,C terms of the plane equations of the four sloping planes */

  sgdVec3 top_plane, bot_plane, left_plane, right_plane ;

  /* A GL/PF-style perspective matrix for this frustum */

  sgdMat4 mat ;

  /* These two are only valid for simple frusta */

  SGDfloat hfov ;    /* Horizontal Field of View */
  SGDfloat vfov ;    /* Vertical   Field of View */

  void update (void) ;
  int getOutcode ( const sgdVec4 src ) const ;

public:

  sgdFrustum (void)
  {
    nnear = SGD_ONE ;
    ffar  = 1000000.0f ;
    hfov  = SGD_45 ;
    vfov  = SGD_45 ;
    update () ;
  }

  void setFrustum ( const SGDfloat l, const SGDfloat r,
                    const SGDfloat b, const SGDfloat t,
                    const SGDfloat n, const SGDfloat f )
  {
    left  = l ; right = r ;
    top   = t ; bot   = b ;
    nnear = n ; ffar  = f ;
    hfov = vfov = SGD_ZERO ;
    update () ;
  } 

  SGDfloat  getHFOV (void) const { return hfov  ; }
  SGDfloat  getVFOV (void) const { return vfov  ; }
  SGDfloat  getNear (void) const { return nnear ; }
  SGDfloat  getFar  (void) const { return ffar  ; }
  SGDfloat  getLeft (void) const { return left  ; }
  SGDfloat  getRight(void) const { return right ; }
  SGDfloat  getTop  (void) const { return top   ; }
  SGDfloat  getBot  (void) const { return bot   ; }

  void getFOV ( SGDfloat *h, SGDfloat *v ) const 
  {
    if ( h != (SGDfloat *) 0 ) *h = hfov ;
    if ( v != (SGDfloat *) 0 ) *v = vfov ;
  }

  void setFOV ( const SGDfloat h, const SGDfloat v )
  {
    hfov = ( h <= 0 ) ? ( v * SGD_THREE / SGD_TWO ) : h ;
    vfov = ( v <= 0 ) ? ( h * SGD_TWO / SGD_THREE ) : v ;
    update () ;
  }

  void getNearFar ( SGDfloat *n, SGDfloat *f ) const 
  {
    if ( n != (SGDfloat *) 0 ) *n = nnear ;
    if ( f != (SGDfloat *) 0 ) *f = ffar  ;
  }

  void setNearFar ( const SGDfloat n, const SGDfloat f )
  {
    nnear = n ;
    ffar  = f ;
    update () ;
  }

  int  contains ( const sgdVec3 p ) const ;
  int  contains ( const sgdSphere *s ) const ;
} ;


/*
  Quaternion routines are Copyright (C) 1999
  Kevin B. Thompson <kevinbthompson@yahoo.com>
  Modified by Sylvan W. Clebsch <sylvan@stanford.edu>
  Largely rewritten by "Negative0" <negative0@earthlink.net>
*/

/*
  Quaternion structure  w = real, (x, y, z) = vector
  CHANGED sqQuat to float array so that syntax matches 
  vector and matrix routines
*/


inline void sgdMakeIdentQuat ( sgdQuat dst )
{
  sgdSetVec4 ( dst, SGD_ZERO, SGD_ZERO, SGD_ZERO, SGD_ONE ) ;
}


inline void sgdSetQuat ( sgdQuat dst,
                        const SGDfloat w, const SGDfloat x,
                        const SGDfloat y, const SGDfloat z )
{
  sgdSetVec4 ( dst, x, y, z, w ) ;
}

inline void sgdCopyQuat ( sgdQuat dst, const sgdQuat src )
{
  sgdCopyVec4 ( dst, src ) ;
}


/* Construct a unit quaternion (length==1) */

inline void sgdNormaliseQuat ( sgdQuat dst, const sgdQuat src )
{
  SGDfloat d = sgdScalarProductVec4 ( src, src ) ;

  d = (d > SGD_ZERO) ? (SGD_ONE / sgdSqrt ( d )) : SGD_ONE ;

  sgdScaleVec4 ( dst, src, d ) ;
}



inline void sgdNormaliseQuat ( sgdQuat dst ) { sgdNormaliseQuat ( dst, dst ) ; }


inline void sgdInvertQuat ( sgdQuat dst, const sgdQuat src )
{
  SGDfloat d = sgdScalarProductVec4 ( src, src ) ;

  d = ( d == SGD_ZERO ) ? SGD_ONE : ( SGD_ONE / d ) ;

  dst[SG_W] =  src[SG_W] * d ;
  dst[SG_X] = -src[SG_X] * d ;
  dst[SG_Y] = -src[SG_Y] * d ;
  dst[SG_Z] = -src[SG_Z] * d ;
}

inline void sgdInvertQuat ( sgdQuat dst ) { sgdInvertQuat ( dst, dst ) ; }


/* Make an angle and axis of rotation from a Quaternion. */

void sgdQuatToAngleAxis ( SGDfloat *angle, sgdVec3 axis, const sgdQuat src ) ;
void sgdQuatToAngleAxis ( SGDfloat *angle,
                         SGDfloat *x, SGDfloat *y, SGDfloat *z,
                         const sgdQuat src ) ;

/* Make a quaternion from a given angle and axis of rotation */

void sgdAngleAxisToQuat ( sgdQuat dst,
                          const SGDfloat angle, const sgdVec3 axis ) ;
void sgdAngleAxisToQuat ( sgdQuat dst,
                         const SGDfloat angle,
                         const SGDfloat x, const SGDfloat y, const SGDfloat z );

/* Convert a matrix to/from a quat */

void sgdMatrixToQuat ( sgdQuat quat, const sgdMat4 m ) ;
void sgdQuatToMatrix ( sgdMat4 m, const sgdQuat quat ) ;

/* Convert a set of eulers to/from a quat */

void sgdQuatToEuler( sgdVec3 hpr, const sgdQuat quat ) ;
void sgdEulerToQuat( sgdQuat quat, const sgdVec3 hpr ) ;

inline void sgdEulerToQuat( sgdQuat dst,
                            SGDfloat h, SGDfloat p, SGDfloat r )
{
  sgdVec3 hpr ;

  sgdSetVec3 ( hpr, h, p, r ) ;

  sgdEulerToQuat( dst, hpr ) ;
}

inline void sgdHPRToQuat ( sgdQuat dst, SGDfloat h, SGDfloat p, SGDfloat r )
{
  sgdVec3 hpr;

  hpr[0] = h * SGD_DEGREES_TO_RADIANS ;
  hpr[1] = p * SGD_DEGREES_TO_RADIANS ;
  hpr[2] = r * SGD_DEGREES_TO_RADIANS ;

  sgdEulerToQuat( dst, hpr ) ;
}

inline void sgdHPRToQuat ( sgdQuat dst, const sgdVec3 hpr )
{
  sgdVec3 tmp ;

  sgdScaleVec3 ( tmp, hpr, SGD_DEGREES_TO_RADIANS ) ;

  sgdEulerToQuat ( dst, tmp ) ;
}

/* Multiply quaternions together (concatenate rotations) */

void sgdMultQuat ( sgdQuat dst, const sgdQuat a, const sgdQuat b ) ;

inline void sgdPostMultQuat ( sgdQuat dst, const sgdQuat q )
{
  sgdQuat r ;

  sgdCopyQuat ( r, dst ) ;
  sgdMultQuat ( dst, r, q ) ;
}

inline void sgdPreMultQuat ( sgdQuat dst, const sgdQuat q )
{
  sgdQuat r ;

  sgdCopyQuat ( r, dst ) ;
  sgdMultQuat ( dst, q, r ) ;
}


/* Rotate a quaternion by a given angle and axis (convenience function) */


inline void sgdRotQuat ( sgdQuat dst, const SGDfloat angle, const sgdVec3 axis )
{
  sgdQuat q ;

  sgdAngleAxisToQuat ( q, angle, axis ) ;
  sgdPostMultQuat ( dst, q ) ;
  sgdNormaliseQuat ( dst ) ;
}


inline void sgdRotQuat ( sgdQuat dst,
                        const SGDfloat angle,
                        const SGDfloat x, const SGDfloat y, const SGDfloat z )
{
  sgdVec3 axis ;

  sgdSetVec3 ( axis, x, y, z ) ;
  sgdRotQuat ( dst, angle, axis ) ;
}

/* SWC - Interpolate between to quaternions */

extern void sgdSlerpQuat ( sgdQuat dst,
                          const sgdQuat from, const sgdQuat to,
                          const SGDfloat t ) ;


/* Conversions between sg and sgd types. */

inline void sgSetVec2 ( sgVec2 dst, sgdVec2 src )
{
  dst [ 0 ] = (SGfloat) src [ 0 ] ;
  dst [ 1 ] = (SGfloat) src [ 1 ] ;
}

inline void sgSetVec3 ( sgVec3 dst, sgdVec3 src )
{
  dst [ 0 ] = (SGfloat) src [ 0 ] ;
  dst [ 1 ] = (SGfloat) src [ 1 ] ;
  dst [ 2 ] = (SGfloat) src [ 2 ] ;
}

inline void sgSetVec4 ( sgVec4 dst, sgdVec4 src )
{
  dst [ 0 ] = (SGfloat) src [ 0 ] ;
  dst [ 1 ] = (SGfloat) src [ 1 ] ;
  dst [ 2 ] = (SGfloat) src [ 2 ] ;
  dst [ 3 ] = (SGfloat) src [ 3 ] ;
}

inline void sgdSetVec2 ( sgdVec2 dst, sgVec2 src )
{
  dst [ 0 ] = (SGDfloat) src [ 0 ] ;
  dst [ 1 ] = (SGDfloat) src [ 1 ] ;
}

inline void sgdSetVec3 ( sgdVec3 dst, sgVec3 src )
{
  dst [ 0 ] = (SGDfloat) src [ 0 ] ;
  dst [ 1 ] = (SGDfloat) src [ 1 ] ;
  dst [ 2 ] = (SGDfloat) src [ 2 ] ;
}

inline void sgdSetVec4 ( sgdVec4 dst, sgVec4 src )
{
  dst [ 0 ] = (SGDfloat) src [ 0 ] ;
  dst [ 1 ] = (SGDfloat) src [ 1 ] ;
  dst [ 2 ] = (SGDfloat) src [ 2 ] ;
  dst [ 3 ] = (SGDfloat) src [ 3 ] ;
}


inline void sgSetMat4 ( sgMat4 dst, sgdMat4 src )
{
  sgSetVec4 ( dst [ 0 ], src [ 0 ] ) ;
  sgSetVec4 ( dst [ 1 ], src [ 1 ] ) ;
  sgSetVec4 ( dst [ 2 ], src [ 2 ] ) ;
  sgSetVec4 ( dst [ 3 ], src [ 3 ] ) ;
}


inline void sgdSetMat4 ( sgdMat4 dst, sgMat4 src )
{
  sgdSetVec4 ( dst [ 0 ], src [ 0 ] ) ;
  sgdSetVec4 ( dst [ 1 ], src [ 1 ] ) ;
  sgdSetVec4 ( dst [ 2 ], src [ 2 ] ) ;
  sgdSetVec4 ( dst [ 3 ], src [ 3 ] ) ;
}


inline void sgSetCoord ( sgCoord *dst, sgdCoord *src )
{
  sgSetVec3 ( dst->xyz, src->xyz ) ;
  sgSetVec3 ( dst->hpr, src->hpr ) ;
}


inline void sgdSetCoord ( sgdCoord *dst, sgCoord *src )
{
  sgdSetVec3 ( dst->xyz, src->xyz ) ;
  sgdSetVec3 ( dst->hpr, src->hpr ) ;
}

inline void sgSetQuat ( sgQuat dst, sgdQuat src )
{
  sgSetVec4 ( dst, src ) ;
}



inline void sgdSetQuat ( sgdQuat dst, sgQuat src )
{
  sgdSetVec4 ( dst, src ) ;
}


/*
  Intersection testing.
*/

int sgdIsectPlanePlane        ( sgdVec3 point, sgdVec3 dir,
				    sgdVec4 plane1, sgdVec4 plane2 ) ;
int sgdIsectInfLinePlane      ( sgdVec3 dst,
				    sgdVec3 l_org, sgdVec3 l_vec,
				    sgdVec4 plane ) ;
int sgdIsectInfLineInfLine    ( sgdVec3 dst,
				    sgdVec3 l1_org, sgdVec3 l1_vec,
				    sgdVec3 l2_org, sgdVec3 l2_vec ) ;
SGDfloat sgdIsectLinesegPlane ( sgdVec3 dst,
				    sgdVec3 v1, sgdVec3 v2,
				    sgdVec4 plane ) ;
bool sgdPointInTriangle       ( sgdVec3 point, sgdVec3 tri[3] );


/*
  TRIANGLE SOLVERS - These work for any triangle.

  SSS  == Side-lengths for all three sides.
  SAS  == Side-lengths for two sides - plus the angle between them.
  ASA  == Two angles plus the length of the Side between them.
  Area == The area of the triangle.
*/

SGfloat sgTriangleSolver_ASAtoArea ( SGfloat angA, SGfloat lenB, SGfloat angC );
SGfloat sgTriangleSolver_SAStoArea ( SGfloat lenA, SGfloat angB, SGfloat lenC );
SGfloat sgTriangleSolver_SSStoArea ( SGfloat lenA, SGfloat lenB, SGfloat lenC );
SGfloat sgTriangleSolver_SAAtoArea ( SGfloat lenA, SGfloat angB, SGfloat angA );
SGfloat sgTriangleSolver_ASStoArea ( SGfloat angB, SGfloat lenA, SGfloat lenB,
                                     int angA_is_obtuse );

void sgTriangleSolver_SSStoAAA ( SGfloat  lenA, SGfloat  lenB, SGfloat  lenC, 
                                 SGfloat *angA, SGfloat *angB, SGfloat *angC ) ;
void sgTriangleSolver_SAStoASA ( SGfloat  lenA, SGfloat  angB, SGfloat  lenC,
                                 SGfloat *angA, SGfloat *lenB, SGfloat *angC ) ;
void sgTriangleSolver_ASAtoSAS ( SGfloat  angA, SGfloat  lenB, SGfloat  angC,
                                 SGfloat *lenA, SGfloat *angB, SGfloat *lenC ) ;
void sgTriangleSolver_SAAtoASS ( SGfloat  lenA, SGfloat  angB, SGfloat  angA,
                                 SGfloat *angC, SGfloat *lenB, SGfloat *lenC ) ;
void sgTriangleSolver_ASStoSAA ( SGfloat  angB, SGfloat  lenA, SGfloat  lenB,
                                 int angA_is_obtuse,
                                 SGfloat *lenC, SGfloat *angA, SGfloat *angC ) ;


SGDfloat sgdTriangleSolver_ASAtoArea(SGDfloat angA,SGDfloat lenB,SGDfloat angC);
SGDfloat sgdTriangleSolver_SAStoArea(SGDfloat lenA,SGDfloat angB,SGDfloat lenC);
SGDfloat sgdTriangleSolver_SSStoArea(SGDfloat lenA,SGDfloat lenB,SGDfloat lenC);
SGDfloat sgdTriangleSolver_ASStoArea(SGDfloat angB,SGDfloat lenA,SGDfloat lenB);
SGDfloat sgdTriangleSolver_SAAtoArea(SGDfloat lenA,SGDfloat angB,SGDfloat angA);

void sgdTriangleSolver_SSStoAAA(SGDfloat  lenA,SGDfloat  lenB, SGDfloat  lenC, 
                                SGDfloat *angA,SGDfloat *angB,SGDfloat *angC ) ;
void sgdTriangleSolver_SAStoASA(SGDfloat  lenA,SGDfloat  angB,SGDfloat  lenC,
                                SGDfloat *angA,SGDfloat *lenB,SGDfloat *angC ) ;
void sgdTriangleSolver_ASAtoSAS(SGDfloat  angA,SGDfloat  lenB,SGDfloat  angC,
                                SGDfloat *lenA,SGDfloat *angB,SGDfloat *lenC ) ;
void sgdTriangleSolver_ASStoSAA(SGDfloat  angB,SGDfloat  lenA,SGDfloat  lenB,
                                SGDfloat *lenC,SGDfloat *angA,SGDfloat *angC ) ;
void sgdTriangleSolver_SAAtoASS(SGDfloat  lenA,SGDfloat  angB,SGDfloat  angA,
                                SGDfloat *angC,SGDfloat *lenB,SGDfloat *lenC ) ;

/*
  SPRING-MASS-DAMPER (with simple Euler integrator)
*/


extern sgVec3 _sgGravity ;

inline void   sgSetGravity     ( float  g ) { sgSetVec3 ( _sgGravity, 0.0f, 0.0f, -g ) ; }
inline void   sgSetGravityVec3 ( sgVec3 g ) { sgCopyVec3 ( _sgGravity, g ) ; }
inline float *sgGetGravityVec3 () { return   _sgGravity    ; }
inline float  sgGetGravity     () { return - _sgGravity[2] ; }


class sgParticle
{
  float ooMass ;  /* One-over-mass */
  sgVec3 pos   ;
  sgVec3 vel   ;
  sgVec3 force ;

public:

  sgParticle ( float mass, sgVec3 _pos )
  {
    setMass    ( mass ) ;
    sgCopyVec3 ( pos, _pos ) ;
    sgZeroVec3 (  vel  ) ;
    sgZeroVec3 ( force ) ;
  }

  sgParticle ( float mass, float x = 0.0f, float y = 0.0f, float z = 0.0f )
  {
    setMass    ( mass  ) ;
    sgSetVec3  ( pos, x, y, z ) ;
    sgZeroVec3 (  vel  ) ;
    sgZeroVec3 ( force ) ;
  }

  float *getPos         () { return pos      ; }
  float *getVel         () { return vel      ; }
  float *getForce       () { return force    ; }
  float  getOneOverMass () { return ooMass   ; }
  float  getMass        () { return 1.0f / ooMass ; }

  void   setPos   ( sgVec3 p ) { sgCopyVec3 ( pos  , p ) ; }
  void   setVel   ( sgVec3 v ) { sgCopyVec3 ( vel  , v ) ; }
  void   setForce ( sgVec3 f ) { sgCopyVec3 ( force, f ) ; }

  void   setPos   ( float x, float y, float z ) { sgSetVec3 ( pos  ,x,y,z ) ; }
  void   setVel   ( float x, float y, float z ) { sgSetVec3 ( vel  ,x,y,z ) ; }
  void   setForce ( float x, float y, float z ) { sgSetVec3 ( force,x,y,z ) ; }

  void   setOneOverMass ( float oom ) { ooMass = oom ; }

  void   setMass ( float m )
  {
    assert ( m > 0.0f ) ;
    ooMass = 1.0f / m ;
  }

  void zeroForce   ()           { sgZeroVec3   ( force ) ; }
  void addForce    ( sgVec3 f ) { sgAddVec3    ( force, f ) ; }
  void subForce    ( sgVec3 f ) { sgSubVec3    ( force, f ) ; }
  void gravityOnly ()           { sgScaleVec3  ( force, sgGetGravityVec3 (), ooMass ) ; }

  void bounce ( sgVec3 normal, float coefRestitution )
  {
    sgVec3 vn, vt ;
    sgScaleVec3 ( vn, normal,
                   sgScalarProductVec3 ( normal, vel ) ) ;
    sgSubVec3 ( vt, vel, vn ) ;
    sgAddScaledVec3 ( vel, vt, vn, -coefRestitution ) ;
  }

  void update ( float dt )
  {
    sgAddScaledVec3 ( vel, force, dt * ooMass ) ;
    sgAddScaledVec3 ( pos, vel, dt ) ;
  }
} ;


class sgSpringDamper
{
  sgParticle *p0 ;
  sgParticle *p1 ;

  float restLength ;
  float stiffness  ;
  float damping    ;

public:

  sgSpringDamper ()
  {
    p0 = p1 = NULL ;
    stiffness  = 1.0f ;
    damping    = 1.0f ;
    restLength = 1.0f ;
  }

  sgSpringDamper ( sgParticle *_p0, sgParticle *_p1,
                   float _stiffness, float _damping,
                   float _restLength = -1.0f )
  {
    p0 = _p0 ;
    p1 = _p1 ;
    stiffness = _stiffness ;
    damping   = _damping   ;

    if ( _restLength < 0.0f )
    {
      if ( p0 != NULL && p1 != NULL )
        restLength = sgDistanceVec3 ( p0->getPos(), p1->getPos() ) ;
      else
        restLength = _restLength ;
    }
    else
      restLength = 1.0f ;
  }

  float       getRestLength () { return restLength ; }
  float       getStiffness  () { return stiffness  ; }
  float       getDamping    () { return damping    ; }

  sgParticle *getParticle   ( int which ) { return ( which == 0 ) ? p0 : p1 ; }


  void setParticles ( sgParticle *_p0, sgParticle *_p1 ) { p0 = _p0 ; p1 = _p1 ; }
  void setParticle  ( int which, sgParticle *p ) { if ( which == 0 ) p0 = p ; else p1 = p ; }

  void setRestLength () { restLength = sgDistanceVec3 ( p0->getPos(), p1->getPos() ) ; }

  void setRestLength ( float l ) { restLength = l ; }
  void setStiffness  ( float s ) { stiffness  = s ; }
  void setDamping    ( float d ) { damping    = d ; }

  void update ()
  {
    sgVec3 dP ; sgSubVec3 ( dP, p0->getPos(), p1->getPos() ) ;
    sgVec3 dV ; sgSubVec3 ( dV, p0->getVel(), p1->getVel() ) ;

    float  L = sgLengthVec3 ( dP ) ; if ( L == 0.0f ) L = 0.0000001f ;
    float  H = ( L - restLength ) * stiffness ;
    float  D = sgScalarProductVec3 ( dV, dP ) * damping / L ;

    sgVec3 F ; sgScaleVec3 ( F, dP, - ( H + D ) / L ) ;

    p0 -> addForce ( F ) ;
    p1 -> subForce ( F ) ;
  }

} ;


/*
  It must be true that (x % NOISE_WRAP_INDEX) == (x & NOISE_MOD_MASK)
  so NOISE_WRAP_INDEX must be a power of two, and NOISE_MOD_MASK must be
  that power of 2 - 1.  as indices are implemented, as unsigned chars,
  NOISE_WRAP_INDEX shoud be less than or equal to 256.
  There's no good reason to change it from 256, really.

  NOISE_LARGE_PWR2 is a large power of 2, we'll go for 4096, to add to
  negative numbers in order to make them positive
*/

#define SG_PERLIN_NOISE_WRAP_INDEX    256
#define SG_PERLIN_NOISE_MOD_MASK      255
#define SG_PERLIN_NOISE_LARGE_PWR2   4096



class sgPerlinNoise_1D
{
private:

  SGfloat gradTable [ SG_PERLIN_NOISE_WRAP_INDEX * 2 + 2 ] ;

public:

  sgPerlinNoise_1D () ;

  void regenerate () ;

  SGfloat getNoise ( SGfloat x ) ;
} ;



class sgPerlinNoise_2D
{
private:

  sgVec2 gradTable [ SG_PERLIN_NOISE_WRAP_INDEX * 2 + 2 ] ;

public:

  sgPerlinNoise_2D () ;

  void regenerate () ;

  SGfloat getNoise ( sgVec2 pos ) ;
  SGfloat getNoise ( SGfloat x, SGfloat y )
  {
    sgVec2 p ;
    sgSetVec2 ( p, x, y ) ;
    return getNoise ( p ) ;
  }
} ;



class sgPerlinNoise_3D
{
private:

  sgVec3 gradTable [ SG_PERLIN_NOISE_WRAP_INDEX * 2 + 2 ] ;

public:

  sgPerlinNoise_3D () ;

  void regenerate () ;

  SGfloat getNoise ( sgVec3 pos ) ;
  SGfloat getNoise ( SGfloat x, SGfloat y, SGfloat z )
  {
    sgVec3 p ;
    sgSetVec3 ( p, x, y, z ) ;
    return getNoise ( p ) ;
  }
} ;



#endif

