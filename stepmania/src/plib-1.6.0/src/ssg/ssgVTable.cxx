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

void ssgVTable::copy_from ( ssgVTable *src, int clone_flags )
{
  ssgLeaf::copy_from ( src, clone_flags ) ;

  gltype  = src -> getPrimitiveType () ;
  indexed = src -> isIndexed () ;

  num_vertices  = src -> getNumVertices  () ;
  num_normals   = src -> getNumNormals   () ;
  num_colours   = src -> getNumColours   () ;
  num_texcoords = src -> getNumTexCoords () ;

  /*
    I should probably copy these arrays if SSG_CLONE_GEOMETRY
    is set - but life is short and ssgVTable is obsolete -
    and the inability to free the space that would create
    was the main reason to obsolete ssgVTable.
  */

  src -> getVertexList   ( (void **) & vertices , & v_index ) ;
  src -> getNormalList   ( (void **) & normals  , & n_index ) ;
  src -> getTexCoordList ( (void **) & texcoords, & t_index ) ;
  src -> getColourList   ( (void **) & colours  , & c_index ) ;

  recalcBSphere () ;
}


ssgBase *ssgVTable::clone ( int clone_flags )
{
  ssgVTable *b = new ssgVTable ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgVTable::ssgVTable ()
{
  type = ssgTypeVTable () ;
  gltype = GL_POINTS ;
  indexed = FALSE ;
  num_vertices  = 0 ; v_index = NULL ; vertices  = NULL ;
  num_normals   = 0 ; n_index = NULL ; normals   = NULL ;
  num_texcoords = 0 ; t_index = NULL ; texcoords = NULL ;
  num_colours   = 0 ; c_index = NULL ; colours   = NULL ;
}


ssgVTable::ssgVTable ( GLenum ty,
            int nv, unsigned short *vi, sgVec3 *vl,
            int nn, unsigned short *ni, sgVec3 *nl,
            int nt, unsigned short *ti, sgVec2 *tl,
            int nc, unsigned short *ci, sgVec4 *cl )
{
  type = ssgTypeVTable () ;
  gltype = ty ;

  indexed = TRUE ;
  num_vertices  = nv ; v_index = vi ; vertices  = vl ;
  num_normals   = nn ; n_index = ni ; normals   = nl ;
  num_texcoords = nt ; t_index = ti ; texcoords = tl ;
  num_colours   = nc ; c_index = ci ; colours   = cl ;

  recalcBSphere () ;
}



ssgVTable::ssgVTable ( GLenum ty,
            int nv, sgVec3 *vl,
            int nn, sgVec3 *nl,
            int nt, sgVec2 *tl,
            int nc, sgVec4 *cl )
{
  gltype = ty ;
  type = ssgTypeVTable () ;

  indexed = FALSE ;
  num_vertices  = nv ; vertices  = vl ;
  num_normals   = nn ; normals   = nl ;
  num_texcoords = nt ; texcoords = tl ;
  num_colours   = nc ; colours   = cl ;

  recalcBSphere () ;
}


ssgVTable::~ssgVTable ()
{
} 


void ssgVTable::getTriangle ( int n, short *v1, short *v2, short *v3 )
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
      *v1 = (n/2)*4 + (n&1) + 0 ;
      *v2 = (n/2)*4 + (n&1) + 1 ;
      *v3 = (n/2)*4 + (n&1) + 2 ;
      return ;

    default : return ;
  }
}


int ssgVTable::getNumTriangles ()
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

  return 0 ;   
}

int ssgVTable::getNumLines ()
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

void ssgVTable::getLine ( int n, short *v1, short *v2 )
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

void ssgVTable::transform ( const sgMat4 m )
{
  int i ;

  for ( i = 0 ; i < num_vertices ; i++ )
    sgXformPnt3 ( vertices[i], vertices[i], m ) ;

  for ( i = 0 ; i < num_normals ; i++ )
    sgXformVec3 ( normals[i], normals[i], m ) ;

  recalcBSphere () ;
}


void ssgVTable::recalcBSphere ()
{
  emptyBSphere () ;
  bbox . empty () ;

  int i ;

  if ( indexed )
    for ( i = 0 ; i < num_vertices ; i++ )
      bbox . extend ( vertices [ v_index [ i ] ] ) ;
  else
    for ( i = 0 ; i < num_vertices ; i++ )
      bbox . extend ( vertices [ i ] ) ;

  extendBSphere ( & bbox ) ;
  dirtyBSphere () ;  /* Cause parents to redo their bspheres */
  bsphere_is_invalid = FALSE ;
}


