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


#include "sg.h"

void sgdVectorProductVec3 ( sgdVec3 dst, const sgdVec3 a, const sgdVec3 b )
{
  dst[0] = a[1] * b[2] - a[2] * b[1] ;
  dst[1] = a[2] * b[0] - a[0] * b[2] ;
  dst[2] = a[0] * b[1] - a[1] * b[0] ;
}

inline SGDfloat _sgdClampToUnity ( const SGDfloat x )
{
  if ( x >  SGD_ONE ) return  SGD_ONE ;
  if ( x < -SGD_ONE ) return -SGD_ONE ;
  return x ;
}

int sgdCompare3DSqdDist( const sgdVec3 v1, const sgdVec3 v2, const SGDfloat sqd_dist )
{
  sgdVec3 tmp ;

  sgdSubVec3 ( tmp, v2, v1 ) ;

  SGDfloat sqdist = tmp[0] * tmp[0] + tmp[1] * tmp[1] + tmp[2] * tmp[2] ;

  if ( sqdist > sqd_dist ) return  1 ;
  if ( sqdist < sqd_dist ) return -1 ;
  return 0 ;
}

void sgdMakeRotMat4( sgdMat4 mat, const SGDfloat angle, const sgdVec3 axis )
{
  sgdVec3 ax ;
  sgdNormalizeVec3 ( ax, axis ) ; 

  SGDfloat temp_angle = angle * SGD_DEGREES_TO_RADIANS ;
  SGDfloat s = sin ( temp_angle ) ;
  SGDfloat c = cos ( temp_angle ) ;
  SGDfloat t = SGD_ONE - c ;
   
  mat[0][0] = t * ax[0] * ax[0] + c ;
  mat[0][1] = t * ax[0] * ax[1] + s * ax[2] ;
  mat[0][2] = t * ax[0] * ax[2] - s * ax[1] ;
  mat[0][3] = SGD_ZERO ;

  mat[1][0] = t * ax[1] * ax[0] - s * ax[2] ;
  mat[1][1] = t * ax[1] * ax[1] + c ;
  mat[1][2] = t * ax[1] * ax[2] + s * ax[0] ;
  mat[1][3] = SGD_ZERO ;

  mat[2][0] = t * ax[2] * ax[0] + s * ax[1] ;
  mat[2][1] = t * ax[2] * ax[1] - s * ax[0] ;
  mat[2][2] = t * ax[2] * ax[2] + c ;
  mat[2][3] = SGD_ZERO ;

  mat[3][0] = SGD_ZERO ;
  mat[3][1] = SGD_ZERO ;
  mat[3][2] = SGD_ZERO ;
  mat[3][3] = SGD_ONE ;
}


/*********************\
*    sgdBox routines   *
\*********************/


void sgdBox::extend ( const sgdVec3 v )
{
  if ( isEmpty () )
  {
    sgdCopyVec3 ( min, v ) ;
    sgdCopyVec3 ( max, v ) ;
  }
  else
  {
    if ( v[0] < min[0] ) min[0] = v[0] ;
    if ( v[1] < min[1] ) min[1] = v[1] ;
    if ( v[2] < min[2] ) min[2] = v[2] ;
    if ( v[0] > max[0] ) max[0] = v[0] ;
    if ( v[1] > max[1] ) max[1] = v[1] ;
    if ( v[2] > max[2] ) max[2] = v[2] ;
  }
}


void sgdBox::extend ( const sgdBox *b )
{
  if ( b -> isEmpty () )
    return ;

  if ( isEmpty () )
  {
    sgdCopyVec3 ( min, b->getMin() ) ;
    sgdCopyVec3 ( max, b->getMax() ) ;
  }
  else
  {
    extend ( b->getMin() ) ;
    extend ( b->getMax() ) ;
  }
}


void sgdBox::extend ( const sgdSphere *s )
{
  if ( s -> isEmpty () ) 
    return ;

  /*
    In essence, this extends around a box around the sphere - which
    is still a perfect solution because both boxes are axially aligned.
  */

  sgdVec3 x ;

  sgdSetVec3 ( x, s->getCenter()[0]+s->getRadius(),
                 s->getCenter()[1]+s->getRadius(),
                 s->getCenter()[2]+s->getRadius() ) ;
  extend ( x ) ;

  sgdSetVec3 ( x, s->getCenter()[0]-s->getRadius(),
                 s->getCenter()[1]-s->getRadius(),
                 s->getCenter()[2]-s->getRadius() ) ;
  extend ( x ) ;
}


int sgdBox::intersects ( const sgdVec4 plane ) const
{
  /*
    Save multiplies by not redoing Ax+By+Cz+D for each point.
  */

  SGDfloat Ax_min        = plane[0] * min[0] ;
  SGDfloat By_min        = plane[1] * min[1] ;
  SGDfloat Cz_min_plus_D = plane[2] * min[2] + plane[3] ;

  SGDfloat Ax_max        = plane[0] * max[0] ;
  SGDfloat By_max        = plane[1] * max[1] ;
  SGDfloat Cz_max_plus_D = plane[2] * max[2] + plane[3] ;

  /*
    Count the number of vertices on the positive side of the plane.
  */

  int count = ( Ax_min + By_min + Cz_min_plus_D > SGD_ZERO ) +
              ( Ax_min + By_min + Cz_max_plus_D > SGD_ZERO ) +
              ( Ax_min + By_max + Cz_min_plus_D > SGD_ZERO ) +
              ( Ax_min + By_max + Cz_max_plus_D > SGD_ZERO ) +
              ( Ax_max + By_min + Cz_min_plus_D > SGD_ZERO ) +
              ( Ax_max + By_min + Cz_max_plus_D > SGD_ZERO ) +
              ( Ax_max + By_max + Cz_min_plus_D > SGD_ZERO ) +
              ( Ax_max + By_max + Cz_max_plus_D > SGD_ZERO ) ;

  /*
    The plane intersects the box unless all 8 are positive
    or none of them are positive.
  */
              
  return count != 0 && count != 8 ;
}



/**********************\
*  sgdSphere routines   *
\**********************/

void sgdSphere::extend ( const sgdVec3 v )
{
  if ( isEmpty () )
  {
    sgdCopyVec3 ( center, v ) ;
    radius = SGD_ZERO ;
    return ;
  }

  SGDfloat d = sgdDistanceVec3 ( center, v ) ;

  if ( d <= radius )  /* Point is already inside sphere */
    return ;

  SGDfloat new_radius = (radius + d) / SGD_TWO ;  /* Grow radius */

  SGDfloat ratio = (new_radius - radius) / d ;

  center[0] += (v[0]-center[0]) * ratio ;    /* Move center */
  center[1] += (v[1]-center[1]) * ratio ;
  center[2] += (v[2]-center[2]) * ratio ;

  radius = new_radius ;
}


