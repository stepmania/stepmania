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


#include "ssgLocal.h"

#define HL_DELTA 0.04f

void ssgVtxTable::copy_from ( ssgVtxTable *src, int clone_flags )
{
  ssgLeaf::copy_from ( src, clone_flags ) ;

  gltype = src -> getPrimitiveType () ;

  if ( src->vertices != NULL && ( clone_flags & SSG_CLONE_GEOMETRY ) )
    vertices = (ssgVertexArray *)( src -> vertices -> clone ( clone_flags )) ;
  else
    vertices = src -> vertices ;

  if ( src->normals != NULL && ( clone_flags & SSG_CLONE_GEOMETRY ) )
    normals = (ssgNormalArray *)( src -> normals -> clone ( clone_flags )) ;
  else
    normals = src -> normals ;

  if ( src->texcoords != NULL && ( clone_flags & SSG_CLONE_GEOMETRY ) )
    texcoords = (ssgTexCoordArray *)( src -> texcoords -> clone ( clone_flags )) ;
  else
    texcoords = src -> texcoords ;

  if ( src->colours != NULL && ( clone_flags & SSG_CLONE_GEOMETRY ) )
    colours = (ssgColourArray *)( src -> colours -> clone ( clone_flags )) ;
  else
    colours = src -> colours ;

  if ( vertices  != NULL ) vertices  -> ref () ;
  if ( normals   != NULL ) normals   -> ref () ;
  if ( texcoords != NULL ) texcoords -> ref () ;
  if ( colours   != NULL ) colours   -> ref () ;

  recalcBSphere () ;
}

