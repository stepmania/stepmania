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

int sgIsectPlanePlane ( sgVec3 point, sgVec3 dir,
                        sgVec4 plane1, sgVec4 plane2 )
{
  /* Determine intersection line direction. */

  sgVectorProductVec3 ( dir, plane1, plane2 ) ;

  SGfloat dnorm = sgLengthVec3 ( dir ) ;

  /* If planes are parallel then fail */

  if ( dnorm < FLT_EPSILON )
  {
    sgZeroVec3 ( point ) ;
    sgZeroVec3 ( dir   ) ;
    return FALSE ;
  }

  /* Determine intersection point with the best suited coordinate plane. */

  SGfloat abs ;
  SGfloat maxabs = sgAbs(dir[0]);
  int index = 0;

  if ((abs = sgAbs(dir[1])) > maxabs) { maxabs = abs ; index = 1; }
  if ((abs = sgAbs(dir[2])) > maxabs) { maxabs = abs ; index = 2; }

  switch ( index )
  {
    case 0:
      sgSetVec3( point,
             SG_ZERO,
             (plane1[2] * plane2[3] - plane2[2] * plane1[3]) / dir[0],
             (plane2[1] * plane1[3] - plane1[1] * plane2[3]) / dir[0] );
      break;
    case 1:
      sgSetVec3( point,
             (plane2[2] * plane1[3] - plane1[2] * plane2[3]) / dir[1],
             SG_ZERO,
             (plane1[0] * plane2[3] - plane2[0] * plane1[3]) / dir[1] );
      break;
    case 2:
      sgSetVec3( point, 
             (plane1[1] * plane2[3] - plane2[1] * plane1[3]) / dir[2],
             (plane2[0] * plane1[3] - plane1[0] * plane2[3]) / dir[2],
             SG_ZERO );
      break;
    default: return FALSE ;  /* Impossible */
  }

  /* Normalize the direction */

  sgScaleVec3( dir, SG_ONE / dnorm );
  return TRUE;
}



/*
   Find the intersection of an infinite line with a plane
   (the line being defined by a point and direction).

   Norman Vine -- nhv@yahoo.com  (with hacks by Steve)
*/

int sgIsectInfLinePlane( sgVec3 dst, sgVec3 l_org, sgVec3 l_vec, sgVec4 plane )
{
  SGfloat tmp = sgScalarProductVec3 ( l_vec, plane ) ;

  /* Is line parallel to plane? */

  if ( sgAbs ( tmp ) < FLT_EPSILON )
    return FALSE ;
 
  sgScaleVec3 ( dst, l_vec, -( sgScalarProductVec3 ( l_org, plane )
                                                + plane[3] ) / tmp ) ;
  sgAddVec3  ( dst, l_org ) ;

  return TRUE ;
}



/*
  Given the origin and direction vector for two lines
  find their intersection - or the point closest to
  both lines if they don't intersect.

  Norman Vine -- nhv@yahoo.com
*/