void sgdSphere::extend ( const sgdBox *b )
{
  if ( b -> isEmpty () )
    return ;

  if ( isEmpty() )
  {
    sgdAddVec3   ( center, b->getMin(), b->getMax() ) ;
    sgdScaleVec3 ( center, SGD_HALF ) ;
    radius = sgdDistanceVec3 ( center, b->getMax() ) ;
    return ;
  }

  /*
    I can't think of a faster way to get an
    utterly minimal sphere.

    The tighter algorithm:- enclose each
    of eight vertices of the box in turn - it
    looks like being pretty costly.
    [8 sqrt()'s]

    The looser algorithm:- enclose the box
    with an empty sphere and then do a
    sphere-extend-sphere. This algorithm
    does well for close-to-cube boxes, but
    makes very poor spheres for long, thin
    boxes.
    [2 sqrt()'s]
  */

#ifdef DONT_REALLY_NEED_A_TIGHT_SPHERE_EXTEND_BOX

  /* LOOSER/FASTER sphere-around-sphere-around-box */
  sgdSphere s ;
  s.empty   ()    ;
  s.enclose ( b ) ;  /* Fast because s is empty */
    enclose ( s ) ;

#else

  /* TIGHTER/EXPENSIVE sphere-around-eight-points */
  sgdVec3 x ;
                                                        extend ( b->getMin() ) ;
  sgdSetVec3 ( x, b->getMin()[0],b->getMin()[1],b->getMax()[2] ) ; extend ( x ) ;
  sgdSetVec3 ( x, b->getMin()[0],b->getMax()[1],b->getMin()[2] ) ; extend ( x ) ;
  sgdSetVec3 ( x, b->getMin()[0],b->getMax()[1],b->getMax()[2] ) ; extend ( x ) ;
  sgdSetVec3 ( x, b->getMax()[0],b->getMin()[1],b->getMin()[2] ) ; extend ( x ) ;
  sgdSetVec3 ( x, b->getMax()[0],b->getMin()[1],b->getMax()[2] ) ; extend ( x ) ;
  sgdSetVec3 ( x, b->getMax()[0],b->getMax()[1],b->getMin()[2] ) ; extend ( x ) ;
                                                        extend ( b->getMax() ) ;
#endif
}


void sgdSphere::extend ( const sgdSphere *s )
{
  if ( s->isEmpty () )
    return ;

  if ( isEmpty () )
  {
    sgdCopyVec3 ( center, s->getCenter() ) ;
    radius = s->getRadius() ;
    return ;
  }

  /* 
    d == The distance between the sphere centers
  */

  SGDfloat d = sgdDistanceVec3 ( center, s->getCenter() ) ;

  if ( d + s->getRadius() <= radius )  /* New sphere is already inside this one */
    return ;

  if ( d + radius <= s->getRadius() )  /* New sphere completely contains this one */
  {
    sgdCopyVec3 ( center, s->getCenter() ) ;
    radius = s->getRadius() ;
    return ;
  } 

  /*
    Build a new sphere that completely contains the other two:

    The center point lies halfway along the line between
    the furthest points on the edges of the two spheres.
    Computing those two points is ugly - so we'll use similar
    triangles
  */

  SGDfloat new_radius = (radius + d + s->getRadius() ) / SGD_TWO ;

  SGDfloat ratio = ( new_radius - radius ) / d ;

  center[0] += ( s->getCenter()[0] - center[0] ) * ratio ;
  center[1] += ( s->getCenter()[1] - center[1] ) * ratio ;
  center[2] += ( s->getCenter()[2] - center[2] ) * ratio ;
  radius = new_radius ;
}


int sgdSphere::intersects ( const sgdBox *b ) const
{
  sgdVec3 closest ;

  if ( b->getMin()[0] > center[0] ) closest[0] = b->getMin()[0] ; else
  if ( b->getMax()[0] < center[0] ) closest[0] = b->getMax()[0] ; else
                                    closest[0] = center[0] ;

  if ( b->getMin()[1] > center[1] ) closest[1] = b->getMin()[1] ; else
  if ( b->getMax()[1] < center[1] ) closest[1] = b->getMax()[1] ; else
                                    closest[1] = center[1] ;

  if ( b->getMin()[2] > center[2] ) closest[2] = b->getMin()[2] ; else
  if ( b->getMax()[2] < center[2] ) closest[2] = b->getMax()[2] ; else
                                    closest[2] = center[2] ;

  return sgdCompare3DSqdDist ( closest, center, sgdSquare ( radius ) ) <= 0 ;
}


/************************\
*   sgdFrustum routines   *
\************************/

void sgdFrustum::update ()
{
  if ( fabs ( ffar - nnear ) < 0.1 )
  {
    ulSetError ( UL_WARNING, "sgdFrustum: Can't support depth of view <0.1 units.");
    return ;
  }

  if ( hfov != SGD_ZERO && vfov != SGD_ZERO )
  {
    if ( fabs ( hfov ) < 0.1 || fabs ( vfov ) < 0.1 )
    {
      ulSetError ( UL_WARNING, "sgdFrustum: Can't support fields of view narrower than 0.1 degrees.");
      return ;
    }

    /* Corners of screen relative to eye... */
  
    right = nnear * tan ( hfov * SGD_DEGREES_TO_RADIANS / SGD_TWO ) ;
    top   = nnear * tan ( vfov * SGD_DEGREES_TO_RADIANS / SGD_TWO ) ;
    left  = -right ;
    bot   = -top   ;
  }

  /*
    Compute plane equations for the four sloping faces of the frustum.

    These are useful for FrustContains(sphere) tests.

    Noting that those planes always go through the origin, their 'D'
    components will always be zero - so the plane equation is really
    just the normal - which is the cross-product of two edges of each face,
    and since we can pick two edges that go through the origin, the
    vectors for the edges are just the normalised corners of the near plane.
  */

  sgdVec3 v1, v2, v3, v4 ;

  sgdSetVec3 ( v1, left , top, -nnear ) ;
  sgdSetVec3 ( v2, right, top, -nnear ) ;
  sgdSetVec3 ( v3, left , bot, -nnear ) ;
  sgdSetVec3 ( v4, right, bot, -nnear ) ;

  sgdNormaliseVec3 ( v1 ) ;
  sgdNormaliseVec3 ( v2 ) ;
  sgdNormaliseVec3 ( v3 ) ;
  sgdNormaliseVec3 ( v4 ) ;

  /*
    Take care of the order of the parameters so that all the planes
    are oriented facing inwards...
  */

  sgdVectorProductVec3 (   top_plane, v1, v2 ) ;
  sgdVectorProductVec3 ( right_plane, v2, v4 ) ;
  sgdVectorProductVec3 (   bot_plane, v4, v3 ) ;
  sgdVectorProductVec3 (  left_plane, v3, v1 ) ;

  /* 
    At this point, you could call

      glMatrixMode ( GL_PROJECTION ) ;
      glLoadIdentity () ;
      glFrustum      ( left, right, bot, top, nnear, ffar ) ;

    Or...

      pfMakePerspFrust ( frust, left, right, bot, top ) ;
      pfFrustNearFar   ( frust, nnear, ffar ) ;

    Or...

      just use the matrix we generate below:
  */

  /* Width, height, depth */

  SGDfloat w = right - left ;
  SGDfloat h = top   - bot  ;
  SGDfloat d = ffar  - nnear ;

  mat[0][0] =  SGD_TWO * nnear / w ;
  mat[0][1] =  SGD_ZERO ;
  mat[0][2] =  SGD_ZERO ;
  mat[0][3] =  SGD_ZERO ;

  mat[1][0] =  SGD_ZERO ;
  mat[1][1] =  SGD_TWO * nnear / h ;
  mat[1][2] =  SGD_ZERO ;
  mat[1][3] =  SGD_ZERO ;

  mat[2][0] =  ( right + left ) / w ;
  mat[2][1] =  ( top   + bot  ) / h ;
  mat[2][2] = -( ffar  + nnear ) / d ;
  mat[2][3] = -SGD_ONE ;

  mat[3][0] =  SGD_ZERO ;
  mat[3][1] =  SGD_ZERO ;
  mat[3][2] = -SGD_TWO * nnear * ffar/ d ;
  mat[3][3] =  SGD_ZERO ;
}