ssgBase *ssgVtxTable::clone ( int clone_flags )
{
  ssgVtxTable *b = new ssgVtxTable ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgVtxTable::ssgVtxTable ()
{
  type = ssgTypeVtxTable () ;
  gltype = GL_POINTS ;
  vertices  = NULL ;
  normals   = NULL ;
  texcoords = NULL ;
  colours   = NULL ;
}


ssgVtxTable::ssgVtxTable ( GLenum ty,
                ssgVertexArray   *vl,
                ssgNormalArray   *nl,
                ssgTexCoordArray *tl,
                ssgColourArray   *cl )
{
  gltype = ty ;
  type = ssgTypeVtxTable () ;

  vertices  = (vl!=NULL) ? vl : new ssgVertexArray   () ;
  normals   = (nl!=NULL) ? nl : new ssgNormalArray   () ;
  texcoords = (tl!=NULL) ? tl : new ssgTexCoordArray () ;
  colours   = (cl!=NULL) ? cl : new ssgColourArray   () ;

  vertices  -> ref () ;
  normals   -> ref () ;
  texcoords -> ref () ;
  colours   -> ref () ;

  recalcBSphere () ;
}

void ssgVtxTable::setVertices ( ssgVertexArray *vl )
{
  ssgDeRefDelete ( vertices ) ;
  vertices = vl ;

  if ( vertices != NULL )
    vertices -> ref () ;

  recalcBSphere () ;
}

void ssgVtxTable::setNormals ( ssgNormalArray *nl )
{
  ssgDeRefDelete ( normals ) ;
  normals = nl ;

  if ( normals != NULL )
    normals -> ref () ;
}

void ssgVtxTable::setTexCoords ( ssgTexCoordArray *tl )
{
  ssgDeRefDelete ( texcoords ) ;
  texcoords = tl ;

  if ( texcoords != NULL )
    texcoords -> ref () ;
}

void ssgVtxTable::setColours ( ssgColourArray *cl )
{
  ssgDeRefDelete ( colours ) ;
  colours = cl ;

  if ( colours != NULL )
    colours -> ref () ;
}

ssgVtxTable::~ssgVtxTable ()
{
  ssgDeRefDelete ( vertices  ) ;
  ssgDeRefDelete ( normals   ) ;
  ssgDeRefDelete ( texcoords ) ;
  ssgDeRefDelete ( colours   ) ;
} 


void ssgVtxTable::getTriangle ( int n, short *v1, short *v2, short *v3 )
{
  switch ( getPrimitiveType () )
  {
    case GL_POLYGON :
    case GL_TRIANGLE_FAN :
      *v1 =  0  ;
      *v2 = n+1 ;
      *v3 = n+2 ;
      return ;

    case GL_TRIANGLES :
      *v1 = n*3 ;
      *v2 = n*3+1 ;
      *v3 = n*3+2 ;
      return ;

    case GL_TRIANGLE_STRIP :
    case GL_QUAD_STRIP :
      if ( n & 1 )
      {
        *v3 =  n  ;
        *v2 = n+1 ;
        *v1 = n+2 ;
      }
      else
      {
        *v1 =  n  ;
        *v2 = n+1 ;
        *v3 = n+2 ;
      }
      return ;

    case GL_QUADS :
      *v1 = n*2 + 0 ;
      *v2 = n*2 + 1 ;
      *v3 = n*2 + 2 - (n&1)*4 ;
      return ;

    default : return ;
  }
}


int ssgVtxTable::getNumTriangles ()
{
  switch ( getPrimitiveType () )
  {
    case GL_POLYGON :
    case GL_TRIANGLE_FAN :
      return getNumVertices() - 2 ;

    case GL_TRIANGLES :
      return getNumVertices() / 3 ;

    case GL_TRIANGLE_STRIP :
      return getNumVertices() - 2 ;

    case GL_QUADS :
      return ( getNumVertices() / 4 ) * 2 ;

    case GL_QUAD_STRIP :
      return ( ( getNumVertices() - 2 ) / 2 ) * 2 ;
    default : break ;
  }

  return 0 ;   /* Should never get here...but you never know! 
	                wk: Yes it does, for GL_POINTS      
										GL_LINES       
										GL_LINE_LOOP   
										GL_LINE_STRIP  
										*/
}

int ssgVtxTable::getNumLines ()
{
  switch ( getPrimitiveType () )
  {
    case GL_POLYGON :
    case GL_TRIANGLE_FAN :
    case GL_TRIANGLES :
    case GL_TRIANGLE_STRIP :
    case GL_QUADS :
    case GL_QUAD_STRIP :
    case GL_POINTS :
      return 0;
		case GL_LINES :   
			// wk: FIXME: check the 3 following formulas. I don't have an OpenGL bokk at hand currently :-(
			return getNumVertices()/2;
    case GL_LINE_LOOP :
    	return getNumVertices();
    case GL_LINE_STRIP :
    	return getNumVertices()-1;
    default : break ;
  }
  assert(false); /* Should never get here  */
  return 0 ;   
}

void ssgVtxTable::getLine ( int n, short *v1, short *v2 )
{ 
	assert( n>=0 );
  switch ( getPrimitiveType () )
  {
    case GL_POLYGON :
    case GL_TRIANGLE_FAN :
    case GL_TRIANGLES :
    case GL_TRIANGLE_STRIP :
    case GL_QUADS :
    case GL_QUAD_STRIP :
    case GL_POINTS :
      assert(false);
		case GL_LINES :   
			// wk: FIXME: check the 3 following formulas. I don't have an OpenGL bokk at hand currently :-(
			assert ( 2*n+1 < getNumVertices() );
			*v1 = 2*n;
			*v2 = 2*n+1 ;
			return ;
    case GL_LINE_LOOP :
			assert ( n < getNumVertices() );
			*v1 = n;
			if ( n == getNumVertices()-1 )
			  *v2 = 0;
			else
				*v2 = n+1;
			return ;
    case GL_LINE_STRIP :
    	assert ( n < getNumVertices()-1 );
			*v1 = n;
			*v2 = n+1;
			return;
    default :
			break ;
  }
  assert(false); /* Should never get here  */
  return ;   
}


void ssgVtxTable::transform ( const sgMat4 m )
{
  int i ;
  int flags = sgClassifyMat4 ( m ) ;

  if ( flags == 0 )
    return ;


  if ( ( flags & SG_PROJECTION ) )
    ulSetError ( UL_WARNING, "ssgVtxTable: Projection matrices currently not supported." ) ;
  /*
    note: it is possible to handle projections, but for each normal we would
    have to know the corresponding vertex coordinates. setting:
        n[3] = dot(v, n) / v[3]
    and then transforming with the full 4x4 transposed inverse matrix would do it.
    see the OpenGL spec.
  */


  for ( i = 0 ; i < getNumVertices() ; i++ )
    sgXformPnt3 ( vertices->get(i), vertices->get(i), m ) ;


  sgMat4 w ;

  if ( ( flags & ( SG_SCALE | SG_MIRROR | SG_NONORTHO ) ) )
  {
    if ( ( flags & SG_NONORTHO ) )
    {
      // use the transposed adjoint matrix (only the upper 3x3 is needed)
      sgVectorProductVec3 ( w[0], m[1], m[2] ) ;
      sgVectorProductVec3 ( w[1], m[2], m[0] ) ;
      sgVectorProductVec3 ( w[2], m[0], m[1] ) ;
    }
    else
    {
      SGfloat scale = SG_ONE ;

      if ( ( flags & SG_SCALE ) )
      {
	// prescale matrix to avoid renormalisation
	scale = scale / sgLengthVec3 ( m[0] ) ;
      }

      if ( ( flags & SG_MIRROR ) )
      {
	// negate to keep normals consistent with triangle orientations
	scale = - scale ;
      }

      sgScaleVec3 ( w[0], m[0], scale ) ;
      sgScaleVec3 ( w[1], m[1], scale ) ;
      sgScaleVec3 ( w[2], m[2], scale ) ;
    }

    m = w ;
  }


  for ( i = 0 ; i < getNumNormals() ; i++ )
    sgXformVec3 ( normals->get(i), normals->get(i), m ) ;


  if ( ( flags & SG_NONORTHO ) )
  {
    for ( i = 0 ; i < getNumNormals() ; i++ )
      sgNormaliseVec3 ( normals->get(i) ) ;
  }


  recalcBSphere () ;
}


void ssgVtxTable::recalcBSphere ()
{
  emptyBSphere () ;
  bbox . empty () ;

  for ( int i = 0 ; i < getNumVertices() ; i++ )
    bbox . extend ( vertices->get(i) ) ;

  extendBSphere ( & bbox ) ;
  dirtyBSphere () ;  /* Cause parents to redo their bspheres */
  bsphere_is_invalid = FALSE ;
}


void ssgVtxTable::drawHighlight ( sgVec4 colour )
{
  _ssgForceLineState () ;

  int i ;
  int num_vertices  = getNumVertices  () ;

  sgVec3 *vx = (sgVec3 *) vertices -> get(0) ;

  glPushAttrib ( GL_POLYGON_BIT ) ;
  glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE ) ;
  glColor4fv ( colour ) ;
  glBegin ( gltype ) ;
  for ( i = 0 ; i < num_vertices ; i++ )
    glVertex3fv ( vx [ i ] ) ;
  glEnd () ;
  glPopAttrib () ;
  glEnable ( GL_DEPTH_TEST ) ;
}


