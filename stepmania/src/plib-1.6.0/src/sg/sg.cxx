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

sgVec3 _sgGravity = { 0.0f, 0.0f, -9.8f } ;

void sgVectorProductVec3 ( sgVec3 dst, const sgVec3 a, const sgVec3 b )
{
  dst[0] = a[1] * b[2] - a[2] * b[1] ;
  dst[1] = a[2] * b[0] - a[0] * b[2] ;
  dst[2] = a[0] * b[1] - a[1] * b[0] ;
}

inline SGfloat _sgClampToUnity ( const SGfloat x )
{
  if ( x >  SG_ONE ) return  SG_ONE ;
  if ( x < -SG_ONE ) return -SG_ONE ;
  return x ;
}

int sgCompare3DSqdDist( const sgVec3 v1, const sgVec3 v2, const SGfloat sqd_dist )
{
  sgVec3 tmp ;

  sgSubVec3 ( tmp, v2, v1 ) ;

  SGfloat sqdist = tmp[0] * tmp[0] + tmp[1] * tmp[1] + tmp[2] * tmp[2] ;

  if ( sqdist > sqd_dist ) return  1 ;
  if ( sqdist < sqd_dist ) return -1 ;
  return 0 ;
}

void sgMakeRotMat4( sgMat4 mat, const SGfloat angle, const sgVec3 axis )
{
  sgVec3 ax ;
  sgNormalizeVec3 ( ax, axis ) ; 

  SGfloat temp_angle = angle * SG_DEGREES_TO_RADIANS ;
  SGfloat s = (SGfloat) sin ( temp_angle ) ;
  SGfloat c = (SGfloat) cos ( temp_angle ) ;
  SGfloat t = SG_ONE - c ;
   
  mat[0][0] = t * ax[0] * ax[0] + c ;
  mat[0][1] = t * ax[0] * ax[1] + s * ax[2] ;
  mat[0][2] = t * ax[0] * ax[2] - s * ax[1] ;
  mat[0][3] = SG_ZERO ;

  mat[1][0] = t * ax[1] * ax[0] - s * ax[2] ;
  mat[1][1] = t * ax[1] * ax[1] + c ;
  mat[1][2] = t * ax[1] * ax[2] + s * ax[0] ;
  mat[1][3] = SG_ZERO ;

  mat[2][0] = t * ax[2] * ax[0] + s * ax[1] ;
  mat[2][1] = t * ax[2] * ax[1] - s * ax[0] ;
  mat[2][2] = t * ax[2] * ax[2] + c ;
  mat[2][3] = SG_ZERO ;

  mat[3][0] = SG_ZERO ;
  mat[3][1] = SG_ZERO ;
  mat[3][2] = SG_ZERO ;
  mat[3][3] = SG_ONE ;
}



 
void sgMakePickMatrix( sgMat4 mat, sgFloat x, sgFloat y,
                       sgFloat width, sgFloat height, sgVec4 viewport )
{
   sgFloat sx =   viewport[2] / width  ;
   sgFloat sy =   viewport[3] / height ;
   sgFloat tx = ( viewport[2] + SG_TWO * (viewport[0] - x) ) / width  ;
   sgFloat ty = ( viewport[3] + SG_TWO * (viewport[1] - y) ) / height ;
 
   mat[0][0] =    sx   ;
   mat[0][1] = SG_ZERO ;
   mat[0][2] = SG_ZERO ;
   mat[0][3] = SG_ZERO ;

   mat[1][0] = SG_ZERO ;
   mat[1][1] =    sy   ;
   mat[1][2] = SG_ZERO ;
   mat[1][3] = SG_ZERO ;

   mat[2][0] = SG_ZERO ;
   mat[2][1] = SG_ZERO ;
   mat[2][2] = SG_ONE  ;
   mat[2][3] = SG_ZERO ;

   mat[3][0] =    tx   ;
   mat[3][1] =    ty   ;
   mat[3][2] = SG_ZERO ;
   mat[3][3] = SG_ONE  ;
}                                                                               



void sgMakeLookAtMat4 ( sgMat4 dst, const sgVec3 eye,
                                    const sgVec3 center,
                                    const sgVec3 up )
{
  // Caveats:
  // 1) In order to compute the line of sight, the eye point must not be equal
  //    to the center point.
  // 2) The up vector must not be parallel to the line of sight from the eye
  //    to the center point.

  /* Compute the direction vectors */
  sgVec3 x,y,z;

  /* Y vector = center - eye */
  sgSubVec3 ( y, center, eye ) ;

  /* Z vector = up */
  sgCopyVec3 ( z, up ) ;

  /* X vector = Y cross Z */
  sgVectorProductVec3 ( x, y, z ) ;

  /* Recompute Z = X cross Y */
  sgVectorProductVec3 ( z, x, y ) ;

  /* Normalize everything */
  sgNormaliseVec3 ( x ) ;
  sgNormaliseVec3 ( y ) ;
  sgNormaliseVec3 ( z ) ;

  /* Build the matrix */
  sgSetVec4 ( dst[0], x[0], x[1], x[2], SG_ZERO ) ;
  sgSetVec4 ( dst[1], y[0], y[1], y[2], SG_ZERO ) ;
  sgSetVec4 ( dst[2], z[0], z[1], z[2], SG_ZERO ) ;
  sgSetVec4 ( dst[3], eye[0], eye[1], eye[2], SG_ONE ) ;
}

// -dw- inconsistent linkage!
float sgTriArea( sgVec3 p0, sgVec3 p1, sgVec3 p2 )
{
  /* 
    From comp.graph.algorithms FAQ
	2A(P) = abs(N.(sum_{i=0}^{n-1}(v_i x v_{i+1})))
	This is an optimized version for a triangle
	but easily extended for planar polygon's with more sides
	by passing in the number of sides and the vv array
	sgTriArea( int nsides, float **vv )
	and changing the normal calculation and the for loop appropriately
	sgMakeNormal( norm, vv[0], vv[1], vv[2] )
	for( int i=0; i<n; i++ )
  */

	sgVec3 sum;
	sgZeroVec3( sum );

	sgVec3 norm;
	sgMakeNormal( norm, p0, p1, p2 );

	float *vv[3];
	vv[0] = p0;
	vv[1] = p1;
	vv[2] = p2;

	for( int i=0; i<3; i++ ) {
		int ii = (i+1) % 3;
		sum[0] += (vv[i][1] * vv[ii][2] - vv[i][2] * vv[ii][1]) ;
		sum[1] += (vv[i][2] * vv[ii][0] - vv[i][0] * vv[ii][2]) ;
		sum[2] += (vv[i][0] * vv[ii][1] - vv[i][1] * vv[ii][0]) ;
	}

	float area = sgAbs(sgScalarProductVec3( norm, sum ));
	return( area / 2.0f );
}

/***************************************************\
*   functions to get the angle between two vectors  *
\***************************************************/

SGfloat sgAngleBetweenVec3 ( sgVec3 v1, sgVec3 v2 )
{
  sgVec3 nv1, nv2 ;

  sgNormalizeVec3 ( nv1, v1 ) ;
  sgNormalizeVec3 ( nv2, v2 ) ;
  return sgAngleBetweenNormalizedVec3 ( nv1, nv2 ) ;
}