#define OC_LEFT_SHIFT   0
#define OC_RIGHT_SHIFT  1
#define OC_TOP_SHIFT    2
#define OC_BOT_SHIFT    3
#define OC_NEAR_SHIFT   4
#define OC_FAR_SHIFT    5

#define OC_ALL_ON_SCREEN 0x3F
#define OC_OFF_TRF      ((1<<OC_TOP_SHIFT)|(1<<OC_RIGHT_SHIFT)|(1<<OC_FAR_SHIFT))
#define OC_OFF_BLN      ((1<<OC_BOT_SHIFT)|(1<<OC_LEFT_SHIFT)|(1<<OC_NEAR_SHIFT))

int sgdFrustum::getOutcode ( const sgdVec3 pt ) const
{
  /* Transform the point by the Frustum's transform. */

  sgdVec4 tmp ;

  tmp [ 0 ] = pt [ 0 ] ;
  tmp [ 1 ] = pt [ 1 ] ;
  tmp [ 2 ] = pt [ 2 ] ;
  tmp [ 3 ] =  SGD_ONE  ;

  sgdXformPnt4 ( tmp, tmp, mat ) ;

  /*
    No need to divide by the 'w' component since we are only checking for
    results in the range 0..1
  */

  return (( tmp[0] <=  tmp[3] ) << OC_RIGHT_SHIFT ) |
         (( tmp[0] >= -tmp[3] ) << OC_LEFT_SHIFT  ) |
         (( tmp[1] <=  tmp[3] ) << OC_TOP_SHIFT   ) |
         (( tmp[1] >= -tmp[3] ) << OC_BOT_SHIFT   ) |
         (( tmp[2] <=  tmp[3] ) << OC_FAR_SHIFT   ) |
         (( tmp[2] >= -tmp[3] ) << OC_NEAR_SHIFT  ) ;
}

int sgdFrustum::contains ( const sgdVec3 pt ) const
{
  return getOutcode ( pt ) == OC_ALL_ON_SCREEN ;
}


int sgdFrustum::contains ( const sgdSphere *s ) const
{
  /*
    Lop off half the database (roughly) with a quick near-plane test - and
    lop off a lot more with a quick far-plane test
  */

  if ( -s->getCenter() [ 2 ] + s->getRadius() < nnear ||
       -s->getCenter() [ 2 ] - s->getRadius() > ffar )
    return SGD_OUTSIDE ;

  /*
    OK, so the sphere lies between near and far.

    Measure the distance of the center point from the four sides of the frustum,
    if it's outside by more than the radius then it's history.

    It's tempting to do a quick test to see if the center point is
    onscreen using sgdFrustumContainsPt - but that takes a matrix transform
    which is 16 multiplies and 12 adds - versus this test which does the
    whole task using only 12 multiplies and 8 adds.
  */

  SGDfloat sp1 = sgdScalarProductVec3 (  left_plane, s->getCenter() ) ;
  SGDfloat sp2 = sgdScalarProductVec3 ( right_plane, s->getCenter() ) ;
  SGDfloat sp3 = sgdScalarProductVec3 (   bot_plane, s->getCenter() ) ;
  SGDfloat sp4 = sgdScalarProductVec3 (   top_plane, s->getCenter() ) ;

  if ( -sp1 >= s->getRadius() || -sp2 >= s->getRadius() ||
       -sp3 >= s->getRadius() || -sp4 >= s->getRadius() )
    return SGD_OUTSIDE ;
  
  /*
    If it's inside by more than the radius then it's *completely* inside
    and we can save time elsewhere if we know that for sure.
  */

  if ( -s->getCenter() [ 2 ] - s->getRadius() > nnear &&
       -s->getCenter() [ 2 ] + s->getRadius() < ffar &&
       sp1 >= s->getRadius() && sp2 >= s->getRadius() &&
       sp3 >= s->getRadius() && sp4 >= s->getRadius() )
    return SGD_INSIDE ;

  return SGD_STRADDLE ;
}


void sgdMakeCoordMat4 ( sgdMat4 m, const SGDfloat x, const SGDfloat y, const SGDfloat z, const SGDfloat h, const SGDfloat p, const SGDfloat r )
{
  SGDfloat ch, sh, cp, sp, cr, sr, srsp, crsp, srcp ;

  if ( h == SGD_ZERO )
  {
    ch = SGD_ONE ;
    sh = SGD_ZERO ;
  }
  else
  {
    sh = sin( h * SGD_DEGREES_TO_RADIANS) ;
    ch = cos( h * SGD_DEGREES_TO_RADIANS) ;
  }

  if ( p == SGD_ZERO )
  {
    cp = SGD_ONE ;
    sp = SGD_ZERO ;
  }
  else
  {
    sp = sin( p * SGD_DEGREES_TO_RADIANS) ;
    cp = cos( p * SGD_DEGREES_TO_RADIANS) ;
  }

  if ( r == SGD_ZERO )
  {
    cr   = SGD_ONE ;
    sr   = SGD_ZERO ;
    srsp = SGD_ZERO ;
    srcp = SGD_ZERO ;
    crsp = sp ;
  }
  else
  {
    sr   = sin( r * SGD_DEGREES_TO_RADIANS) ;
    cr   = cos( r * SGD_DEGREES_TO_RADIANS) ;
    srsp = sr * sp ;
    crsp = cr * sp ;
    srcp = sr * cp ;
  }

  m[0][0] =  ch * cr - sh * srsp ;
  m[1][0] = -sh * cp ;
  m[2][0] =  sr * ch + sh * crsp ;
  m[3][0] =  x ;

  m[0][1] =  cr * sh + srsp * ch ;
  m[1][1] =  ch * cp ;
  m[2][1] =  sr * sh - crsp * ch ;
  m[3][1] =  y ;

  m[0][2] = -srcp ;
  m[1][2] =  sp ;
  m[2][2] =  cr * cp ;
  m[3][2] =  z ;

  m[0][3] =  SGD_ZERO ;
  m[1][3] =  SGD_ZERO ;
  m[2][3] =  SGD_ZERO ;
  m[3][3] =  SGD_ONE ;
}