void ssgVtxTable::drawHighlight ( sgVec4 colour, int v )
{
  _ssgForceLineState () ;

  int num_vertices  = getNumVertices  () ;

  if ( v < 0 || v >= num_vertices )
    return ;

  sgVec3 *vx = (sgVec3 *) vertices -> get(v) ;

  float x = vx[0][0] ;
  float y = vx[0][1] ;
  float z = vx[0][2] ;

  sgVec3 t[6] ;
  sgSetVec3 ( t[0], x-HL_DELTA,y,z ) ;
  sgSetVec3 ( t[1], x+HL_DELTA,y,z ) ;
  sgSetVec3 ( t[2], x,y-HL_DELTA,z ) ;
  sgSetVec3 ( t[3], x,y+HL_DELTA,z ) ;
  sgSetVec3 ( t[4], x,y,z-HL_DELTA ) ;
  sgSetVec3 ( t[5], x,y,z+HL_DELTA ) ;
  glColor4fv ( colour ) ;
  glLineWidth ( 4.0f ) ;
  glBegin ( GL_LINES ) ;
  glVertex3fv ( t[0] ) ;
  glVertex3fv ( t[1] ) ;
  glVertex3fv ( t[2] ) ;
  glVertex3fv ( t[3] ) ;
  glVertex3fv ( t[4] ) ;
  glVertex3fv ( t[5] ) ;
  glEnd () ;
  glLineWidth ( 1.0f ) ;
  glEnable ( GL_DEPTH_TEST ) ;
}