SGfloat sgAngleBetweenNormalizedVec3 (sgVec3 first, sgVec3 second, sgVec3 normal) { 
 // result is in the range  0..2*pi
 // Attention: first and second have to be normalized
 // the normal is needed to decide between for example 0.123 looking "from one side"
 // and -0.123 looking fomr the other

  SGfloat myCos, abs1, abs2, SProduct, deltaAngle, myNorm;

#ifdef _DEBUG
	SGfloat tfA,tfB;

	tfA = sgScalarProductVec3 (first, first);
	tfB = sgScalarProductVec3 (second, second);
	assert(tfA>0.99);
	assert(tfA<1.01);
	assert(tfB>0.99);
	assert(tfB<1.01);
#endif
  
	if((normal[0]==0) && (normal[1]==0) && (normal[2]==0))
	{	
		ulSetError ( UL_WARNING, "sgGetAngleBetweenVectors: Normal is zero.");
		return 0.0;
	}
	sgVec3 temp;
  sgVectorProductVec3( temp, first, second);
  myNorm = sgLengthVec3 ( temp );
  if ( (sgScalarProductVec3(temp, normal))<0 ) {
    myNorm = -myNorm;
  }

  if ( myNorm < -0.99999 ) {
    deltaAngle = -SG_PI*0.5;
  }
  else
  {   if ( myNorm > 0.99999 ) {
         deltaAngle = SG_PI*0.5;
      }
      else
      {   deltaAngle = (SGfloat)asin((double)myNorm);
      }
  }
	// deltaAngle is in the range -SG_PI*0.5 to +SG_PI*0.5 here
	// However, the correct result could also be 
	// deltaAngleS := pi - deltaAngle
	// Please note that:
	// cos(deltaAngleS)=cos(pi-deltaAngle)=-cos(deltaAngle)
	// So, the question is whether + or - cos(deltaAngle)
	// is sgScalarProductVec3(first, second)
  
  if ( deltaAngle < 0 ) 
    deltaAngle = deltaAngle + 2*SG_PI; // unnessecary?
  
  SProduct = sgScalarProductVec3(first, second);
  myCos = (SGfloat) cos(deltaAngle);
  
	abs1 = SProduct - myCos;
  if ( abs1 < 0 ) 
		abs1 = -abs1;
  
	abs2 = SProduct + myCos;
  if ( abs2 < 0 ) 
    abs2 = -abs2;
  
	assert( (abs1 < 0.1) || (abs2 < 0.1) );
  if ( abs2 < abs1 ) { // deltaAngleS is the correct result
    if ( deltaAngle <= SG_PI ) {
      deltaAngle = SG_PI - deltaAngle;
    }else {
      deltaAngle = 3*SG_PI - deltaAngle;
    }
  }
	assert(deltaAngle >= 0.0);
	assert(deltaAngle <= 2.0*SG_PI);
  return deltaAngle;
}

SGfloat sgAngleBetweenVec3 ( sgVec3 v1, sgVec3 v2, sgVec3 normal )
// nornmal has to be normalized.
{
  sgVec3 nv1, nv2 ;

  sgNormalizeVec3 ( nv1, v1 ) ;
  sgNormalizeVec3 ( nv2, v2 ) ;
  return sgAngleBetweenNormalizedVec3 ( nv1, nv2, normal ) ;
}


/*********************\
*    sgBox routines   *
\*********************/