void sgdMakeTransMat4 ( sgdMat4 m, const sgdVec3 xyz )
{
  m[0][1] = m[0][2] = m[0][3] =
  m[1][0] = m[1][2] = m[1][3] =
  m[2][0] = m[2][1] = m[2][3] = SGD_ZERO ;
  m[0][0] = m[1][1] = m[2][2] = m[3][3] = SGD_ONE ;
  sgdCopyVec3 ( m[3], xyz ) ;
}


void sgdMakeTransMat4 ( sgdMat4 m, const SGDfloat x, const SGDfloat y, const SGDfloat z )
{
  m[0][1] = m[0][2] = m[0][3] =
  m[1][0] = m[1][2] = m[1][3] =
  m[2][0] = m[2][1] = m[2][3] = SGD_ZERO ;
  m[0][0] = m[1][1] = m[2][2] = m[3][3] = SGD_ONE ;
  sgdSetVec3 ( m[3], x, y, z ) ;
}


void sgdSetCoord ( sgdCoord *dst, const sgdMat4 src )
{
  sgdCopyVec3 ( dst->xyz, src[3] ) ;
    
  sgdMat4 mat ;

  SGDfloat s = sgdLengthVec3 ( src[0] ) ;

  if ( s <= 0.00001 )
  {
    ulSetError ( UL_WARNING, "sgdMat4ToCoord: ERROR - Bad Matrix." ) ;
    sgdSetVec3 ( dst -> hpr, SGD_ZERO, SGD_ZERO, SGD_ZERO ) ;
    return ;
  }

  sgdScaleMat4 ( mat, src, SGD_ONE / s ) ;
    
  dst->hpr[1] = asin ( _sgdClampToUnity ( mat[1][2] ) ) ;

  SGDfloat cp = cos ( dst->hpr[1] ) ;
    
  /* If pointing nearly vertically up - then heading is ill-defined */

  if ( cp > -0.00001 && cp < 0.00001 )
  {
    SGDfloat cr = _sgdClampToUnity ( mat[0][1] ) ; 
    SGDfloat sr = _sgdClampToUnity (-mat[2][1] ) ;

    dst->hpr[0] = SGD_ZERO ;
    dst->hpr[2] = atan2 ( sr, cr ) ;
  }
  else
  {
    SGDfloat sr = _sgdClampToUnity ( -mat[0][2] / cp ) ;
    SGDfloat cr = _sgdClampToUnity (  mat[2][2] / cp ) ;
    SGDfloat sh = _sgdClampToUnity ( -mat[1][0] / cp ) ;
    SGDfloat ch = _sgdClampToUnity (  mat[1][1] / cp ) ;
  
    if ( (sh == SGD_ZERO && ch == SGD_ZERO) || (sr == SGD_ZERO && cr == SGD_ZERO) )
    {
      cr = _sgdClampToUnity ( mat[0][1] ) ;
      sr = _sgdClampToUnity (-mat[2][1] ) ;

      dst->hpr[0] = SGD_ZERO ;
    }
    else
      dst->hpr[0] = atan2 ( sh, ch ) ;

    dst->hpr[2] = atan2 ( sr, cr ) ;
  }

  sgdScaleVec3 ( dst->hpr, SGD_RADIANS_TO_DEGREES ) ;
}


void sgdMakeNormal(sgdVec3 dst, const sgdVec3 a, const sgdVec3 b, const sgdVec3 c )
{
  sgdVec3 ab ; sgdSubVec3 ( ab, b, a ) ; sgdNormaliseVec3 ( ab ) ;
  sgdVec3 ac ; sgdSubVec3 ( ac, c, a ) ; sgdNormaliseVec3 ( ac ) ;
  sgdVectorProductVec3 ( dst, ab,ac ) ; sgdNormaliseVec3 ( dst ) ; /* XXX DO WE REALLY NEED THIS? */
}


void sgdPreMultMat4( sgdMat4 dst, const sgdMat4 src )
{
  sgdMat4 mat ;
  sgdMultMat4 ( mat, dst, src ) ;
  sgdCopyMat4 ( dst, mat ) ;
}

void sgdPostMultMat4( sgdMat4 dst, const sgdMat4 src )
{
  sgdMat4 mat ;
  sgdMultMat4 ( mat, src, dst ) ;
  sgdCopyMat4 ( dst, mat ) ;
}

void sgdMultMat4( sgdMat4 dst, const sgdMat4 m1, const sgdMat4 m2 )
{
  for ( int j = 0 ; j < 4 ; j++ )
  {
    dst[0][j] = m2[0][0] * m1[0][j] +
		m2[0][1] * m1[1][j] +
		m2[0][2] * m1[2][j] +
		m2[0][3] * m1[3][j] ;

    dst[1][j] = m2[1][0] * m1[0][j] +
		m2[1][1] * m1[1][j] +
		m2[1][2] * m1[2][j] +
		m2[1][3] * m1[3][j] ;

    dst[2][j] = m2[2][0] * m1[0][j] +
		m2[2][1] * m1[1][j] +
		m2[2][2] * m1[2][j] +
		m2[2][3] * m1[3][j] ;

    dst[3][j] = m2[3][0] * m1[0][j] +
		m2[3][1] * m1[1][j] +
		m2[3][2] * m1[2][j] +
		m2[3][3] * m1[3][j] ;
  }
}