void ssgVtxTable::draw ()
{
  if ( ! preDraw () )
    return ;

  if ( hasState () ) getState () -> apply () ;

  stats_num_leaves++ ;
  stats_num_vertices += getNumVertices() ;

#ifdef _SSG_USE_DLIST
  if ( dlist )
    glCallList ( dlist ) ;
  else
#endif
    draw_geometry () ;

  if ( postDrawCB != NULL )
    (*postDrawCB)(this) ;
}


void ssgVtxTable::pick ( int baseName )
{
  int i ;
  int num_vertices  = getNumVertices  () ;

  sgVec3 *vx = (sgVec3 *) vertices -> get(0) ;

  /* Test the entire primitive. */

  glPushName ( baseName ) ;
  glBegin  ( gltype ) ;

  for ( i = 0 ; i < num_vertices ; i++ )
    glVertex3fv ( vx [ i ] ) ;
 
  glEnd     () ;

  /* Then test each vertex in turn */

  for ( i = 0 ; i < num_vertices ; i++ )
  {
    glLoadName  ( baseName + i + 1 ) ;
    glBegin  ( GL_POINTS ) ;
    glVertex3fv ( vx [ i ] ) ;
    glEnd     () ;
  }

  glPopName () ;
}


void ssgVtxTable::draw_geometry ()
{
  int num_vertices  = getNumVertices  () ;
  int num_colours   = getNumColours   () ;
  int num_normals   = getNumNormals   () ;
  int num_texcoords = getNumTexCoords () ;

  if ( num_colours == 0 ) glColor4f   ( 1.0f, 1.0f, 1.0f, 1.0f ) ;
  if ( num_colours == 1 ) glColor4fv  ( colours -> get(0) ) ;
  if ( num_normals == 1 ) glNormal3fv ( normals -> get(0) ) ;
  
  glPushClientAttrib ( GL_CLIENT_VERTEX_ARRAY_BIT ) ;

  if ( num_colours > 1 )
  {
    glEnableClientState ( GL_COLOR_ARRAY ) ;
    glColorPointer ( 4, GL_FLOAT, 0, colours->get(0) ) ;
  }

  if ( num_normals > 1 )
  {
    glEnableClientState ( GL_NORMAL_ARRAY ) ;
    glNormalPointer ( GL_FLOAT, 0, normals->get(0) ) ;
  }

  if ( num_texcoords > 1 )
  {
    glEnableClientState ( GL_TEXTURE_COORD_ARRAY ) ;
    glTexCoordPointer ( 2, GL_FLOAT, 0, texcoords->get(0) ) ;
  }

  glEnableClientState ( GL_VERTEX_ARRAY ) ;
  glVertexPointer ( 3, GL_FLOAT, 0, vertices->get(0) ) ;

  glDrawArrays ( gltype, 0, num_vertices ) ;

  glPopClientAttrib () ;
}