void ssgVTable::draw ()
{
  if ( ! preDraw () )
    return ;

  if ( hasState () ) getState () -> apply () ;

  stats_num_leaves++ ;
  stats_num_vertices += num_vertices ;

#ifdef _SSG_USE_DLIST
  if ( dlist )
    glCallList ( dlist ) ;
  else
#endif
    draw_geometry () ;

  if ( postDrawCB != NULL )
    (*postDrawCB)(this) ;
}

void ssgVTable::drawHighlight ( sgVec4 /* colour */, int /* vertex id */ )
{
}

void ssgVTable::drawHighlight ( sgVec4 /* colour */ )
{
}

void ssgVTable::pick ( int baseName )
{
  int i ;

  glPushName ( baseName ) ;

  if ( indexed )
  {
    glBegin ( gltype ) ;

    for ( i = 0 ; i < num_vertices ; i++ )
      glVertex3fv( vertices [ v_index [ i ]] ) ;
   
    glEnd () ;

  /* Then test each vertex in turn */

    for ( i = 0 ; i < num_vertices ; i++ )
    {
      glLoadName ( baseName + i + 1 ) ;
      glBegin  ( GL_POINTS ) ;
      glVertex3fv( vertices [ v_index [ i ]] ) ;
      glEnd    () ;
    }
  }
  else
  {
    glBegin ( gltype ) ;

    for ( i = 0 ; i < num_vertices ; i++ )
    {
      glLoadName ( baseName + i + 1 ) ;
      glVertex3fv( vertices  [ i ] ) ;
    }
   
    glEnd () ;

  /* Then test each vertex in turn */

    for ( i = 0 ; i < num_vertices ; i++ )
    {
      glLoadName ( baseName + i + 1 ) ;
      glBegin  ( GL_POINTS ) ;
      glVertex3fv( vertices [ i ] ) ;
      glEnd    () ;
    }
  }

  glPopName () ;
}

void ssgVTable::draw_geometry ()
{
  if ( indexed )
  {
    glBegin ( gltype ) ;

    if ( num_colours == 0 ) glColor4f   ( 1.0f, 1.0f, 1.0f, 1.0f ) ;
    if ( num_colours == 1 ) glColor4fv  ( colours [ c_index [ 0 ]] ) ;
    if ( num_normals == 1 ) glNormal3fv ( normals [ n_index [ 0 ]] ) ;
    
    for ( int i = 0 ; i < num_vertices ; i++ )
    {
      if ( num_colours   > 1 ) glColor4fv    ( colours   [ c_index [ i ]] ) ;
      if ( num_normals   > 1 ) glNormal3fv   ( normals   [ n_index [ i ]] ) ;
      if ( num_texcoords > 1 ) glTexCoord2fv ( texcoords [ t_index [ i ]] ) ;

      glVertex3fv   ( vertices  [ v_index [ i ]] ) ;
    }
   
    glEnd () ;
  }
  else
  {
    glBegin ( gltype ) ;

    if ( num_colours == 0 ) glColor4f   ( 1.0f, 1.0f, 1.0f, 1.0f ) ;
    if ( num_colours == 1 ) glColor4fv  ( colours [ 0 ] ) ;
    if ( num_normals == 1 ) glNormal3fv ( normals [ 0 ] ) ;
    
    for ( int i = 0 ; i < num_vertices ; i++ )
    {
      if ( num_colours   > 1 ) glColor4fv    ( colours   [ i ] ) ;
      if ( num_normals   > 1 ) glNormal3fv   ( normals   [ i ] ) ;
      if ( num_texcoords > 1 ) glTexCoord2fv ( texcoords [ i ] ) ;

      glVertex3fv   ( vertices  [ i ] ) ;
    }
   
    glEnd () ;
  }
}



void ssgVTable::hot_triangles ( sgVec3 s, sgMat4 m, int /* test_needed */ )
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

void ssgVTable::los_triangles ( sgVec3 s, sgMat4 m, int /* test_needed */ )
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

void ssgVTable::isect_triangles ( sgSphere *s, sgMat4 m, int test_needed )
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


void ssgVTable::print ( FILE *fd, char *indent, int how_much )
{
  ssgLeaf::print ( fd, indent, how_much ) ;
}