void sgBox::extend ( const sgVec3 v )
{
  if ( isEmpty () )
  {
    sgCopyVec3 ( min, v ) ;
    sgCopyVec3 ( max, v ) ;
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


void sgBox::extend ( const sgBox *b )
{
  if ( b -> isEmpty () )
    return ;

  if ( isEmpty () )
  {
    sgCopyVec3 ( min, b->getMin() ) ;
    sgCopyVec3 ( max, b->getMax() ) ;
  }
  else
  {
    extend ( b->getMin() ) ;
    extend ( b->getMax() ) ;
  }
}


void sgBox::extend ( const sgSphere *s )
{
  if ( s -> isEmpty () ) 
    return ;

  /*
    In essence, this extends around a box around the sphere - which
    is still a perfect solution because both boxes are axially aligned.
  */

  sgVec3 x ;

  sgSetVec3 ( x, s->getCenter()[0]+s->getRadius(),
                 s->getCenter()[1]+s->getRadius(),
                 s->getCenter()[2]+s->getRadius() ) ;
  extend ( x ) ;

  sgSetVec3 ( x, s->getCenter()[0]-s->getRadius(),
                 s->getCenter()[1]-s->getRadius(),
                 s->getCenter()[2]-s->getRadius() ) ;
  extend ( x ) ;
}


int sgBox::intersects ( const sgVec4 plane ) const 
{
  /*
    Save multiplies by not redoing Ax+By+Cz+D for each point.
  */

  SGfloat Ax_min        = plane[0] * min[0] ;
  SGfloat By_min        = plane[1] * min[1] ;
  SGfloat Cz_min_plus_D = plane[2] * min[2] + plane[3] ;

  SGfloat Ax_max        = plane[0] * max[0] ;
  SGfloat By_max        = plane[1] * max[1] ;
  SGfloat Cz_max_plus_D = plane[2] * max[2] + plane[3] ;

  /*
    Count the number of vertices on the positive side of the plane.
  */

  int count = ( Ax_min + By_min + Cz_min_plus_D > SG_ZERO ) +
              ( Ax_min + By_min + Cz_max_plus_D > SG_ZERO ) +
              ( Ax_min + By_max + Cz_min_plus_D > SG_ZERO ) +
              ( Ax_min + By_max + Cz_max_plus_D > SG_ZERO ) +
              ( Ax_max + By_min + Cz_min_plus_D > SG_ZERO ) +
              ( Ax_max + By_min + Cz_max_plus_D > SG_ZERO ) +
              ( Ax_max + By_max + Cz_min_plus_D > SG_ZERO ) +
              ( Ax_max + By_max + Cz_max_plus_D > SG_ZERO ) ;

  /*
    The plane intersects the box unless all 8 are positive
    or none of them are positive.
  */
              
  return count != 0 && count != 8 ;
}



/**********************\
*  sgSphere routines   *
\**********************/

void sgSphere::extend ( const sgVec3 v )
{
  if ( isEmpty () )
  {
    sgCopyVec3 ( center, v ) ;
    radius = SG_ZERO ;
    return ;
  }

  SGfloat d = sgDistanceVec3 ( center, v ) ;

  if ( d <= radius )  /* Point is already inside sphere */
    return ;

  SGfloat new_radius = (radius + d) / SG_TWO ;  /* Grow radius */

  SGfloat ratio = (new_radius - radius) / d ;

  center[0] += (v[0]-center[0]) * ratio ;    /* Move center */
  center[1] += (v[1]-center[1]) * ratio ;
  center[2] += (v[2]-center[2]) * ratio ;

  radius = new_radius ;
}


void sgSphere::extend ( const sgBox *b )
{
  if ( b -> isEmpty () )
    return ;

  if ( isEmpty() )
  {
    sgAddVec3   ( center, b->getMin(), b->getMax() ) ;
    sgScaleVec3 ( center, SG_HALF ) ;
    radius = sgDistanceVec3 ( center, b->getMax() ) ;
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
  sgSphere s ;
  s.empty   ()    ;
  s.enclose ( b ) ;  /* Fast because s is empty */
    enclose ( s ) ;

#else

  /* TIGHTER/EXPENSIVE sphere-around-eight-points */
  sgVec3 x ;
                                                        extend ( b->getMin() ) ;
  sgSetVec3 ( x, b->getMin()[0],b->getMin()[1],b->getMax()[2] ) ; extend ( x ) ;
  sgSetVec3 ( x, b->getMin()[0],b->getMax()[1],b->getMin()[2] ) ; extend ( x ) ;
  sgSetVec3 ( x, b->getMin()[0],b->getMax()[1],b->getMax()[2] ) ; extend ( x ) ;
  sgSetVec3 ( x, b->getMax()[0],b->getMin()[1],b->getMin()[2] ) ; extend ( x ) ;
  sgSetVec3 ( x, b->getMax()[0],b->getMin()[1],b->getMax()[2] ) ; extend ( x ) ;
  sgSetVec3 ( x, b->getMax()[0],b->getMax()[1],b->getMin()[2] ) ; extend ( x ) ;
                                                        extend ( b->getMax() ) ;
#endif
}


void sgSphere::extend ( const sgSphere *s )
{
  if ( s->isEmpty () )
    return ;

  if ( isEmpty () )
  {
    sgCopyVec3 ( center, s->getCenter() ) ;
    radius = s->getRadius() ;
    return ;
  }

  /* 
    d == The distance between the sphere centers
  */

  SGfloat d = sgDistanceVec3 ( center, s->getCenter() ) ;

  if ( d + s->getRadius() <= radius )  /* New sphere is already inside this one */
    return ;

  if ( d + radius <= s->getRadius() )  /* New sphere completely contains this one */
  {
    sgCopyVec3 ( center, s->getCenter() ) ;
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

  SGfloat new_radius = (radius + d + s->getRadius() ) / SG_TWO ;

  SGfloat ratio = ( new_radius - radius ) / d ;

  center[0] += ( s->getCenter()[0] - center[0] ) * ratio ;
  center[1] += ( s->getCenter()[1] - center[1] ) * ratio ;
  center[2] += ( s->getCenter()[2] - center[2] ) * ratio ;
  radius = new_radius ;
}


int sgSphere::intersects ( const sgBox *b ) const 
{
  sgVec3 closest ;

  if ( b->getMin()[0] > center[0] ) closest[0] = b->getMin()[0] ; else
  if ( b->getMax()[0] < center[0] ) closest[0] = b->getMax()[0] ; else
                                    closest[0] = center[0] ;

  if ( b->getMin()[1] > center[1] ) closest[1] = b->getMin()[1] ; else
  if ( b->getMax()[1] < center[1] ) closest[1] = b->getMax()[1] ; else
                                    closest[1] = center[1] ;

  if ( b->getMin()[2] > center[2] ) closest[2] = b->getMin()[2] ; else
  if ( b->getMax()[2] < center[2] ) closest[2] = b->getMax()[2] ; else
                                    closest[2] = center[2] ;

  return sgCompare3DSqdDist ( closest, center, sgSquare ( radius ) ) <= 0 ;
}


/************************\
*   sgFrustum routines   *
\************************/

void sgFrustum::update ()
{
  if ( fabs ( ffar - nnear ) < 0.1 )
  {
    ulSetError ( UL_WARNING, "sgFrustum: Can't support depth of view <0.1 units.");
    return ;
  }

  if ( hfov != SG_ZERO && vfov != SG_ZERO )
  {
    if ( fabs ( hfov ) < 0.1 || fabs ( vfov ) < 0.1 )
    {
      ulSetError ( UL_WARNING, "sgFrustum: Can't support fields of view narrower than 0.1 degrees.");
      return ;
    }

    /* Corners of screen relative to eye... */
  
    right = nnear * (SGfloat) tan ( hfov * SG_DEGREES_TO_RADIANS / SG_TWO ) ;
    top   = nnear * (SGfloat) tan ( vfov * SG_DEGREES_TO_RADIANS / SG_TWO ) ;
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

  sgVec3 v1, v2, v3, v4 ;

  sgSetVec3 ( v1, left , top, -nnear ) ;
  sgSetVec3 ( v2, right, top, -nnear ) ;
  sgSetVec3 ( v3, left , bot, -nnear ) ;
  sgSetVec3 ( v4, right, bot, -nnear ) ;

  sgNormaliseVec3 ( v1 ) ;
  sgNormaliseVec3 ( v2 ) ;
  sgNormaliseVec3 ( v3 ) ;
  sgNormaliseVec3 ( v4 ) ;

  /*
    Take care of the order of the parameters so that all the planes
    are oriented facing inwards...
  */

  sgVectorProductVec3 (   top_plane, v1, v2 ) ;
  sgVectorProductVec3 ( right_plane, v2, v4 ) ;
  sgVectorProductVec3 (   bot_plane, v4, v3 ) ;
  sgVectorProductVec3 (  left_plane, v3, v1 ) ;

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

  SGfloat w = right - left ;
  SGfloat h = top   - bot  ;
  SGfloat d = ffar  - nnear ;

  mat[0][0] =  SG_TWO * nnear / w ;
  mat[0][1] =  SG_ZERO ;
  mat[0][2] =  SG_ZERO ;
  mat[0][3] =  SG_ZERO ;

  mat[1][0] =  SG_ZERO ;
  mat[1][1] =  SG_TWO * nnear / h ;
  mat[1][2] =  SG_ZERO ;
  mat[1][3] =  SG_ZERO ;

  mat[2][0] =  ( right + left ) / w ;
  mat[2][1] =  ( top   + bot  ) / h ;
  mat[2][2] = -( ffar  + nnear ) / d ;
  mat[2][3] = -SG_ONE ;

  mat[3][0] =  SG_ZERO ;
  mat[3][1] =  SG_ZERO ;
  mat[3][2] = -SG_TWO * nnear * ffar/ d ;
  mat[3][3] =  SG_ZERO ;
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

int sgFrustum::getOutcode ( const sgVec3 pt ) const 
{
  /* Transform the point by the Frustum's transform. */

  sgVec4 tmp ;

  tmp [ 0 ] = pt [ 0 ] ;
  tmp [ 1 ] = pt [ 1 ] ;
  tmp [ 2 ] = pt [ 2 ] ;
  tmp [ 3 ] =  SG_ONE  ;

  sgXformPnt4 ( tmp, tmp, mat ) ;

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

int sgFrustum::contains ( const sgVec3 pt ) const 
{
  return getOutcode ( pt ) == OC_ALL_ON_SCREEN ;
}


int sgFrustum::contains ( const sgSphere *s ) const 
{
  /*
    Lop off half the database (roughly) with a quick near-plane test - and
    lop off a lot more with a quick far-plane test
  */

  if ( -s->getCenter() [ 2 ] + s->getRadius() < nnear ||
       -s->getCenter() [ 2 ] - s->getRadius() > ffar )
    return SG_OUTSIDE ;

  /*
    OK, so the sphere lies between near and far.

    Measure the distance of the center point from the four sides of the frustum,
    if it's outside by more than the radius then it's history.

    It's tempting to do a quick test to see if the center point is
    onscreen using sgFrustumContainsPt - but that takes a matrix transform
    which is 16 multiplies and 12 adds - versus this test which does the
    whole task using only 12 multiplies and 8 adds.
  */

  SGfloat sp1 = sgScalarProductVec3 (  left_plane, s->getCenter() ) ;
  SGfloat sp2 = sgScalarProductVec3 ( right_plane, s->getCenter() ) ;
  SGfloat sp3 = sgScalarProductVec3 (   bot_plane, s->getCenter() ) ;
  SGfloat sp4 = sgScalarProductVec3 (   top_plane, s->getCenter() ) ;

  if ( -sp1 >= s->getRadius() || -sp2 >= s->getRadius() ||
       -sp3 >= s->getRadius() || -sp4 >= s->getRadius() )
    return SG_OUTSIDE ;
  
  /*
    If it's inside by more than the radius then it's *completely* inside
    and we can save time elsewhere if we know that for sure.
  */

  if ( -s->getCenter() [ 2 ] - s->getRadius() > nnear &&
       -s->getCenter() [ 2 ] + s->getRadius() < ffar &&
       sp1 >= s->getRadius() && sp2 >= s->getRadius() &&
       sp3 >= s->getRadius() && sp4 >= s->getRadius() )
    return SG_INSIDE ;

  return SG_STRADDLE ;
}



SGfloat sgDistSquaredToLineVec3 ( const sgLine3 line, const sgVec3 pnt )
{
  sgVec3 r ; sgSubVec3 ( r, pnt, line.point_on_line ) ;
 
  return sgScalarProductVec3 ( r, r ) -
         sgScalarProductVec3 ( r, line.direction_vector ) ;
}



SGfloat sgDistSquaredToLineSegmentVec3 ( const sgLineSegment3 line,
                                         const sgVec3 pnt )
{
  sgLine3 l ; sgLineSegment3ToLine3 ( & l, line ) ;
 
sgVec3 v ; sgSubVec3 ( v, line.b, line.a ) ;
  sgVec3 r1 ; sgSubVec3 ( r1, pnt, line.a ) ;
 
  SGfloat r1_dot_v = sgScalarProductVec3 ( r1, v /*l.direction_vector*/ ) ;
 
  if ( r1_dot_v <= 0 )  /* Off the "A" end  */
    return sgScalarProductVec3 ( r1, r1 ) ;
 
  sgVec3 r2 ; sgSubVec3 ( r2, pnt, line.b ) ;

  SGfloat r2_dot_v = sgScalarProductVec3 ( r2, v /*l.direction_vector*/ ) ;
 
  if ( r2_dot_v >= 0 )  /* Off the "B" end */
    return sgScalarProductVec3 ( r2, r2 ) ;
 
  /* Closest point on line is on the line segment */
 
  return sgScalarProductVec3 ( r1, r1 ) - r1_dot_v ;
}



void sgMakeCoordMat4 ( sgMat4 m, const SGfloat x, const SGfloat y, const SGfloat z, const SGfloat h, const SGfloat p, const SGfloat r )
{
  SGfloat ch, sh, cp, sp, cr, sr, srsp, crsp, srcp ;

  if ( h == SG_ZERO )
  {
    ch = SGD_ONE ;
    sh = SGD_ZERO ;
  }
  else
  {
    sh = sgSin ( h ) ;
    ch = sgCos ( h ) ;
  }

  if ( p == SG_ZERO )
  {
    cp = SGD_ONE ;
    sp = SGD_ZERO ;
  }
  else
  {
    sp = sgSin ( p ) ;
    cp = sgCos ( p ) ;
  }

  if ( r == SG_ZERO )
  {
    cr   = SGD_ONE ;
    sr   = SGD_ZERO ;
    srsp = SGD_ZERO ;
    srcp = SGD_ZERO ;
    crsp = sp ;
  }
  else
  {
    sr   = sgSin ( r ) ;
    cr   = sgCos ( r ) ;
    srsp = sr * sp ;
    crsp = cr * sp ;
    srcp = sr * cp ;
  }

  m[0][0] = (SGfloat)(  ch * cr - sh * srsp ) ;
  m[1][0] = (SGfloat)( -sh * cp ) ;
  m[2][0] = (SGfloat)(  sr * ch + sh * crsp ) ;
  m[3][0] =  x ;

  m[0][1] = (SGfloat)( cr * sh + srsp * ch ) ;
  m[1][1] = (SGfloat)( ch * cp ) ;
  m[2][1] = (SGfloat)( sr * sh - crsp * ch ) ;
  m[3][1] =  y ;

  m[0][2] = (SGfloat)( -srcp ) ;
  m[1][2] = (SGfloat)(  sp ) ;
  m[2][2] = (SGfloat)(  cr * cp ) ;
  m[3][2] =  z ;

  m[0][3] =  SG_ZERO ;
  m[1][3] =  SG_ZERO ;
  m[2][3] =  SG_ZERO ;
  m[3][3] =  SG_ONE ;
}


void sgMakeTransMat4 ( sgMat4 m, const sgVec3 xyz )
{
  m[0][1] = m[0][2] = m[0][3] =
  m[1][0] = m[1][2] = m[1][3] =
  m[2][0] = m[2][1] = m[2][3] = SG_ZERO ;
  m[0][0] = m[1][1] = m[2][2] = m[3][3] = SG_ONE ;
  sgCopyVec3 ( m[3], xyz ) ;
}


void sgMakeTransMat4 ( sgMat4 m, const SGfloat x, const SGfloat y, const SGfloat z )
{
  m[0][1] = m[0][2] = m[0][3] =
  m[1][0] = m[1][2] = m[1][3] =
  m[2][0] = m[2][1] = m[2][3] = SG_ZERO ;
  m[0][0] = m[1][1] = m[2][2] = m[3][3] = SG_ONE ;
  sgSetVec3 ( m[3], x, y, z ) ;
}


void sgSetCoord ( sgCoord *dst, const sgMat4 src )
{
  sgCopyVec3 ( dst->xyz, src[3] ) ;
    
  sgMat4 mat ;

  SGfloat s = sgLengthVec3 ( src[0] ) ;

  if ( s <= 0.00001 )
  {
    ulSetError ( UL_WARNING, "sgMat4ToCoord: ERROR - Bad Matrix." ) ;
    sgSetVec3 ( dst -> hpr, SG_ZERO, SG_ZERO, SG_ZERO ) ;
    return ;
  }

  sgScaleMat4 ( mat, src, SG_ONE / s ) ;

  dst->hpr[1] = sgASin ( _sgClampToUnity ( mat[1][2] ) ) ;

  SGfloat cp = sgCos ( dst->hpr[1] ) ;
 
  /* If pointing nearly vertically up - then heading is ill-defined */

  if ( cp > -0.00001 && cp < 0.00001 )
  {
    SGfloat cr = _sgClampToUnity ( mat[0][1] ) ; 
    SGfloat sr = _sgClampToUnity (-mat[2][1] ) ;

    dst->hpr[0] = SG_ZERO ;
    dst->hpr[2] = sgATan2 ( sr, cr ) ;
  }
  else
  {
    cp = SG_ONE / cp ;
    SGfloat sr = _sgClampToUnity ( -mat[0][2] * cp ) ;
    SGfloat cr = _sgClampToUnity (  mat[2][2] * cp ) ;
    SGfloat sh = _sgClampToUnity ( -mat[1][0] * cp ) ;
    SGfloat ch = _sgClampToUnity (  mat[1][1] * cp ) ;
	
    if ( (sh == SG_ZERO && ch == SG_ZERO) || (sr == SG_ZERO && cr == SG_ZERO) )
    {
      cr = _sgClampToUnity ( mat[0][1] ) ;
      sr = _sgClampToUnity (-mat[2][1] ) ;

      dst->hpr[0] = SG_ZERO ;
    }
    else
      dst->hpr[0] = sgATan2 ( sh, ch ) ;

    dst->hpr[2] = sgATan2 ( sr, cr ) ;
  }
}


void sgMakeNormal(sgVec3 dst, const sgVec3 a, const sgVec3 b, const sgVec3 c )
{
  sgVec3 ab ; sgSubVec3 ( ab, b, a ) ;
  sgVec3 ac ; sgSubVec3 ( ac, c, a ) ;
  sgVectorProductVec3 ( dst, ab,ac ) ; sgNormaliseVec3 ( dst ) ;
}


void sgPreMultMat4( sgMat4 dst, const sgMat4 src )
{
  sgMat4 mat ;
  sgMultMat4 ( mat, dst, src ) ;
  sgCopyMat4 ( dst, mat ) ;
}

void sgPostMultMat4( sgMat4 dst, const sgMat4 src )
{
  sgMat4 mat ;
  sgMultMat4 ( mat, src, dst ) ;
  sgCopyMat4 ( dst, mat ) ;
}

void sgMultMat4( sgMat4 dst, const sgMat4 m1, const sgMat4 m2 )
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


void sgTransposeNegateMat4 ( sgMat4 dst, const sgMat4 src )
{
  /* Poor man's invert - can be used when matrix is a simple rotate-translate */

  dst[0][0] = src[0][0] ;
  dst[1][0] = src[0][1] ;
  dst[2][0] = src[0][2] ;
  dst[3][0] = - sgScalarProductVec3 ( src[3], src[0] ) ;

  dst[0][1] = src[1][0] ;
  dst[1][1] = src[1][1] ;
  dst[2][1] = src[1][2] ;
  dst[3][1] = - sgScalarProductVec3 ( src[3], src[1] ) ;
                                                                               
  dst[0][2] = src[2][0] ;                                                      
  dst[1][2] = src[2][1] ;                                                      
  dst[2][2] = src[2][2] ;                                                      
  dst[3][2] = - sgScalarProductVec3 ( src[3], src[2] ) ;
                                                                               
  dst[0][3] = SG_ZERO ;
  dst[1][3] = SG_ZERO ;                                                        
  dst[2][3] = SG_ZERO ;                                                        
  dst[3][3] = SG_ONE  ;
}


void sgTransposeNegateMat4 ( sgMat4 dst )
{
  sgMat4 src ;
  sgCopyMat4 ( src, dst ) ;
  sgTransposeNegateMat4 ( dst, src ) ;
}



void sgInvertMat4 ( sgMat4 dst, const sgMat4 src )
{
  sgMat4 tmp ;

  sgCopyMat4 ( tmp, src ) ;
  sgMakeIdentMat4 ( dst ) ;

  for ( int i = 0 ; i != 4 ; i++ )
  {
    SGfloat val = tmp[i][i] ;
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
        SGfloat t ;
        t = dst[j][i]; dst[j][i] = dst[j][ind]; dst[j][ind] = t ;
        t = tmp[j][i]; tmp[j][i] = tmp[j][ind]; tmp[j][ind] = t ;
      }
    }

    // if ( val == SG_ZERO)
    if ( fabs(val) <= FLT_EPSILON )
    {
      ulSetError ( UL_WARNING, "sg: ERROR - Singular matrix, no inverse!" ) ;
      sgMakeIdentMat4 ( dst ) ;  /* Do *something* */
      return;
    }

    SGfloat ival = SG_ONE / val ;

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



void sgXformVec3 ( sgVec3 dst, const sgVec3 src, const sgMat4 mat )
{
  SGfloat t0 = src[ 0 ] ;
  SGfloat t1 = src[ 1 ] ;
  SGfloat t2 = src[ 2 ] ;

  dst[0] = t0 * mat[ 0 ][ 0 ] +
           t1 * mat[ 1 ][ 0 ] +
           t2 * mat[ 2 ][ 0 ] ;

  dst[1] = t0 * mat[ 0 ][ 1 ] +
           t1 * mat[ 1 ][ 1 ] +
           t2 * mat[ 2 ][ 1 ] ;

  dst[2] = t0 * mat[ 0 ][ 2 ] +
           t1 * mat[ 1 ][ 2 ] +
           t2 * mat[ 2 ][ 2 ] ;
}


void sgXformPnt3 ( sgVec3 dst, const sgVec3 src, const sgMat4 mat )
{
  SGfloat t0 = src[ 0 ] ;
  SGfloat t1 = src[ 1 ] ;
  SGfloat t2 = src[ 2 ] ;

  dst[0] = t0 * mat[ 0 ][ 0 ] +
           t1 * mat[ 1 ][ 0 ] +
           t2 * mat[ 2 ][ 0 ] +
                mat[ 3 ][ 0 ] ;

  dst[1] = t0 * mat[ 0 ][ 1 ] +
           t1 * mat[ 1 ][ 1 ] +
           t2 * mat[ 2 ][ 1 ] +
                mat[ 3 ][ 1 ] ;

  dst[2] = t0 * mat[ 0 ][ 2 ] +
           t1 * mat[ 1 ][ 2 ] +
           t2 * mat[ 2 ][ 2 ] +
                mat[ 3 ][ 2 ] ;
}


void sgXformPnt4 ( sgVec4 dst, const sgVec4 src, const sgMat4 mat )
{
  SGfloat t0 = src[ 0 ] ;
  SGfloat t1 = src[ 1 ] ;
  SGfloat t2 = src[ 2 ] ;
  SGfloat t3 = src[ 3 ] ;

  dst[0] = t0 * mat[ 0 ][ 0 ] +
           t1 * mat[ 1 ][ 0 ] +
           t2 * mat[ 2 ][ 0 ] +
           t3 * mat[ 3 ][ 0 ] ;

  dst[1] = t0 * mat[ 0 ][ 1 ] +
           t1 * mat[ 1 ][ 1 ] +
           t2 * mat[ 2 ][ 1 ] +
           t3 * mat[ 3 ][ 1 ] ;

  dst[2] = t0 * mat[ 0 ][ 2 ] +
           t1 * mat[ 1 ][ 2 ] +
           t2 * mat[ 2 ][ 2 ] +
           t3 * mat[ 3 ][ 2 ] ;

  dst[3] = t0 * mat[ 0 ][ 3 ] +
           t1 * mat[ 1 ][ 3 ] +
           t2 * mat[ 2 ][ 3 ] +
           t3 * mat[ 3 ][ 3 ] ;
}


void sgFullXformPnt3 ( sgVec3 dst, const sgVec3 src, const sgMat4 mat )
{
  sgVec4 tmp ;

  tmp [ 0 ] = src [ 0 ] ;
  tmp [ 1 ] = src [ 1 ] ;
  tmp [ 2 ] = src [ 2 ] ;
  tmp [ 3 ] =   SG_ONE  ;

  sgXformPnt4 ( tmp, tmp, mat ) ;
  sgScaleVec3 ( dst, tmp, SG_ONE / tmp [ 3 ] ) ;
}

void sgFullXformPnt4 ( sgVec4 dst, const sgVec4 src, const sgMat4 mat )
{
  sgXformPnt4 ( dst, src, mat ) ;
  sgScaleVec4 ( dst, SG_ONE / dst [ 3 ] ) ;
}

void sgHPRfromVec3 ( sgVec3 hpr, const sgVec3 src )
{
  sgVec3 tmp ;
  sgCopyVec3 ( tmp, src ) ;
  sgNormaliseVec3 ( tmp ) ;
  hpr[0] = - sgATan2 ( tmp [ 0 ], tmp [ 1 ] ) ;
  hpr[1] = - sgATan2 ( tmp [ 2 ], sgSqrt ( sgSquare ( tmp [ 0 ] ) +
                                           sgSquare ( tmp [ 1 ] ) ) ) ;
  hpr[2] = SG_ZERO ;
}



/*
  Quaternion routines are Copyright (C) 1999
  Kevin B. Thompson <kevinbthompson@yahoo.com>
  Modified by Sylvan W. Clebsch <sylvan@stanford.edu>
  Largely rewritten by "Negative0" <negative0@earthlink.net>
*/


void sgQuatToAngleAxis ( SGfloat *angle,
                         SGfloat *x, SGfloat *y, SGfloat *z,
                         const sgQuat src )
{
  sgVec3 axis ;

  sgQuatToAngleAxis ( angle, axis, src ) ;

  *x = axis [ 0 ] ;
  *y = axis [ 1 ] ;
  *z = axis [ 2 ] ;
}


void sgQuatToAngleAxis ( SGfloat *angle, sgVec3 axis, const sgQuat src )
{
  SGfloat a = (SGfloat) acos ( src[SG_W] ) ;
  SGfloat s = (SGfloat) sin  ( a ) ;
 
  *angle = a * SG_RADIANS_TO_DEGREES * SG_TWO ;

  if ( s == SG_ZERO )
    sgSetVec3 ( axis, SG_ZERO, SG_ZERO, SG_ONE );
  else
  {
    sgSetVec3   ( axis, src[SG_X], src[SG_Y], src[SG_Z] ) ;
    sgScaleVec3 ( axis, SG_ONE / s ) ;
  }
}


void sgAngleAxisToQuat ( sgQuat dst,
                         const SGfloat angle,
                         const SGfloat x, const SGfloat y, const SGfloat z )
{
  sgVec3 axis ; 
  sgSetVec3 ( axis, x, y, z ) ;
  sgAngleAxisToQuat ( dst, angle, axis ) ;
}


void sgAngleAxisToQuat ( sgQuat dst, const SGfloat angle, const sgVec3 axis )
{
  SGfloat temp_angle = angle * SG_DEGREES_TO_RADIANS / SG_TWO ;

  sgVec3 ax ;
  sgNormaliseVec3 ( ax, axis ) ;

  SGfloat s = - (SGfloat) sin ( temp_angle ) ;

  dst[SG_W] = (SGfloat) cos ( temp_angle ) ;
  sgScaleVec3 ( dst, ax, s ) ;
}


//from gamasutra.com
//by nb

void sgMatrixToQuat( sgQuat quat, const sgMat4 m )
{
  SGfloat tr, s, q[4] ;
  int   i, j, k ;

  int nxt[3] = {1, 2, 0};

  tr = m[0][0] + m[1][1] + m[2][2];

  // check the diagonal
  if (tr > SG_ZERO )
  {
    s = (SGfloat) sgSqrt (tr + SG_ONE);
    quat[SG_W] = s / SG_TWO;
    s = SG_HALF / s;
    quat[SG_X] = (m[1][2] - m[2][1]) * s;
    quat[SG_Y] = (m[2][0] - m[0][2]) * s;
    quat[SG_Z] = (m[0][1] - m[1][0]) * s;
  }
  else
  {		
    // diagonal is negative
   	i = 0;
    if (m[1][1] > m[0][0]) i = 1;
    if (m[2][2] > m[i][i]) i = 2;
    j = nxt[i];
    k = nxt[j];
    s = (SGfloat) sgSqrt ((m[i][i] - (m[j][j] + m[k][k])) + SG_ONE);
    q[i] = s * SG_HALF;
            
    if (s != SG_ZERO) s = SG_HALF / s;

    q[3] = (m[j][k] - m[k][j]) * s;
    q[j] = (m[i][j] + m[j][i]) * s;
    q[k] = (m[i][k] + m[k][i]) * s;

    quat[SG_X] = q[0];
    quat[SG_Y] = q[1];
    quat[SG_Z] = q[2];
    quat[SG_W] = q[3];
  }

  // seems to yield the inverse rotation, so:
  quat[SG_W] = - quat[SG_W];
}


void sgMultQuat ( sgQuat dst, const sgQuat a, const sgQuat b )
{
  /* [ ww' - v.v', vxv' + wv' + v'w ] */

  SGfloat t[8];

  t[0] = (a[SG_W] + a[SG_X]) * (b[SG_W] + b[SG_X]);
  t[1] = (a[SG_Z] - a[SG_Y]) * (b[SG_Y] - b[SG_Z]);
  t[2] = (a[SG_X] - a[SG_W]) * (b[SG_Y] + b[SG_Z]);
  t[3] = (a[SG_Y] + a[SG_Z]) * (b[SG_X] - b[SG_W]);
  t[4] = (a[SG_X] + a[SG_Z]) * (b[SG_X] + b[SG_Y]);
  t[5] = (a[SG_X] - a[SG_Z]) * (b[SG_X] - b[SG_Y]);
  t[6] = (a[SG_W] + a[SG_Y]) * (b[SG_W] - b[SG_Z]);
  t[7] = (a[SG_W] - a[SG_Y]) * (b[SG_W] + b[SG_Z]);

  dst[SG_W] =  t[1] + ((-t[4] - t[5] + t[6] + t[7]) * SG_HALF);
  dst[SG_X] =  t[0] - (( t[4] + t[5] + t[6] + t[7]) * SG_HALF);
  dst[SG_Y] = -t[2] + (( t[4] - t[5] + t[6] - t[7]) * SG_HALF);
  dst[SG_Z] = -t[3] + (( t[4] - t[5] - t[6] + t[7]) * SG_HALF);
}

//from gamasutra.com
//by nb@netcom.ca 

void sgMultQuat2 ( sgQuat dst, const sgQuat a, const sgQuat b )
{
  SGfloat A, B, C, D, E, F, G, H;

  A = (a[SG_W] + a[SG_X]) * (b[SG_W] + b[SG_X]) ;
  B = (a[SG_Z] - a[SG_Y]) * (b[SG_Y] - b[SG_Z]) ;
  C = (a[SG_X] - a[SG_W]) * (b[SG_Y] + b[SG_Z]) ;
  D = (a[SG_Y] + a[SG_Z]) * (b[SG_X] - b[SG_W]) ;
  E = (a[SG_X] + a[SG_Z]) * (b[SG_X] + b[SG_Y]) ;
  F = (a[SG_X] - a[SG_Z]) * (b[SG_X] - b[SG_Y]) ;
  G = (a[SG_W] + a[SG_Y]) * (b[SG_W] - b[SG_Z]) ;
  H = (a[SG_W] - a[SG_Y]) * (b[SG_W] + b[SG_Z]) ;


  dst[SG_W] =  B + (-E - F + G + H) / SG_TWO ;
  dst[SG_X] =  A - ( E + F + G + H) / SG_TWO ; 
  dst[SG_Y] = -C + ( E - F + G - H) / SG_TWO ;
  dst[SG_Z] = -D + ( E - F - G + H) / SG_TWO ;
}

//from gamasutra.com
//by nb@netcom.ca 

void sgEulerToQuat(sgQuat quat, const sgVec3 hpr )
{
  SGfloat cr, cp, cy, sr, sp, sy, cpcy, spsy;

// calculate trig identities
  cr = (SGfloat) cos(hpr[2]*SG_DEGREES_TO_RADIANS/SG_TWO);
  cp = (SGfloat) cos(hpr[1]*SG_DEGREES_TO_RADIANS/SG_TWO);
  cy = (SGfloat) cos(hpr[0]*SG_DEGREES_TO_RADIANS/SG_TWO);

  sr = (SGfloat) sin(hpr[2]*SG_DEGREES_TO_RADIANS/SG_TWO);
  sp = (SGfloat) sin(hpr[1]*SG_DEGREES_TO_RADIANS/SG_TWO);
  sy = (SGfloat) sin(hpr[0]*SG_DEGREES_TO_RADIANS/SG_TWO);
  
  cpcy = cp * cy;
  spsy = sp * sy;

  quat[SG_W] = cr * cpcy + sr * spsy;
  quat[SG_X] = sr * cpcy - cr * spsy;
  quat[SG_Y] = cr * sp * cy + sr * cp * sy;
  quat[SG_Z] = cr * cp * sy - sr * sp * cy;
}

//from darwin3d.com
// jeffl@darwin3d.com

void sgQuatToEuler( sgVec3 hpr, const sgQuat quat )
{
  SGfloat matrix[3][3];
  SGfloat cx,sx;
  SGfloat cy,sy;
  SGfloat cz,sz;

  // CONVERT QUATERNION TO MATRIX - I DON'T REALLY NEED ALL OF IT

  matrix[0][0] = SG_ONE - (SG_TWO * quat[SG_Y] * quat[SG_Y])
                        - (SG_TWO * quat[SG_Z] * quat[SG_Z]);
//matrix[0][1] = (SG_TWO * quat->x * quat->y) - (SG_TWO * quat->w * quat->z);
//matrix[0][2] = (SG_TWO * quat->x * quat->z) + (SG_TWO * quat->w * quat->y);

  matrix[1][0] = (SG_TWO * quat[SG_X] * quat[SG_Y]) +
                          (SG_TWO * quat[SG_W] * quat[SG_Z]);
//matrix[1][1] = SG_ONE - (SG_TWO * quat->x * quat->x)
//                      - (SG_TWO * quat->z * quat->z);
//matrix[1][2] = (SG_TWO * quat->y * quat->z) - (SG_TWO * quat->w * quat->x);

  matrix[2][0] = (SG_TWO * quat[SG_X] * quat[SG_Z]) -
                 (SG_TWO * quat[SG_W] * quat[SG_Y]);
  matrix[2][1] = (SG_TWO * quat[SG_Y] * quat[SG_Z]) +
                 (SG_TWO * quat[SG_W] * quat[SG_X]);
  matrix[2][2] = SG_ONE - (SG_TWO * quat[SG_X] * quat[SG_X])
                        - (SG_TWO * quat[SG_Y] * quat[SG_Y]);

  sy = -matrix[2][0];
  cy = (SGfloat)sgSqrt(SG_ONE - (sy * sy));

  hpr[1] = sgATan2 ( sy, cy ) ;

  // AVOID DIVIDE BY ZERO ERROR ONLY WHERE Y= +-90 or +-270 
  // NOT CHECKING cy BECAUSE OF PRECISION ERRORS
  if (sy != SG_ONE && sy != -SG_ONE)	
  {
    cx = matrix[2][2] / cy;
    sx = matrix[2][1] / cy;
    hpr[0] = sgATan2 ( sx, cx ) ;

    cz = matrix[0][0] / cy;
    sz = matrix[1][0] / cy;
    hpr[2] = sgATan2 ( sz, cz ) ;
  }
  else
  {
    // SINCE Cos(Y) IS 0, I AM SCREWED.  ADOPT THE STANDARD Z = 0
    // I THINK THERE IS A WAY TO FIX THIS BUT I AM NOT SURE.  EULERS SUCK
    // NEED SOME MORE OF THE MATRIX TERMS NOW

    matrix[1][1] = SG_ONE - (SG_TWO * quat[SG_X] * quat[SG_X])
                          - (SG_TWO * quat[SG_Z] * quat[SG_Z]);
    matrix[1][2] = (SG_TWO * quat[SG_Y] * quat[SG_Z]) -
                   (SG_TWO * quat[SG_W] * quat[SG_X]);

    cx =  matrix[1][1];
    sx = -matrix[1][2];
    hpr[0] = sgATan2 ( sx, cx ) ;

    cz = SG_ONE ;
    sz = SG_ZERO ;
    hpr[2] = sgATan2 ( sz, cz ) ;
  }
}


void sgQuatToMatrix ( sgMat4 dst, const sgQuat q )
{
  SGfloat two_xx = q[SG_X] * (q[SG_X] + q[SG_X]) ;
  SGfloat two_xy = q[SG_X] * (q[SG_Y] + q[SG_Y]) ;
  SGfloat two_xz = q[SG_X] * (q[SG_Z] + q[SG_Z]) ;

  SGfloat two_wx = q[SG_W] * (q[SG_X] + q[SG_X]) ;
  SGfloat two_wy = q[SG_W] * (q[SG_Y] + q[SG_Y]) ;
  SGfloat two_wz = q[SG_W] * (q[SG_Z] + q[SG_Z]) ;

  SGfloat two_yy = q[SG_Y] * (q[SG_Y] + q[SG_Y]) ;
  SGfloat two_yz = q[SG_Y] * (q[SG_Z] + q[SG_Z]) ;

  SGfloat two_zz = q[SG_Z] * (q[SG_Z] + q[SG_Z]) ;

  sgSetVec4 ( dst[0], SG_ONE-(two_yy+two_zz), two_xy-two_wz, two_xz+two_wy, SG_ZERO ) ;
  sgSetVec4 ( dst[1], two_xy+two_wz, SG_ONE-(two_xx+two_zz), two_yz-two_wx, SG_ZERO ) ;
  sgSetVec4 ( dst[2], two_xz-two_wy, two_yz+two_wx, SG_ONE-(two_xx+two_yy), SG_ZERO ) ;
  sgSetVec4 ( dst[3], SG_ZERO, SG_ZERO, SG_ZERO, SG_ONE ) ;
}


//from gamasutra.com
//by nb@netcom.ca 

/*****************************
 DEPRECATED...use sgQuatToMatrix instead.
*****************************/

void sgMakeRotMat42( sgMat4 m, sgQuat quat ){
 SGfloat wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

  // calculate coefficients
  x2 = quat[SG_X] + quat[SG_X]; y2 = quat[SG_Y] + quat[SG_Y]; 
  z2 = quat[SG_Z] + quat[SG_Z];
  xx = quat[SG_X] * x2;   xy = quat[SG_X] * y2;   xz = quat[SG_X] * z2;
  yy = quat[SG_Y] * y2;   yz = quat[SG_Y] * z2;   zz = quat[SG_Z] * z2;
  wx = quat[SG_W] * x2;   wy = quat[SG_W] * y2;   wz = quat[SG_W] * z2;

  m[0][0] = SG_ONE- (yy + zz); 	m[0][1] = xy - wz;
  m[0][2] = xz + wy;		m[0][3] = SG_ZERO ;
 
  m[1][0] = xy + wz;		m[1][1] = SG_ONE- (xx + zz);
  m[1][2] = yz - wx;		m[1][3] = SG_ZERO ;

  m[2][0] = xz - wy;		m[2][1] = yz + wx;
  m[2][2] = SG_ONE- (xx + yy);		m[2][3] = SG_ZERO ;

  m[3][0] = 0;			m[3][1] = 0;
  m[3][2] = 0;			m[3][3] = 1;
}

//from gamasutra.com
//by nb@netcom.ca 

void sgSlerpQuat2( sgQuat dst, const sgQuat from, const sgQuat to, const SGfloat t )
{
  SGfloat to1[4];
  SGfloat omega, cosom, sinom, scale0, scale1;

  // calc cosine
  cosom = from[SG_X] * to[SG_X] +
          from[SG_Y] * to[SG_Y] +
          from[SG_Z] * to[SG_Z] +
          from[SG_W] * to[SG_W];

  // adjust signs (if necessary)

  if ( cosom < SG_ZERO )
  { 
    cosom = -cosom; 
    to1[0] = - to[SG_X];
    to1[1] = - to[SG_Y];
    to1[2] = - to[SG_Z];
    to1[3] = - to[SG_W];
  }
  else
  {
    to1[0] = to[SG_X];
    to1[1] = to[SG_Y];
    to1[2] = to[SG_Z];
    to1[3] = to[SG_W];
  }

  // calculate coefficients
#define DELTA SG_ZERO 
  if ( (SG_ONE- cosom) > DELTA )
  {
    // standard case (slerp)
    omega = (SGfloat) acos(cosom);
    sinom = (SGfloat) sin(omega);
    scale0 = (SGfloat) sin((SG_ONE- t) * omega) / sinom;
    scale1 = (SGfloat) sin(t * omega) / sinom;
  }
  else
  {        
    // "from" and "to" quaternions are very close 
    //  ... so we can do a linear interpolation
    scale0 = SG_ONE- t;
    scale1 = t;
  }

  // calculate final values
  dst[SG_X] = scale0 * from[SG_X] + scale1 * to1[0];
  dst[SG_Y] = scale0 * from[SG_Y] + scale1 * to1[1];
  dst[SG_Z] = scale0 * from[SG_Z] + scale1 * to1[2];
  dst[SG_W] = scale0 * from[SG_W] + scale1 * to1[3];
}

void sgSlerpQuat( sgQuat dst, const sgQuat from, const sgQuat to, const SGfloat t )
{
  SGfloat co, scale0, scale1;
  bool flip = false ;

  /* SWC - Interpolate between to quaternions */

  co = sgScalarProductVec4 ( from, to ) ;

  if ( co < SG_ZERO )
  {
    co = -co;
    flip = true ;
  }

  if ( co < SG_ONE - (SGfloat) 1e-6 )
  {
    SGfloat o = (SGfloat) acos ( co );
    SGfloat so = SG_ONE / (SGfloat) sin ( o );

    scale0 = (SGfloat) sin ( (SG_ONE - t) * o ) * so;
    scale1 = (SGfloat) sin ( t * o ) * so;
  }
  else
  {
    scale0 = SG_ONE - t;
    scale1 = t;
  }

  if ( flip )
  {
    scale1 = -scale1 ;
  }

  dst[SG_X] = scale0 * from[SG_X] + scale1 * to[SG_X] ;
  dst[SG_Y] = scale0 * from[SG_Y] + scale1 * to[SG_Y] ;
  dst[SG_Z] = scale0 * from[SG_Z] + scale1 * to[SG_Z] ;
  dst[SG_W] = scale0 * from[SG_W] + scale1 * to[SG_W] ;
}


void sgReflectInPlaneVec3 ( sgVec3 dst, const sgVec3 src, const sgVec4 plane )
{
  SGfloat src_dot_norm  = sgScalarProductVec3 ( src, plane ) ;

  sgVec3 tmp ;

  sgScaleVec3 ( tmp, plane, SG_TWO * src_dot_norm ) ;
  sgSubVec3 ( dst, src, tmp ) ;
}

                                                                                

int sgClassifyMat4 ( const sgMat4 m )
{
  const SGfloat epsilon = 1e-6f ;

  int flags = 0 ;


  SGfloat sx, sy, sz ;

  if ( m[0][1] == SG_ZERO && m[0][2] == SG_ZERO &&
       m[1][0] == SG_ZERO && m[1][2] == SG_ZERO &&
       m[2][0] == SG_ZERO && m[2][1] == SG_ZERO )
  {

    int n = ( m[0][0] < 0 ) + ( m[1][1] < 0 ) + ( m[2][2] < 0 ) ;

    if ( n > 1 )
      flags |= SG_ROTATION ;

    if ( n % 2 != 0 )
      flags |= SG_MIRROR ;

    sx = m[0][0] * m[0][0] ;
    sy = m[1][1] * m[1][1] ;
    sz = m[2][2] * m[2][2] ;

  }
  else
  {

    flags |= SG_ROTATION ;

    if ( sgAbs ( sgScalarProductVec3 ( m[1], m[2] ) ) > epsilon ||
         sgAbs ( sgScalarProductVec3 ( m[2], m[0] ) ) > epsilon ||
         sgAbs ( sgScalarProductVec3 ( m[0], m[1] ) ) > epsilon )
    {
      flags |= SG_NONORTHO ;
    }

    sgVec3 temp ;
    sgVectorProductVec3 ( temp, m[0], m[1] ) ;
    SGfloat det = sgScalarProductVec3 ( temp, m[2] ) ;

    if ( det < 0 )
      flags |= SG_MIRROR ;

    sx = sgScalarProductVec3 ( m[0], m[0] ) ;
    sy = sgScalarProductVec3 ( m[1], m[1] ) ;
    sz = sgScalarProductVec3 ( m[2], m[2] ) ;

  }


  if ( sgAbs ( sx - sy ) > epsilon ||
       sgAbs ( sx - sz ) > epsilon )
  {
    flags |= SG_NONORTHO ;
    flags |= SG_GENERAL_SCALE ; // also set general scale bit, though it may be deleted in the future
  }
  else
  {
    if ( sgAbs ( sx - SG_ONE ) > epsilon )
      flags |= SG_SCALE ;
  }


  if ( m[3][0] != SG_ZERO || m[3][1] != SG_ZERO || m[3][2] != SG_ZERO )
  {
    flags |= SG_TRANSLATION ;
  }


  if ( m[0][3] != SG_ZERO || m[1][3] != SG_ZERO || m[2][3] != SG_ZERO ||
       m[3][3] != SG_ONE )
  {
    flags |= SG_PROJECTION ;
  }


  return flags ;
}

 
SGfloat sgTriangleSolver_ASAtoArea ( SGfloat angA, SGfloat lenB, SGfloat angC )
{
  /* Get the third angle */

  SGfloat angB = SG_180 - (angA + angC) ;

  /* Use Sine Rule to get length of a second side - then use SAStoArea. */

  SGfloat sinB = sgSin ( angB ) ;

  if ( sinB == SG_ZERO )
    return SG_ZERO ;

  SGfloat lenA = lenB * sgSin(angA) / sinB ;

  return sgTriangleSolver_SAStoArea ( lenA, angC, lenB ) ;
}


SGfloat sgTriangleSolver_SAStoArea ( SGfloat lenA, SGfloat angB, SGfloat lenC )
{
  return SG_HALF * lenC * lenA * sgSin ( angB ) ;
}


SGfloat sgTriangleSolver_SSStoArea ( SGfloat lenA, SGfloat lenB, SGfloat lenC )
{
  /* Heron's formula */

  SGfloat s = ( lenA + lenB + lenC ) / SG_TWO ;
  SGfloat q = s * (s-lenA) * (s-lenB) * (s-lenC) ;

  /* Ikky illegal triangles generate zero areas. */

  return ( q <= SG_ZERO ) ? SG_ZERO : sgSqrt ( q ) ;
}


SGfloat sgTriangleSolver_ASStoArea ( SGfloat angB, SGfloat lenA, SGfloat lenB,
                                     int angA_is_obtuse )
{
  SGfloat lenC ;

  sgTriangleSolver_ASStoSAA ( angB, lenA, lenB, angA_is_obtuse,
                                                         &lenC, NULL, NULL ) ;

  return sgTriangleSolver_SAStoArea ( lenA, angB, lenC ) ;
}

SGfloat sgTriangleSolver_SAAtoArea ( SGfloat lenA, SGfloat angB, SGfloat angA )
{
  SGfloat lenC ;

  sgTriangleSolver_SAAtoASS ( lenA, angB, angA, NULL, NULL, &lenC ) ;

  return sgTriangleSolver_SAStoArea ( lenA, angB, lenC ) ;
}

void sgTriangleSolver_SSStoAAA ( SGfloat  lenA, SGfloat  lenB, SGfloat  lenC,
                                 SGfloat *angA, SGfloat *angB, SGfloat *angC )
{
  SGfloat aa, bb, cc ;

  int flag =  ( lenA == SG_ZERO )     |
             (( lenB == SG_ZERO )<<1) |
             (( lenC == SG_ZERO )<<2) ;

  /* Ikky zero-sized triangles generate zero/90 angles appropriately. */
  /* Ikky triangles with all lengths zero generate 60 degree angles. */
  /* Ikky impossible triangles generate all zero angles. */

  switch ( flag )
  {
    case 0 :  /* no zero-lengthed sides */
     /* Cosine law */
     aa = sgACos (( lenB*lenB + lenC*lenC - lenA*lenA )/(SG_TWO*lenB*lenC)) ;
     bb = sgACos (( lenA*lenA + lenC*lenC - lenB*lenB )/(SG_TWO*lenA*lenC)) ;
     cc = sgACos (( lenA*lenA + lenB*lenB - lenC*lenC )/(SG_TWO*lenA*lenB)) ;
     break ;

    case 1 :  /* lenA is zero */
     aa = SG_ZERO ;
     bb = cc = SG_90 ;
     break ;

    case 2 : /* lenB is zero */
     bb = SG_ZERO ;
     aa = cc = SG_90 ;
     break ;

    case 4 : /* lenC is zero */
      cc = SG_ZERO ;
      aa = bb = SG_90 ;
      break ;

    case 3 : /* Two lengths are zero and the third isn't?!? */
    case 5 :
    case 6 :
      aa = bb = cc = SG_ZERO ;
      break ;

    default : /* All three sides are zero length */
      aa = bb = cc = SG_60 ;
      break ;
  }

  if ( angA ) *angA = aa ;
  if ( angB ) *angB = bb ;
  if ( angC ) *angC = cc ;
}

void sgTriangleSolver_SAStoASA ( SGfloat  lenA, SGfloat  angB, SGfloat  lenC,
                                 SGfloat *angA, SGfloat *lenB, SGfloat *angC )
{
  /* Get third side using Cosine Rule */

  SGfloat s = lenC * lenC +
              lenA * lenA - SG_TWO * lenC * lenA * sgCos( angB ) ;

  SGfloat lb = ( s <= SG_ZERO ) ? SG_ZERO : (SGfloat) sqrt ( s ) ;

  if ( lenB ) *lenB = lb ;

  sgTriangleSolver_SSStoAAA ( lenA, lb, lenC, angA, NULL, angC ) ;
}


void sgTriangleSolver_ASAtoSAS ( SGfloat  angA, SGfloat  lenB, SGfloat  angC,
                                 SGfloat *lenA, SGfloat *angB, SGfloat *lenC )
{
  /* Find the missing angle */

  SGfloat bb = SG_180 - (angA + angC) ;

  if ( angB ) *angB = bb ;

  /* Use Sine Rule */

  SGfloat sinB = sgSin ( bb ) ;

  if ( sinB == SG_ZERO )
  {
    if ( lenA ) *lenA = lenB / SG_TWO ;  /* One valid interpretation */
    if ( lenC ) *lenC = lenB / SG_TWO ;
  }
  else
  {
    if ( lenA ) *lenA = lenB * sgSin(angA) / sinB ;
    if ( lenC ) *lenC = lenB * sgSin(angC) / sinB ;
  }
}


void sgTriangleSolver_ASStoSAA ( SGfloat angB, SGfloat lenA, SGfloat lenB,
                                 int angA_is_obtuse,
                                 SGfloat *lenC, SGfloat *angA, SGfloat *angC )
{
  /* Sine law */

  SGfloat aa = (lenB == SG_ZERO ) ? SG_ZERO : sgASin (lenA * sgSin(angB)/lenB) ;

  if ( angA_is_obtuse )
    aa = SG_180 - aa ;

  if ( angA ) *angA = aa ;

  /* Find the missing angle */

  SGfloat cc = SG_180 - (aa + angB) ;

  if ( angC ) *angC = cc ;

  /* Use SAStoASA to get the last length */

  sgTriangleSolver_SAStoASA ( lenA, cc, lenB, NULL, lenC, NULL ) ;
}


void sgTriangleSolver_SAAtoASS ( SGfloat  lenA, SGfloat  angB, SGfloat  angA,
                                 SGfloat *angC, SGfloat *lenB, SGfloat *lenC )
{
  /* Find the missing angle */

  SGfloat cc = SG_180 - (angB + angA) ;

  if ( angC ) *angC = cc ;

  sgTriangleSolver_ASAtoSAS ( cc, lenA, angB, lenC, NULL, lenB ) ;
}