void sgdTransposeNegateMat4 ( sgdMat4 dst, const sgdMat4 src )
{
  /* Poor man's invert - can be used when matrix is a simple rotate-translate */

  dst[0][0] = src[0][0] ;
  dst[1][0] = src[0][1] ;
  dst[2][0] = src[0][2] ;
  dst[3][0] = - sgdScalarProductVec3 ( src[3], src[0] ) ;

  dst[0][1] = src[1][0] ;
  dst[1][1] = src[1][1] ;
  dst[2][1] = src[1][2] ;
  dst[3][1] = - sgdScalarProductVec3 ( src[3], src[1] ) ;
                                                                               
  dst[0][2] = src[2][0] ;                                                      
  dst[1][2] = src[2][1] ;                                                      
  dst[2][2] = src[2][2] ;                                                      
  dst[3][2] = - sgdScalarProductVec3 ( src[3], src[2] ) ;
                                                                               
  dst[0][3] = SGD_ZERO ;
  dst[1][3] = SGD_ZERO ;                                                        
  dst[2][3] = SGD_ZERO ;                                                        
  dst[3][3] = SGD_ONE  ;
}

void sgdTransposeNegateMat4 ( sgdMat4 dst )
{
  sgdMat4 src ;
  sgdCopyMat4 ( src, dst ) ;
  sgdTransposeNegateMat4 ( dst, src ) ;
}


void sgdInvertMat4 ( sgdMat4 dst, const sgdMat4 src )
{
  sgdMat4 tmp ;

  sgdCopyMat4 ( tmp, src ) ;
  sgdMakeIdentMat4 ( dst ) ;

  for ( int i = 0 ; i != 4 ; i++ )
  {
    SGDfloat val = tmp[i][i] ;
    int ind = i ;
    int j ;

    for ( j = i + 1 ; j != 4 ; j++ )
    {
      if ( fabs ( tmp[i][j] ) > fabs(val) )
      {
        ind = j;
        val = tmp[i][j] ;
      }
    }

    if ( ind != i )
    {                   /* swap columns */
      for ( j = 0 ; j != 4 ; j++ )
      {
        SGDfloat t ;
        t = dst[j][i]; dst[j][i] = dst[j][ind]; dst[j][ind] = t ;
        t = tmp[j][i]; tmp[j][i] = tmp[j][ind]; tmp[j][ind] = t ;
      }
    }

    // if ( val == SG_ZERO)
    if ( fabs(val) <= DBL_EPSILON )
    {
      ulSetError ( UL_WARNING, "sg: ERROR - Singular matrix, no inverse!" ) ;
      sgdMakeIdentMat4 ( dst ) ;  /* Do *something* */
      return;
    }

    SGDfloat ival = SGD_ONE / val ;

    for ( j = 0 ; j != 4 ; j++ )
    {
      tmp[j][i] *= ival ;
      dst[j][i] *= ival ;
    }

    for (j = 0; j != 4; j++)
    {
      if ( j == i )
        continue ;

      val = tmp[i][j] ;

      for ( int k = 0 ; k != 4 ; k++ )
      {
        tmp[k][j] -= tmp[k][i] * val ;
        dst[k][j] -= dst[k][i] * val ;
      }
    }
  }
}



void sgdXformVec3 ( sgdVec3 dst, const sgdVec3 src, const sgdMat4 mat )
{
  SGDfloat t0 = src[ 0 ] ;
  SGDfloat t1 = src[ 1 ] ;
  SGDfloat t2 = src[ 2 ] ;

  dst[0] = ( t0 * mat[ 0 ][ 0 ] +
             t1 * mat[ 1 ][ 0 ] +
             t2 * mat[ 2 ][ 0 ] ) ;

  dst[1] = ( t0 * mat[ 0 ][ 1 ] +
             t1 * mat[ 1 ][ 1 ] +
             t2 * mat[ 2 ][ 1 ] ) ;

  dst[2] = ( t0 * mat[ 0 ][ 2 ] +
             t1 * mat[ 1 ][ 2 ] +
             t2 * mat[ 2 ][ 2 ] ) ;
}


void sgdXformPnt3 ( sgdVec3 dst, const sgdVec3 src, const sgdMat4 mat )
{
  SGDfloat t0 = src[ 0 ] ;
  SGDfloat t1 = src[ 1 ] ;
  SGDfloat t2 = src[ 2 ] ;

  dst[0] = ( t0 * mat[ 0 ][ 0 ] +
             t1 * mat[ 1 ][ 0 ] +
             t2 * mat[ 2 ][ 0 ] +
                  mat[ 3 ][ 0 ] ) ;

  dst[1] = ( t0 * mat[ 0 ][ 1 ] +
             t1 * mat[ 1 ][ 1 ] +
             t2 * mat[ 2 ][ 1 ] +
                  mat[ 3 ][ 1 ] ) ;

  dst[2] = ( t0 * mat[ 0 ][ 2 ] +
             t1 * mat[ 1 ][ 2 ] +
             t2 * mat[ 2 ][ 2 ] +
                  mat[ 3 ][ 2 ] ) ;
}


void sgdXformPnt4 ( sgdVec4 dst, const sgdVec4 src, const sgdMat4 mat )
{
  SGDfloat t0 = src[ 0 ] ;
  SGDfloat t1 = src[ 1 ] ;
  SGDfloat t2 = src[ 2 ] ;
  SGDfloat t3 = src[ 3 ] ;

  dst[0] = ( t0 * mat[ 0 ][ 0 ] +
             t1 * mat[ 1 ][ 0 ] +
             t2 * mat[ 2 ][ 0 ] +
             t3 * mat[ 3 ][ 0 ] ) ;

  dst[1] = ( t0 * mat[ 0 ][ 1 ] +
             t1 * mat[ 1 ][ 1 ] +
             t2 * mat[ 2 ][ 1 ] +
             t3 * mat[ 3 ][ 1 ] ) ;

  dst[2] = ( t0 * mat[ 0 ][ 2 ] +
             t1 * mat[ 1 ][ 2 ] +
             t2 * mat[ 2 ][ 2 ] +
             t3 * mat[ 3 ][ 2 ] ) ;

  dst[3] = ( t0 * mat[ 0 ][ 3 ] +
             t1 * mat[ 1 ][ 3 ] +
             t2 * mat[ 2 ][ 3 ] +
             t3 * mat[ 3 ][ 3 ] ) ;
}


void sgdFullXformPnt3 ( sgdVec3 dst, const sgdVec3 src, const sgdMat4 mat )
{
  sgdVec4 tmp ;

  tmp [ 0 ] = src [ 0 ] ;
  tmp [ 1 ] = src [ 1 ] ;
  tmp [ 2 ] = src [ 2 ] ;
  tmp [ 3 ] =   SGD_ONE  ;

  sgdXformPnt4 ( tmp, tmp, mat ) ;
  sgdScaleVec3 ( dst, tmp, SGD_ONE / tmp [ 3 ] ) ;
}

