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

/*
  Determine the origin and unit direction vector of a line
  formed by the intersection of two planes.
  Returned point will lie on axis that was used to
  determine intersection.
         Norman Vine - nhv@yahoo.com
*/

int sgdIsectPlanePlane ( sgdVec3 point, sgdVec3 dir,
                        sgdVec4 plane1, sgdVec4 plane2 )
{
  /* Determine intersection line direction. */

  sgdVectorProductVec3 ( dir, plane1, plane2 ) ;

  SGDfloat dnorm = sgdLengthVec3 ( dir ) ;

  /* If planes are parallel then fail */

  if ( dnorm < DBL_EPSILON )
  {
    sgdZeroVec3 ( point ) ;
    sgdZeroVec3 ( dir   ) ;
    return FALSE ;
  }

  /* Determine intersection point with the best suited coordinate plane. */

  SGDfloat abs ;
  SGDfloat maxabs = sgdAbs(dir[0]);
  int index = 0;

  if ((abs = sgdAbs(dir[1])) > maxabs) { maxabs = abs ; index = 1; }
  if ((abs = sgdAbs(dir[2])) > maxabs) { maxabs = abs ; index = 2; }

  switch ( index )
  {
    case 0:
      sgdSetVec3( point,
			SGD_ZERO,
			(plane1[2] * plane2[3] - plane2[2] * plane1[3]) / dir[0],
			(plane2[1] * plane1[3] - plane1[1] * plane2[3]) / dir[0] );
      break;
    case 1:
      sgdSetVec3( point,
			(plane2[2] * plane1[3] - plane1[2] * plane2[3]) / dir[1],
		     SGD_ZERO,
            (plane1[0] * plane2[3] - plane2[0] * plane1[3]) / dir[1] );
      break;
    case 2:
      sgdSetVec3( point, 
            (plane1[1] * plane2[3] - plane2[1] * plane1[3]) / dir[2],
            (plane2[0] * plane1[3] - plane1[0] * plane2[3]) / dir[2],
             SGD_ZERO );
      break;
    default: return FALSE ;  /* Impossible */
  }

  /* Normalize the direction */

  sgdScaleVec3( dir, SGD_ONE / dnorm );
  return TRUE;
}



/*
   Find the intersection of an infinite line with a plane
   (the line being defined by a point and direction).

   Norman Vine -- nhv@yahoo.com  (with hacks by Steve)
*/

int sgdIsectInfLinePlane( sgdVec3 dst, sgdVec3 l_org, sgdVec3 l_vec, sgdVec4 plane )
{
  SGDfloat tmp = sgdScalarProductVec3 ( l_vec, plane ) ;

  /* Is line parallel to plane? */

  if ( sgdAbs ( tmp ) < DBL_EPSILON )
    return FALSE ;
 
  sgdScaleVec3 ( dst, l_vec, -( sgdScalarProductVec3 ( l_org, plane )
                                                + plane[3] ) / tmp ) ;
  sgdAddVec3  ( dst, l_org ) ;

  return TRUE ;
}



/*
  Given the origin and direction vector for two lines
  find their intersection - or the point closest to
  both lines if they don't intersect.

  Norman Vine -- nhv@yahoo.com
*/

int sgdIsectInfLineInfLine( sgdVec3 dst,
           sgdVec3 l1_org, sgdVec3 l1_vec,
           sgdVec3 l2_org, sgdVec3 l2_vec )
{
  sgdVec3 vec_l1, vec_l2 ;

  sgdNormalizeVec3 ( vec_l1, l1_vec ) ;
  sgdNormalizeVec3 ( vec_l2, l2_vec ) ;

  /* Connecting line 'C' is perpendicular to both */

  sgdVec3 perp ;
  sgdVectorProductVec3 ( perp, vec_l1, vec_l2 ) ;

  /* check for near-parallel lines */

  SGDfloat dist = sgdScalarProductVec3 ( perp, perp ) ;

  if ( dist < DBL_EPSILON )
  {
    /* degenerate: lines parallel - any point will do. */

    sgdCopyVec3 ( dst, l2_org ) ;
    return TRUE ;
  }
  
  /*
    Form a plane containing the line A and C,
    and another containing B and C
  */

  sgdScaleVec3 ( perp, SGD_ONE / sgdSqrt( dist ) ) ;
 
  sgdVec4 pa ;
  sgdVec4 pb ;
  sgdVec3 tmp ;

  sgdVectorProductVec3 ( tmp, perp, vec_l1 ) ;
  sgdNormalizeVec3     ( tmp );
  sgdMakePlane         ( pa, tmp, l1_org ) ;

  sgdVectorProductVec3 ( tmp, perp, vec_l2 ) ;
  sgdNormalizeVec3     ( tmp );
  sgdMakePlane         ( pb, tmp, l2_org ) ;

  sgdVec3 tmp_org, tmp_vec ;

  if ( ! sgdIsectPlanePlane( tmp_org, tmp_vec, pa, pb ) )
  {
    /* This *shouldn't* ever happen because we already
      tested for parallel lines - but with roundoff
      errors, it *might* in borderline cases so...
    */

    sgdCopyVec3 ( dst, l2_org ) ;
    return FALSE ;
  }
 
  if ( ! sgdIsectInfLinePlane ( dst, l2_org, vec_l2, pa ) )
  {
    sgdCopyVec3 ( dst, l2_org ) ;
    return FALSE ;
  }

  return TRUE ;
}