int sgIsectInfLineInfLine( sgVec3 dst,
           sgVec3 l1_org, sgVec3 l1_vec,
           sgVec3 l2_org, sgVec3 l2_vec )
{
  sgVec3 vec_l1, vec_l2 ;

  sgNormalizeVec3 ( vec_l1, l1_vec ) ;
  sgNormalizeVec3 ( vec_l2, l2_vec ) ;

  /* Connecting line 'C' is perpendicular to both */

  sgVec3 perp ;
  sgVectorProductVec3 ( perp, vec_l1, vec_l2 ) ;

  /* check for near-parallel lines */

  SGfloat dist = sgScalarProductVec3 ( perp, perp ) ;

  if ( dist < FLT_EPSILON )
  {
    /* degenerate: lines parallel - any point will do. */

    sgCopyVec3 ( dst, l2_org ) ;
    return TRUE ;
  }
  
  /*
    Form a plane containing the line A and C,
    and another containing B and C
  */

  sgScaleVec3 ( perp, SG_ONE / sgSqrt( dist ) ) ;
 
  sgVec4 pa ;
  sgVec4 pb ;
  sgVec3 tmp ;

  sgVectorProductVec3 ( tmp, perp, vec_l1 ) ;
  sgNormalizeVec3     ( tmp );
  sgMakePlane         ( pa, tmp, l1_org ) ;

  sgVectorProductVec3 ( tmp, perp, vec_l2 ) ;
  sgNormalizeVec3     ( tmp );
  sgMakePlane         ( pb, tmp, l2_org ) ;

  sgVec3 tmp_org, tmp_vec ;

  if ( ! sgIsectPlanePlane( tmp_org, tmp_vec, pa, pb ) )
  {
    /* This *shouldn't* ever happen because we already
      tested for parallel lines - but with roundoff
      errors, it *might* in borderline cases so...
    */

    sgCopyVec3 ( dst, l2_org ) ;
    return FALSE ;
  }
 
  if ( ! sgIsectInfLinePlane ( dst, l2_org, vec_l2, pa ) )
  {
    sgCopyVec3 ( dst, l2_org ) ;
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

SGfloat sgIsectLinesegPlane ( sgVec3 dst, sgVec3 v1, sgVec3 v2, sgVec4 plane )
{
  sgVec3 delta ;

  sgSubVec3 ( delta, v2, v1 ) ;

  SGfloat p = sgScalarProductVec3 ( plane, delta ) ;

  if ( p == SG_ZERO )
  {
    dst [ 0 ] = dst [ 1 ] = dst [ 2 ] = FLT_MAX ;
    return FLT_MAX ;
  }

  float s = - ( sgScalarProductVec3 ( plane, v1 ) + plane[3] ) / p ;

  sgScaleVec3 ( dst, delta, s ) ;
  sgAddVec3   ( dst, dst, v1 ) ;

  return s ;
}


// return the sign of a value
static inline const int SG_SIGN( const SGfloat x ) {
	return x < 0 ? -1 : 1;
}

// return the minimum of two values
static inline SGfloat SG_MIN2( const SGfloat a, const SGfloat b ) {
	return a < b ? a : b;
}

// return the minimum of three values
static inline SGfloat SG_MIN3( const SGfloat a, const SGfloat b, const SGfloat c) {
	return ( a < b ? SG_MIN2(a, c) : SG_MIN2(b, c) );
}

// return the minimum and maximum of three values
static void SG_MIN_MAX3 ( SGfloat &min, SGfloat &max, const SGfloat &a,
						  const SGfloat &b, const SGfloat &c)
{
	if( a > b ) {
		if( a > c ) {
			max = a;
			min = SG_MIN2( b,  c );
		} else {
			max = c;
			min = SG_MIN2( a, b );
		}
	} else {
		if( b > c ) {
			max = b;
			min = SG_MIN2( a, c );
		} else {
			max = c;
			min = SG_MIN2( a, b );
		}
	}
}

/*
 * Given a point and a triangle lying on the same plane
 * check to see if the point is inside the triangle
 */
bool sgPointInTriangle( sgVec3 point, sgVec3 tri[3] )
{
	sgVec3 dif;

	int i;
	for( i=0; i<3; i++ ) {
		SGfloat min, max;
		SG_MIN_MAX3 ( min, max, tri[0][i], tri[1][i], tri[2][i] );
		// punt if outside bouding cube
		if( (point[i] < min) || (point[i] > max) )
			return false;
		dif[i] = max - min;
	}

	// drop the smallest dimension so we only have to work in 2d.
	SGfloat min_dim = SG_MIN3 (dif[0], dif[1], dif[2]);
	SGfloat x1, y1, x2, y2, x3, y3, rx, ry;
	if ( fabs(min_dim-dif[0]) <= FLT_EPSILON ) {
		// x is the smallest dimension
		x1 = point[1];
		y1 = point[2];
		x2 = tri[0][1];
		y2 = tri[0][2];
		x3 = tri[1][1];
		y3 = tri[1][2];
		rx = tri[2][1];
		ry = tri[2][2];
	} else
	if ( fabs(min_dim-dif[1]) <= FLT_EPSILON ) {
		// y is the smallest dimension
		x1 = point[0];
		y1 = point[2];
		x2 = tri[0][0];
		y2 = tri[0][2];
		x3 = tri[1][0];
		y3 = tri[1][2];
		rx = tri[2][0];
		ry = tri[2][2];
	} else
	if ( fabs(min_dim-dif[2]) <= FLT_EPSILON ) {
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
	SGfloat tmp = (y2 - y3) / (x2 - x3);
	int side1 = SG_SIGN (tmp * (rx - x3) + y3 - ry);
	int side2 = SG_SIGN (tmp * (x1 - x3) + y3 - y1);
	if ( side1 != side2 ) {
		// printf("failed side 1 check\n");
		return false;
	}

	// check if intersection point is on correct side of p2 <-> p3 as p1
	tmp = (y3 - ry) / (x3 - rx);
	side1 = SG_SIGN (tmp * (x2 - rx) + ry - y2);
	side2 = SG_SIGN (tmp * (x1 - rx) + ry - y1);
	if ( side1 != side2 ) {
		// printf("failed side 2 check\n");
		return false;
	}

	// check if intersection point is on correct side of p1 <-> p3 as p2
	tmp = (y2 - ry) / (x2 - rx);
	side1 = SG_SIGN (tmp * (x3 - rx) + ry - y3);
	side2 = SG_SIGN (tmp * (x1 - rx) + ry - y1);
	if ( side1 != side2 ) {
		// printf("failed side 3  check\n");
		return false;
	}

	return true;
}