void sgdHPRfromVec3 ( sgdVec3 hpr, sgdVec3 src )
{
  sgdVec3 tmp ;
  sgdCopyVec3 ( tmp, src ) ;
  sgdNormaliseVec3 ( tmp ) ;
  hpr[0] = -atan2 ( tmp [ 0 ], tmp [ 1 ] ) * SGD_RADIANS_TO_DEGREES ;
  hpr[1] = -atan2 ( tmp [ 2 ], sqrt ( sgdSquare ( tmp [ 0 ] ) +
                                      sgdSquare ( tmp [ 1 ] ) ) ) *
                                             SGD_RADIANS_TO_DEGREES ;
  hpr[2] = SGD_ZERO ;
}



/*
  Quaternion routines are Copyright (C) 1999
  Kevin B. Thompson <kevinbthompson@yahoo.com>
  Modified by Sylvan W. Clebsch <sylvan@stanford.edu>
  Largely rewritten by "Negative0" <negative0@earthlink.net>
*/


void sgdQuatToAngleAxis ( SGDfloat *angle,
                         SGDfloat *x, SGDfloat *y, SGDfloat *z,
                         const sgdQuat src )
{
  sgdVec3 axis ;

  sgdQuatToAngleAxis ( angle, axis, src ) ;

  *x = axis [ 0 ] ;
  *y = axis [ 1 ] ;
  *z = axis [ 2 ] ;
}


void sgdQuatToAngleAxis ( SGDfloat *angle, sgdVec3 axis, const sgdQuat src )
{
  SGDfloat a = (SGDfloat) acos ( src[SGD_W] ) ;
  SGDfloat s = (SGDfloat) sin  ( a ) ;

  *angle = a * SGD_RADIANS_TO_DEGREES * SGD_TWO ;

  if ( s == SGD_ZERO )
    sgdSetVec3 ( axis, SGD_ZERO, SGD_ZERO, SGD_ONE );
  else
  {
    sgdSetVec3   ( axis, src[SGD_X], src[SGD_Y], src[SGD_Z] ) ;
    sgdScaleVec3 ( axis, SGD_ONE / s ) ;
  }
}


void sgdAngleAxisToQuat ( sgdQuat dst,
                         const SGDfloat angle,
                         const SGDfloat x, const SGDfloat y, const SGDfloat z )
{
  sgdVec3 axis ; 
  sgdSetVec3 ( axis, x, y, z ) ;
  sgdAngleAxisToQuat ( dst, angle, axis ) ;
}


void sgdAngleAxisToQuat ( sgdQuat dst, const SGDfloat angle, const sgdVec3 axis )
{
  SGDfloat temp_angle = angle * SGD_DEGREES_TO_RADIANS / SGD_TWO ;

  sgdVec3 ax ;
  sgdNormaliseVec3 ( ax, axis ) ;

  SGDfloat s = - (SGDfloat) sin ( temp_angle ) ;

  dst[SGD_W] = (SGDfloat) cos ( temp_angle ) ;
  sgdScaleVec3 ( dst, ax, s ) ;
}


//from gamasutra.com
//by nb

void sgdMatrixToQuat( sgdQuat quat, sgdMat4 m )
{
  SGDfloat tr, s, q[4] ;
  int   i, j, k ;

  int nxt[3] = {1, 2, 0};

  tr = m[0][0] + m[1][1] + m[2][2];

  // check the diagonal
  if (tr > SGD_ZERO )
  {
    s = (SGDfloat) sqrt (tr + SGD_ONE);
    quat[SGD_W] = s / SGD_TWO;
    s = SGD_HALF / s;
    quat[SGD_X] = (m[1][2] - m[2][1]) * s;
    quat[SGD_Y] = (m[2][0] - m[0][2]) * s;
    quat[SGD_Z] = (m[0][1] - m[1][0]) * s;
  }
  else
  {		
    // diagonal is negative
   	i = 0;
    if (m[1][1] > m[0][0]) i = 1;
    if (m[2][2] > m[i][i]) i = 2;
    j = nxt[i];
    k = nxt[j];
    s = sqrt ((m[i][i] - (m[j][j] + m[k][k])) + SGD_ONE);
    q[i] = s * SGD_HALF;
            
    if (s != SGD_ZERO) s = SGD_HALF / s;

    q[3] = (m[j][k] - m[k][j]) * s;
    q[j] = (m[i][j] + m[j][i]) * s;
    q[k] = (m[i][k] + m[k][i]) * s;

    quat[SGD_X] = q[0];
    quat[SGD_Y] = q[1];
    quat[SGD_Z] = q[2];
    quat[SGD_W] = q[3];
  }
}


void sgdMultQuat ( sgdQuat dst, const sgdQuat a, const sgdQuat b )
{
  /* [ ww' - v.v', vxv' + wv' + v'w ] */

  SGDfloat t[8];

  t[0] = (a[SGD_W] + a[SGD_X]) * (b[SGD_W] + b[SGD_X]);
  t[1] = (a[SGD_Z] - a[SGD_Y]) * (b[SGD_Y] - b[SGD_Z]);
  t[2] = (a[SGD_X] - a[SGD_W]) * (b[SGD_Y] + b[SGD_Z]);
  t[3] = (a[SGD_Y] + a[SGD_Z]) * (b[SGD_X] - b[SGD_W]);
  t[4] = (a[SGD_X] + a[SGD_Z]) * (b[SGD_X] + b[SGD_Y]);
  t[5] = (a[SGD_X] - a[SGD_Z]) * (b[SGD_X] - b[SGD_Y]);
  t[6] = (a[SGD_W] + a[SGD_Y]) * (b[SGD_W] - b[SGD_Z]);
  t[7] = (a[SGD_W] - a[SGD_Y]) * (b[SGD_W] + b[SGD_Z]);

  dst[SGD_W] =  t[1] + ((-t[4] - t[5] + t[6] + t[7]) * SGD_HALF);
  dst[SGD_X] =  t[0] - (( t[4] + t[5] + t[6] + t[7]) * SGD_HALF);
  dst[SGD_Y] = -t[2] + (( t[4] - t[5] + t[6] - t[7]) * SGD_HALF);
  dst[SGD_Z] = -t[3] + (( t[4] - t[5] - t[6] + t[7]) * SGD_HALF);
}

//from gamasutra.com
//by nb@netcom.ca 