void ssgVtxTable::hot_triangles ( sgVec3 s, sgMat4 m, int /* test_needed */ )
{
  int nt = getNumTriangles () ;

  stats_hot_triangles += nt ;

  for ( int i = 0 ; i < nt ; i++ )
  {
    short   v1,  v2,  v3 ;
    sgVec3 vv1, vv2, vv3 ;
    sgVec4 plane ;

    getTriangle ( i, &v1, &v2, &v3 ) ;

    sgXformPnt3 ( vv1, getVertex(v1), m ) ;
    sgXformPnt3 ( vv2, getVertex(v2), m ) ;
    sgXformPnt3 ( vv3, getVertex(v3), m ) ;

    /*
      Does the X/Y coordinate lie outside the triangle's bbox, or
      does the Z coordinate lie beneath the bbox ?
    */

    if ( ( s[0] < vv1[0] && s[0] < vv2[0] && s[0] < vv3[0] ) ||
         ( s[1] < vv1[1] && s[1] < vv2[1] && s[1] < vv3[1] ) ||
         ( s[0] > vv1[0] && s[0] > vv2[0] && s[0] > vv3[0] ) ||
         ( s[1] > vv1[1] && s[1] > vv2[1] && s[1] > vv3[1] ) ||
         ( s[2] < vv1[2] && s[2] < vv2[2] && s[2] < vv3[2] ) )
      continue ;

    sgMakePlane ( plane, vv1, vv2, vv3 ) ;

    if ( _ssgIsHotTest )
    {
      /* No HOT from upside-down or vertical triangles */

      if ( getCullFace() && plane [ 2 ] <= 0 )
        continue ;

      /* Find the point vertically below the text point
      as it crosses the plane of the polygon */

      float z = sgHeightOfPlaneVec2 ( plane, s ) ;

      /* No HOT from below the triangle */

      if ( z > s[2] )
         continue ;

      /* Outside the vertical extent of the triangle? */

      if ( ( z < vv1[2] && z < vv2[2] && z < vv3[2] ) ||
         ( z > vv1[2] && z > vv2[2] && z > vv3[2] ) )
         continue ;
    }

    /*
      Now it gets messy - the isect point is inside
      the bbox of the triangle - but that's not enough.
      Is it inside the triangle itself?
    */

    float  e1 =  s [0] * vv1[1] -  s [1] * vv1[0] ;
    float  e2 =  s [0] * vv2[1] -  s [1] * vv2[0] ;
    float  e3 =  s [0] * vv3[1] -  s [1] * vv3[0] ;
    float ep1 = vv1[0] * vv2[1] - vv1[1] * vv2[0] ;
    float ep2 = vv2[0] * vv3[1] - vv2[1] * vv3[0] ;
    float ep3 = vv3[0] * vv1[1] - vv3[1] * vv1[0] ;

    float ap = (float) fabs ( ep1 + ep2 + ep3 ) ;
    float ai = (float) ( fabs ( e1 + ep1 - e2 ) +
                         fabs ( e2 + ep2 - e3 ) +
                         fabs ( e3 + ep3 - e1 ) ) ;

    if ( ai > ap * 1.01 )
      continue ;

    _ssgAddHit ( this, i, m, plane ) ;
  }
}