/*
  Intersect the line segment from v1->v2 with the 'plane'.
  'dst' is the intersection point and the return
  result is in the range 0..1 if the intersection lies
  between v1 and v2, >1 if beyond v2 and <0 if before v1.
  FLT_MAX is returned if the vector does not intersect
  the plane.

  Steve Baker
*/

SGDfloat sgdIsectLinesegPlane ( sgdVec3 dst, sgdVec3 v1, sgdVec3 v2, sgdVec4 plane )
{
  sgdVec3 delta ;

  sgdSubVec3 ( delta, v2, v1 ) ;

  SGDfloat p = sgdScalarProductVec3 ( plane, delta ) ;

  if ( p == SGD_ZERO )
  {
    dst [ 0 ] = dst [ 1 ] = dst [ 2 ] = FLT_MAX ;
    return FLT_MAX ;
  }

  float s = (float) (- ( sgdScalarProductVec3 ( plane, v1 ) + plane[3] ) / p) ;

  sgdScaleVec3 ( dst, delta, s ) ;
  sgdAddVec3   ( dst, dst, v1 ) ;

  return s ;
}



// return the sign of a value
static inline const int SGD_SIGN(const SGDfloat x) {
	return x < 0 ? -1 : 1;
}

// return the minimum of two values
static inline SGDfloat SGD_MIN2(const SGDfloat a, const SGDfloat b) {
	return a < b ? a : b;
}

// return the minimum of three values
static inline SGDfloat SGD_MIN3( const SGDfloat a, const SGDfloat b, const SGDfloat c) {
	return (a < b ? SGD_MIN2 (a, c) : SGD_MIN2 (b, c));
}

// return the minimum and maximum of three values
static void SGD_MIN_MAX3 ( SGDfloat &min, SGDfloat &max,
						   const SGDfloat &a, const SGDfloat &b,
						   const SGDfloat &c)
{
	if( a > b ) {
		if( a > c ) {
			max = a;
			min = SGD_MIN2( b,  c );
		} else {
			max = c;
			min = SGD_MIN2( a, b );
		}
	} else {
		if( b > c ) {
			max = b;
			min = SGD_MIN2( a, c );
		} else {
			max = c;
			min = SGD_MIN2( a, b );
		}
	}
}

/*
 * Given a point and a triangle lying on the same plane
 * check to see if the point is inside the triangle
 */
bool sgdPointInTriangle( sgdVec3 point, sgdVec3 tri[3] )
{
	sgdVec3 dif;

	int i;
	for( i=0; i<3; i++ ) {
		SGDfloat min, max;
		SGD_MIN_MAX3 ( min, max, tri[0][i], tri[1][i], tri[2][i] );
		// punt if outside bouding cube
		if( (point[i] < min) || (point[i] > max) )
			return false;
		dif[i] = max - min;
	}

	// drop the smallest dimension so we only have to work in 2d.
	SGDfloat min_dim = SGD_MIN3 (dif[0], dif[1], dif[2]);
	SGDfloat x1, y1, x2, y2, x3, y3, rx, ry;
	if ( fabs(min_dim-dif[0]) <= DBL_EPSILON ) {
		// x is the smallest dimension
		x1 = point[1];
		y1 = point[2];
		x2 = tri[0][1];
		y2 = tri[0][2];
		x3 = tri[1][1];
		y3 = tri[1][2];
		rx = tri[2][1];
		ry = tri[2][2];
	} else if ( fabs(min_dim-dif[1]) <= DBL_EPSILON ) {
		// y is the smallest dimension
		x1 = point[0];
		y1 = point[2];
		x2 = tri[0][0];
		y2 = tri[0][2];
		x3 = tri[1][0];
		y3 = tri[1][2];
		rx = tri[2][0];
		ry = tri[2][2];
	} else if ( fabs(min_dim-dif[2]) <= DBL_EPSILON ) {
		// z is the smallest dimension
		x1 = point[0];
		y1 = point[1];
		x2 = tri[0][0];
		y2 = tri[0][1];
		x3 = tri[1][0];
		y3 = tri[1][1];
		rx = tri[2][0];
		ry = tri[2][1];
	} else {
		// all dimensions are really small so lets call it close
		// enough and return a successful match
		return true;
	}

	// check if intersection point is on the same side of p1 <-> p2 as p3  
	SGDfloat tmp = (y2 - y3) / (x2 - x3);
	int side1 = SGD_SIGN (tmp * (rx - x3) + y3 - ry);
	int side2 = SGD_SIGN (tmp * (x1 - x3) + y3 - y1);
	if ( side1 != side2 ) {
		// printf("failed side 1 check\n");
		return false;
	}

	// check if intersection point is on correct side of p2 <-> p3 as p1
	tmp = (y3 - ry) / (x3 - rx);
	side1 = SGD_SIGN (tmp * (x2 - rx) + ry - y2);
	side2 = SGD_SIGN (tmp * (x1 - rx) + ry - y1);
	if ( side1 != side2 ) {
		// printf("failed side 2 check\n");
		return false;
	}

	// check if intersection point is on correct side of p1 <-> p3 as p2
	tmp = (y2 - ry) / (x2 - rx);
	side1 = SGD_SIGN (tmp * (x3 - rx) + ry - y3);
	side2 = SGD_SIGN (tmp * (x1 - rx) + ry - y1);
	if ( side1 != side2 ) {
		// printf("failed side 3  check\n");
		return false;
	}

	return true;
}