void sgdMultQuat2 ( sgdQuat dst, const sgdQuat a, const sgdQuat b )
{
  SGDfloat A, B, C, D, E, F, G, H;

  A = (a[SGD_W] + a[SGD_X]) * (b[SGD_W] + b[SGD_X]) ;
  B = (a[SGD_Z] - a[SGD_Y]) * (b[SGD_Y] - b[SGD_Z]) ;
  C = (a[SGD_X] - a[SGD_W]) * (b[SGD_Y] + b[SGD_Z]) ;
  D = (a[SGD_Y] + a[SGD_Z]) * (b[SGD_X] - b[SGD_W]) ;
  E = (a[SGD_X] + a[SGD_Z]) * (b[SGD_X] + b[SGD_Y]) ;
  F = (a[SGD_X] - a[SGD_Z]) * (b[SGD_X] - b[SGD_Y]) ;
  G = (a[SGD_W] + a[SGD_Y]) * (b[SGD_W] - b[SGD_Z]) ;
  H = (a[SGD_W] - a[SGD_Y]) * (b[SGD_W] + b[SGD_Z]) ;


  dst[SGD_W] =  B + (-E - F + G + H) / SGD_TWO ;
  dst[SGD_X] =  A - ( E + F + G + H) / SGD_TWO ; 
  dst[SGD_Y] = -C + ( E - F + G - H) / SGD_TWO ;
  dst[SGD_Z] = -D + ( E - F - G + H) / SGD_TWO ;
}

//from gamasutra.com
//by nb@netcom.ca 

void sgdEulerToQuat(sgdQuat quat, sgdVec3 hpr )
{
  SGDfloat cr, cp, cy, sr, sp, sy, cpcy, spsy;

// calculate trig identities
  cr = (SGDfloat) cos(hpr[2]*SGD_DEGREES_TO_RADIANS/SGD_TWO);
  cp = (SGDfloat) cos(hpr[1]*SGD_DEGREES_TO_RADIANS/SGD_TWO);
  cy = (SGDfloat) cos(hpr[0]*SGD_DEGREES_TO_RADIANS/SGD_TWO);

  sr = (SGDfloat) sin(hpr[2]*SGD_DEGREES_TO_RADIANS/SGD_TWO);
  sp = (SGDfloat) sin(hpr[1]*SGD_DEGREES_TO_RADIANS/SGD_TWO);
  sy = (SGDfloat) sin(hpr[0]*SGD_DEGREES_TO_RADIANS/SGD_TWO);
  
  cpcy = cp * cy;
  spsy = sp * sy;

  quat[SGD_W] = cr * cpcy + sr * spsy;
  quat[SGD_X] = sr * cpcy - cr * spsy;
  quat[SGD_Y] = cr * sp * cy + sr * cp * sy;
  quat[SGD_Z] = cr * cp * sy - sr * sp * cy;
}

//from darwin3d.com
// jeffl@darwin3d.com

void sgdQuatToEuler( sgdVec3 hpr, const sgdQuat quat )
{
  SGDfloat matrix[3][3];
  SGDfloat cx,sx;
  SGDfloat cy,sy,yr;
  SGDfloat cz,sz;

  // CONVERT QUATERNION TO MATRIX - I DON'T REALLY NEED ALL OF IT

  matrix[0][0] = SGD_ONE - (SGD_TWO * quat[SGD_Y] * quat[SGD_Y])
                         - (SGD_TWO * quat[SGD_Z] * quat[SGD_Z]);
//matrix[0][1] = (SGD_TWO * quat->x * quat->y) - (SGD_TWO * quat->w * quat->z);
//matrix[0][2] = (SGD_TWO * quat->x * quat->z) + (SGD_TWO * quat->w * quat->y);

  matrix[1][0] = (SGD_TWO * quat[SGD_X] * quat[SGD_Y]) +
                          (SGD_TWO * quat[SGD_W] * quat[SGD_Z]);
//matrix[1][1] = SGD_ONE - (SGD_TWO * quat->x * quat->x)
//                      - (SGD_TWO * quat->z * quat->z);
//matrix[1][2] = (SGD_TWO * quat->y * quat->z) - (SGD_TWO * quat->w * quat->x);

  matrix[2][0] = (SGD_TWO * quat[SGD_X] * quat[SGD_Z]) -
                 (SGD_TWO * quat[SGD_W] * quat[SGD_Y]);
  matrix[2][1] = (SGD_TWO * quat[SGD_Y] * quat[SGD_Z]) +
                 (SGD_TWO * quat[SGD_W] * quat[SGD_X]);
  matrix[2][2] = SGD_ONE - (SGD_TWO * quat[SGD_X] * quat[SGD_X])
                        - (SGD_TWO * quat[SGD_Y] * quat[SGD_Y]);

  sy = -matrix[2][0];
  cy = sqrt(SGD_ONE - (sy * sy));
  yr = (SGDfloat)atan2(sy,cy);
  hpr[1] = yr * SGD_RADIANS_TO_DEGREES ;

  // AVOID DIVIDE BY ZERO ERROR ONLY WHERE Y= +-90 or +-270 
  // NOT CHECKING cy BECAUSE OF PRECISION ERRORS
  if (sy != SGD_ONE && sy != -SGD_ONE)	
  {
    cx = matrix[2][2] / cy;
    sx = matrix[2][1] / cy;
    hpr[0] = ((SGDfloat)atan2(sx,cx)) * SGD_RADIANS_TO_DEGREES ;

    cz = matrix[0][0] / cy;
    sz = matrix[1][0] / cy;
    hpr[2] = ((SGDfloat)atan2(sz,cz)) * SGD_RADIANS_TO_DEGREES ;
  }
  else
  {
    // SINCE Cos(Y) IS 0, I AM SCREWED.  ADOPT THE STANDARD Z = 0
    // I THINK THERE IS A WAY TO FIX THIS BUT I AM NOT SURE.  EULERS SUCK
    // NEED SOME MORE OF THE MATRIX TERMS NOW

    matrix[1][1] = SGD_ONE - (SGD_TWO * quat[SGD_X] * quat[SGD_X])
                          - (SGD_TWO * quat[SGD_Z] * quat[SGD_Z]);
    matrix[1][2] = (SGD_TWO * quat[SGD_Y] * quat[SGD_Z]) -
                   (SGD_TWO * quat[SGD_W] * quat[SGD_X]);

    cx =  matrix[1][1];
    sx = -matrix[1][2];
    hpr[0] = ((SGDfloat)atan2(sx,cx)) * SGD_RADIANS_TO_DEGREES ;

    cz = SGD_ONE ;
    sz = SGD_ZERO ;
    hpr[2] = ((SGDfloat)atan2(sz,cz)) * SGD_RADIANS_TO_DEGREES ;
  }
}