void ssgVtxTable::los_triangles ( sgVec3 s, sgMat4 m, int /* test_needed */ )
{
  int nt = getNumTriangles () ;

  stats_los_triangles += nt ;

  for ( int i = 0 ; i < nt ; i++ )
  {
    short   v1,  v2,  v3 ;
    sgVec3 vv1, vv2, vv3 ;
    sgVec4 plane ;

    SGfloat edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
    SGfloat det,inv_det;
    SGfloat /*t,*/u,v;

    getTriangle ( i, &v1, &v2, &v3 ) ;

    sgXformPnt3 ( vv1, getVertex(v1), m ) ;
    sgXformPnt3 ( vv2, getVertex(v2), m ) ;
    sgXformPnt3 ( vv3, getVertex(v3), m ) ;
    sgVec3 cam;
    cam[0] = m[0][3];
    cam[1] = m[1][3];
    cam[2] = m[2][3];
    //if ( _ssgIsLosTest )
    //{

      /* find vectors for two edges sharing vert0 */
      sgSubVec3(edge1, vv2, vv1);
      sgSubVec3(edge2, vv3, vv1);

      /* begin calculating determinant - also used to calculate U parameter */
      sgVectorProductVec3(pvec, s, edge2);

      /* if determinant is near zero, ray lies in plane of triangle */
      det = sgScalarProductVec3(edge1, pvec);

      if (det > -0.0000001 && det < 0.0000001) continue;
      inv_det = (float)1.0 / det;

      /* calculate distance from vert0 to ray origin */
      sgSubVec3(tvec, cam, vv1);

      /* calculate U parameter and test bounds */
      u = sgScalarProductVec3(tvec, pvec) * inv_det;
      if (u < 0.0 || u > 1.0)
         continue;

      /* prepare to test V parameter */
      sgVectorProductVec3(qvec, tvec, edge1);

      /* calculate V parameter and test bounds */
      v = sgScalarProductVec3(s, qvec) * inv_det;
      if (v < 0.0 || u + v > 1.0)
         continue;

      /* calculate t, ray intersects triangle */
      //t = sgScalarProductVec3(edge2, qvec) * inv_det;
    //}

    sgMakePlane ( plane, vv1, vv2, vv3 ) ;
    _ssgAddHit ( this, i, m, plane ) ;
  }
}


void ssgVtxTable::isect_triangles ( sgSphere *s, sgMat4 m, int test_needed )
{
  int nt = getNumTriangles () ;

  stats_isect_triangles += nt ;

  for ( int i = 0 ; i < nt ; i++ )
  {
    short   v1,  v2,  v3 ;
    sgVec3 vv1, vv2, vv3 ;
    sgVec4 plane ;

    getTriangle ( i, &v1, &v2, &v3 ) ;

    sgXformPnt3 ( vv1, getVertex(v1), m ) ;
    sgXformPnt3 ( vv2, getVertex(v2), m ) ;
    sgXformPnt3 ( vv3, getVertex(v3), m ) ;

    sgMakePlane ( plane, vv1, vv2, vv3 ) ;

    if ( ! test_needed )
    {
      _ssgAddHit ( this, i, m, plane ) ;
      continue ;
    }

    float dp = (float) fabs ( sgDistToPlaneVec3 ( plane, s->getCenter() ) ) ;

    if ( dp > s->getRadius() )
      continue ;

    /*
      The BSphere touches the plane containing
      the triangle - but does it actually touch
      the triangle itself?  Let's erect some
      vertical walls around the triangle.
    */

    /*
      Construct a 'wall' as a plane through
      two vertices and a third vertex made
      by adding the surface normal to the
      first of those two vertices.
    */

    sgVec3 vvX ;
    sgVec4 planeX ;

    sgAddVec3 ( vvX, plane, vv1 ) ;
    sgMakePlane ( planeX, vv1, vv2, vvX ) ;
    float dp1 = sgDistToPlaneVec3 ( planeX, s->getCenter() ) ;
    
    if ( dp1 > s->getRadius() )
      continue ;

    sgAddVec3 ( vvX, plane, vv2 ) ;
    sgMakePlane ( planeX, vv2, vv3, vvX ) ;
    float dp2 = sgDistToPlaneVec3 ( planeX, s->getCenter() ) ;
    
    if ( dp2 > s->getRadius() )
      continue ;

    sgAddVec3 ( vvX, plane, vv3 ) ;
    sgMakePlane ( planeX, vv3, vv1, vvX ) ;
    float dp3 = sgDistToPlaneVec3 ( planeX, s->getCenter() ) ;
    
    if ( dp3 > s->getRadius() )
      continue ;

    /*
      OK, so we now know that the sphere
      intersects the plane of the triangle
      and is not more than one radius outside
      the walls. However, you can still get
      close enough to the wall and to the
      triangle itself and *still* not
      intersect the triangle itself.

      However, if the center is inside the
      triangle then we don't need that
      costly test.
    */
 
    if ( dp1 <= 0 && dp2 <= 0 && dp3 <= 0 )
    {
      _ssgAddHit ( this, i, m, plane ) ;
      continue ;
    }

    /*
      <sigh> ...now we really need that costly set of tests...

      If the sphere penetrates the plane of the triangle
      and the plane of the wall, then we can use pythagoras
      to determine if the sphere actually intersects that
      edge between the wall and the triangle.

        if ( dp_sqd + dp1_sqd > radius_sqd ) ...in! else ...out!
    */

    float r2 = s->getRadius() * s->getRadius() - dp * dp ;

    if ( dp1 * dp1 <= r2 ||
         dp2 * dp2 <= r2 ||
         dp3 * dp3 <= r2 )
    {
      _ssgAddHit ( this, i, m, plane ) ;
      continue ;
    }
  }
}