int ssgVTable::load ( FILE *fd )
{
  sgVec3 temp;

  _ssgReadVec3  ( fd, temp ); bbox.setMin( temp ) ;
  _ssgReadVec3  ( fd, temp ); bbox.setMax( temp ) ;
  _ssgReadInt   ( fd, &indexed ) ;
  _ssgReadInt   ( fd, (int *)(&gltype) ) ;

  _ssgReadInt   ( fd, &num_vertices ) ;
  _ssgReadInt   ( fd, &num_normals  ) ;
  _ssgReadInt   ( fd, &num_texcoords) ;
  _ssgReadInt   ( fd, &num_colours  ) ;

  int i ;
  int max ;

  /* Vertices */

  if ( indexed )
  {
    v_index = new unsigned short [ num_vertices ] ;
    _ssgReadUShort ( fd, num_vertices, v_index ) ;

    max = 0 ;

    for ( i = 0 ; i < num_vertices ; i++ )
      if ( v_index [ i ] > max ) max = v_index[i] ;      
  }
  else
    max = num_vertices ;

  vertices = new sgVec3 [ max ] ;
  _ssgReadFloat ( fd, max * 3, (float *) vertices ) ;

  /* Normals */

  if ( indexed )
  {
    n_index = new unsigned short [ num_normals ] ;
    _ssgReadUShort ( fd, num_normals, n_index ) ;

    max = 0 ;

    for ( i = 0 ; i < num_normals ; i++ )
      if ( n_index [ i ] > max ) max = n_index[i] ;      
  }
  else
    max = num_normals ;

  normals = new sgVec3 [ max ] ;
  _ssgReadFloat ( fd, max * 3, (float *) normals ) ;

  /* Texture Coordinates */

  if ( indexed )
  {
    t_index = new unsigned short [ num_texcoords ] ;
    _ssgReadUShort ( fd, num_texcoords, t_index ) ;

    max = 0 ;

    for ( i = 0 ; i < num_texcoords ; i++ )
      if ( t_index [ i ] > max ) max = t_index[i] ;      
  }
  else
    max = num_texcoords ;

  texcoords = new sgVec2 [ max ] ;
  _ssgReadFloat ( fd, max * 2, (float *) texcoords ) ;

  /* Colours */

  if ( indexed )
  {
    c_index = new unsigned short [ num_colours ] ;
    _ssgReadUShort ( fd, num_colours, c_index ) ;

    max = 0 ;

    for ( i = 0 ; i < num_colours ; i++ )
      if ( c_index [ i ] > max ) max = c_index[i] ;      
  }
  else
    max = num_colours ;

  colours = new sgVec4 [ max ] ;
  _ssgReadFloat ( fd, max * 4, (float *) colours ) ;

  return ssgLeaf::load(fd) ;
}

int ssgVTable::save ( FILE *fd )
{
  _ssgWriteVec3  ( fd, bbox.getMin() ) ;
  _ssgWriteVec3  ( fd, bbox.getMax() ) ;
  _ssgWriteInt   ( fd, indexed ) ;
  _ssgWriteInt   ( fd, (int) gltype ) ;

  _ssgWriteInt   ( fd, num_vertices ) ;
  _ssgWriteInt   ( fd, num_normals  ) ;
  _ssgWriteInt   ( fd, num_texcoords) ;
  _ssgWriteInt   ( fd, num_colours  ) ;

  int i ;
  int max ;

  /* Vertices */

  if ( indexed )
  {
    max = 0 ;

    for ( i = 0 ; i < num_vertices ; i++ )
      if ( v_index [ i ] > max ) max = v_index[i] ;      

    _ssgWriteUShort ( fd, num_vertices, v_index ) ;
  }
  else
    max = num_vertices ;

  _ssgWriteFloat ( fd, max * 3, (float *)vertices ) ;

  /* Normals */

  if ( indexed )
  {
    max = 0 ;

    for ( i = 0 ; i < num_normals ; i++ )
      if ( n_index [ i ] > max ) max = n_index[i] ;      

    _ssgWriteUShort ( fd, num_normals, n_index ) ;
  }
  else
    max = num_normals ;

  _ssgWriteFloat ( fd, max * 3, (float *)normals ) ;

  /* Texture Coordinates */

  if ( indexed )
  {
    max = 0 ;

    for ( i = 0 ; i < num_texcoords ; i++ )
      if ( t_index [ i ] > max ) max = t_index[i] ;      

    _ssgWriteUShort ( fd, num_texcoords, t_index ) ;
  }
  else
    max = num_texcoords ;

  _ssgWriteFloat ( fd, max * 2, (float *)texcoords ) ;

  /* Colours */

  if ( indexed )
  {
    max = 0 ;

    for ( i = 0 ; i < num_colours ; i++ )
      if ( c_index [ i ] > max ) max = c_index[i] ;      

    _ssgWriteUShort ( fd, num_colours, c_index ) ;
  }
  else
    max = num_colours ;

  _ssgWriteFloat ( fd, max * 4, (float *)colours ) ;

  return ssgLeaf::save(fd) ;
}