void sgdQuatToMatrix ( sgdMat4 dst, sgdQuat q )
{
  SGDfloat two_xx = q[SGD_X] * (q[SGD_X] + q[SGD_X]) ;
  SGDfloat two_xy = q[SGD_X] * (q[SGD_Y] + q[SGD_Y]) ;
  SGDfloat two_xz = q[SGD_X] * (q[SGD_Z] + q[SGD_Z]) ;

  SGDfloat two_wx = q[SGD_W] * (q[SGD_X] + q[SGD_X]) ;
  SGDfloat two_wy = q[SGD_W] * (q[SGD_Y] + q[SGD_Y]) ;
  SGDfloat two_wz = q[SGD_W] * (q[SGD_Z] + q[SGD_Z]) ;

  SGDfloat two_yy = q[SGD_Y] * (q[SGD_Y] + q[SGD_Y]) ;
  SGDfloat two_yz = q[SGD_Y] * (q[SGD_Z] + q[SGD_Z]) ;

  SGDfloat two_zz = q[SGD_Z] * (q[SGD_Z] + q[SGD_Z]) ;

  sgdSetVec4 ( dst[0], SGD_ONE-(two_yy+two_zz), two_xy-two_wz, two_xz+two_wy, SGD_ZERO ) ;
  sgdSetVec4 ( dst[1], two_xy+two_wz, SGD_ONE-(two_xx+two_zz), two_yz-two_wx, SGD_ZERO ) ;
  sgdSetVec4 ( dst[2], two_xz-two_wy, two_yz+two_wx, SGD_ONE-(two_xx+two_yy), SGD_ZERO ) ;
  sgdSetVec4 ( dst[3], SGD_ZERO, SGD_ZERO, SGD_ZERO, SGD_ONE ) ;
}


//from gamasutra.com
//by nb@netcom.ca 

/************************************
 DEPRECATED - use sgdQuatToMatrix instead.
*************************************/

void sgdMakeRotMat42( sgdMat4 m, sgdQuat quat ){
  SGDfloat wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

  // calculate coefficients
  x2 = quat[SGD_X] + quat[SGD_X]; y2 = quat[SGD_Y] + quat[SGD_Y]; 
  z2 = quat[SGD_Z] + quat[SGD_Z];
  xx = quat[SGD_X] * x2;   xy = quat[SGD_X] * y2;   xz = quat[SGD_X] * z2;
  yy = quat[SGD_Y] * y2;   yz = quat[SGD_Y] * z2;   zz = quat[SGD_Z] * z2;
  wx = quat[SGD_W] * x2;   wy = quat[SGD_W] * y2;   wz = quat[SGD_W] * z2;

  m[0][0] = SGD_ONE- (yy + zz); 	m[0][1] = xy - wz;
  m[0][2] = xz + wy;		m[0][3] = SGD_ZERO ;
 
  m[1][0] = xy + wz;		m[1][1] = SGD_ONE- (xx + zz);
  m[1][2] = yz - wx;		m[1][3] = SGD_ZERO ;

  m[2][0] = xz - wy;		m[2][1] = yz + wx;
  m[2][2] = SGD_ONE- (xx + yy);		m[2][3] = SGD_ZERO ;

  m[3][0] = 0;			m[3][1] = 0;
  m[3][2] = 0;			m[3][3] = 1;
}



//from gamasutra.com
//by nb@netcom.ca 
void sgdSlerpQuat2( sgdQuat dst, const sgdQuat from, const sgdQuat to, const SGDfloat t )
{
	SGDfloat           to1[4];
	SGDfloat        omega, cosom, sinom, scale0, scale1;

        // calc cosine
        cosom = from[SGD_X] * to[SGD_X] + from[SGD_Y] * to[SGD_Y] + from[SGD_Z] * to[SGD_Z]
			       + from[SGD_W] * to[SGD_W];

        // adjust signs (if necessary)
        if ( cosom <SGD_ZERO  ){ 
			cosom = -cosom; 
			to1[0] = - to[SGD_X];
		to1[1] = - to[SGD_Y];
		to1[2] = - to[SGD_Z];
		to1[3] = - to[SGD_W];
        } else  {
		to1[0] = to[SGD_X];
		to1[1] = to[SGD_Y];
		to1[2] = to[SGD_Z];
		to1[3] = to[SGD_W];
        }

        // calculate coefficients
#define DELTA SGD_ZERO 
       if ( (SGD_ONE- cosom) > DELTA ) {
                // standard case (slerp)
                omega = acos(cosom);
                sinom = sin(omega);
                scale0 = sin((SGD_ONE- t) * omega) / sinom;
                scale1 = sin(t * omega) / sinom;

        } else {        
    // "from" and "to" quaternions are very close 
	    //  ... so we can do a linear interpolation
                scale0 = SGD_ONE- t;
                scale1 = t;
        }
	// calculate final values
	dst[SGD_X] = scale0 * from[SGD_X] + scale1 * to1[0];
	dst[SGD_Y] = scale0 * from[SGD_Y] + scale1 * to1[1];
	dst[SGD_Z] = scale0 * from[SGD_Z] + scale1 * to1[2];
	dst[SGD_W] = scale0 * from[SGD_W] + scale1 * to1[3];
}

void sgdSlerpQuat( sgdQuat dst, const sgdQuat from, const sgdQuat to, const SGDfloat t )
{
  SGDfloat sign, co, scale0, scale1;

  /* SWC - Interpolate between to quaternions */

  co = from[SGD_X] * to[SGD_X] + from[SGD_Y] * to[SGD_Y] + from[SGD_X] * to[SGD_Z] + 
	  from[SGD_W] * to[SGD_W];

  if( co < SGD_ZERO )
  {
    co = -co;
    sign = -SGD_ONE;
  }
  else
    sign = SGD_ONE;

  if( co < SGD_ONE )
  {
    SGDfloat o = (SGDfloat)acos( co );
    SGDfloat so = (SGDfloat)sin( o );

    scale0 = (SGDfloat)sin( (SGD_ONE - t) * o ) / so;
    scale1 = (SGDfloat)sin( t * o ) / so;
  }
  else
  {
    scale0 = SGD_ONE - t;
    scale1 = t;
  }

  dst[SGD_X] = scale0 * from[SGD_X] + scale1 * ((sign > SGD_ZERO) ? to[SGD_X] : -to[SGD_X]);
  dst[SGD_Y] = scale0 * from[SGD_Y] + scale1 * ((sign > SGD_ZERO) ? to[SGD_Y] : -to[SGD_Y]);
  dst[SGD_Z] = scale0 * from[SGD_Z] + scale1 * ((sign > SGD_ZERO) ? to[SGD_Z] : -to[SGD_Z]);
  dst[SGD_W] = scale0 * from[SGD_W] + scale1 * ((sign > SGD_ZERO) ? to[SGD_W] : -to[SGD_W]);
}


 
void sgdReflectInPlaneVec3 ( sgdVec3 dst, const sgdVec3 src, const sgdVec4 plane )
{
  SGDfloat src_dot_norm  = sgdScalarProductVec3 ( src, plane ) ;
 
  sgdVec3 tmp ;

  sgdScaleVec3 ( tmp, plane, SGD_TWO * src_dot_norm ) ;
  sgdSubVec3 ( dst, src, tmp ) ;
}