void ssgVtxTable::print ( FILE *fd, char *indent, int how_much )
{
  char in [ 100 ] ;

  if ( how_much == 0 ) 
    return ;
  
  sprintf ( in, "%s  ", indent );
	
  ssgLeaf  ::print ( fd, indent, how_much ) ;
		
  if ( vertices == NULL )
    fprintf ( fd, "%s  No Vertices!\n", indent ) ;
  else
    vertices  -> print ( fd, in, how_much ) ;

  if ( normals == NULL )
    fprintf ( fd, "%s  No Normals!\n", indent ) ;
  else
    normals   -> print ( fd, in, how_much ) ;

  if ( texcoords == NULL )
    fprintf ( fd, "%s  No Texcoords!\n", indent ) ;
  else
    texcoords -> print ( fd, in, how_much ) ;

  if ( colours == NULL )
    fprintf ( fd, "%s  No Colours!\n", indent ) ;
  else
    colours   -> print ( fd, in, how_much ) ;
}



int ssgVtxTable::load ( FILE *fd )
{
  sgVec3 temp;

  _ssgReadVec3  ( fd, temp ); bbox.setMin( temp ) ;
  _ssgReadVec3  ( fd, temp ); bbox.setMax( temp ) ;
  _ssgReadInt   ( fd, (int *)(&gltype) ) ;

  if ( ! ssgLeaf::load(fd) )
    return FALSE ;

  if ( ! _ssgLoadObject ( fd, (ssgBase **)&vertices,  ssgTypeVertexArray ()   ) ||
       ! _ssgLoadObject ( fd, (ssgBase **)&normals,   ssgTypeNormalArray ()   ) ||
       ! _ssgLoadObject ( fd, (ssgBase **)&texcoords, ssgTypeTexCoordArray () ) ||
       ! _ssgLoadObject ( fd, (ssgBase **)&colours,   ssgTypeColourArray ()   ) )
    return FALSE ;

  if ( vertices  != NULL ) vertices  -> ref () ;
  if ( normals   != NULL ) normals   -> ref () ;
  if ( texcoords != NULL ) texcoords -> ref () ;
  if ( colours   != NULL ) colours   -> ref () ;
     
  return TRUE ;
}


int ssgVtxTable::save ( FILE *fd )
{
  _ssgWriteVec3  ( fd, bbox.getMin() ) ;
  _ssgWriteVec3  ( fd, bbox.getMax() ) ;
  _ssgWriteInt   ( fd, (int) gltype ) ;

  if ( ! ssgLeaf::save(fd) )
    return FALSE ;

  if ( ! _ssgSaveObject ( fd, vertices  ) ||
       ! _ssgSaveObject ( fd, normals   ) ||
       ! _ssgSaveObject ( fd, texcoords ) ||
       ! _ssgSaveObject ( fd, colours   ) )
    return FALSE ;

  return TRUE ;
}
