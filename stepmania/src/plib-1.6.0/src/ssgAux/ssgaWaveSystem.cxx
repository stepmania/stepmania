
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


#include "ssgAux.h"
#include <string.h>

#define G 9.8f

void ssgaWaveSystem::updateAnimation ( float tim )
{
  if ( ntriangles <= 0 ||
       normals    == NULL || colours    == NULL ||
       texcoords  == NULL || vertices   == NULL ||
       orig_vertices == NULL )
    return ;

  int i ;

  float adjSpeed   [ SSGA_MAX_WAVETRAIN ] ;
  float sinHeading [ SSGA_MAX_WAVETRAIN ] ;
  float cosHeading [ SSGA_MAX_WAVETRAIN ] ;
  float length     [ SSGA_MAX_WAVETRAIN ] ;
  float lambda     [ SSGA_MAX_WAVETRAIN ] ;
  float height     [ SSGA_MAX_WAVETRAIN ] ;

  /* Pre-adjust speed's to allow for wind speed. */

  int num_trains = 0 ;

  for ( i = 0 ; i < SSGA_MAX_WAVETRAIN ; i++ )
    if ( train [ i ] != NULL )
    {
      adjSpeed   [num_trains] = train [ i ] -> getSpeed () * G *
                                                      tim / windSpeed ;
      sinHeading [num_trains] = (float) -sin ( train[i]->getHeading () *
                                                      SG_DEGREES_TO_RADIANS ) ;
      cosHeading [num_trains] = (float) cos ( train[i]->getHeading () *
                                                      SG_DEGREES_TO_RADIANS ) ;
      length     [num_trains] = train [ i ] -> getLength     () ;
      lambda     [num_trains] = train [ i ] -> getLambda     () ;
      height     [num_trains] = train [ i ] -> getWaveHeight () ;
      num_trains++ ;
    }

  for ( i = 0 ; i <= nstrips ; i++ )
  {
    float fade_i = (i<2) ? 0.0f : (i<7) ? (float)(i-2)/5.0f :
		   (i>nstrips-2) ? 0.0f :
		   (i>nstrips-7) ? (float)(nstrips-i-2)/5.0f : 1.0f ;

    for ( int j = 0 ; j <= nstacks ; j++ )
    {
      float fade_j = (j<2) ? 0.0f : (j<7) ? (float)(j-2)/5.0f :
		     (j>nstacks-2) ? 0.0f :
		     (j>nstacks-7) ? (float)(nstacks-j-2)/5.0f : 1.0f ;

      float edge_fade = fade_i * fade_j ;

      int  idx = i * (nstrips+1) + j ;

      float x0 = orig_vertices [idx][0] + center[0] ;
      float y0 = orig_vertices [idx][1] + center[1] ; 
      float z0 = vertices [idx][2] ;

      float depth = (gridGetter==NULL) ? 1000000.0f : gridGetter ( x0, y0 ) ;

      float xx = x0 ;
      float yy = y0 ;
      float zz = center[2] ;

      for ( int t = 0 ; t < num_trains ; t++ )
      {
	float adjHeight = height [ t ] * edge_fade ;
	float adjLength = ( depth < 0.2f ) ? 0.2f :
	                    ( depth > length[t] ) ? length[t] : depth ;

	float phase = ( x0 * sinHeading[t] + y0 * cosHeading[t] ) / adjLength -
                                             adjSpeed[t] - lambda[t] * z0 ;

        float delta = adjHeight * (float) sin ( phase ) ;

  	xx += delta * sinHeading [ t ] ;
  	yy += delta * cosHeading [ t ] ;
        zz += adjHeight * (float) -cos ( phase ) ;
      }

      sgSetVec3 ( vertices  [idx], xx, yy, zz ) ; 
      sgSetVec2 ( texcoords [idx], tu * x0 / size[0], tv * y0 /size[1] ) ;
    }
  }

  for ( i = 0 ; i < nstrips ; i++ )
  {
    int i1 =   i   * (nstrips+1) ;
    int i2 = (i+1) * (nstrips+1) ;

    for ( int j = 0 ; j < nstacks ; j++ )
    {
      int idx1 = i1 +   j   ;
      int idx2 = i2 +   j   ;
      int idx3 = i1 + (j+1) ;

      sgVec3 ab ; sgSubVec3 ( ab, vertices[idx3], vertices[idx1] ) ;
      sgVec3 ac ; sgSubVec3 ( ac, vertices[idx2], vertices[idx1] ) ;

      float nx = ab[1] * ac[2] - ab[2] * ac[1] ;
      float ny = ab[2] * ac[0] - ab[0] * ac[2] ;
      float nz = ab[0] * ac[1] - ab[1] * ac[0] ;

      /* About 10% of execution time is in this instruction!  */
      float rlen = 1.0f / (float) sqrt ( nx * nx + ny * ny + nz * nz ) ;

      normals[idx1][0] = nx * rlen ;
      normals[idx1][1] = ny * rlen ;
      normals[idx1][2] = nz * rlen ;
    }
  }

  for ( i = 0 ; i < nstrips ; i++ )
  {
    ssgVtxTable      *vt = (ssgVtxTable *) getKid ( i ) ;
    ssgVertexArray   *vv = vt -> getVertices  () ;
    ssgNormalArray   *nn = vt -> getNormals   () ;
    ssgColourArray   *cc = vt -> getColours   () ;
    ssgTexCoordArray *tt = vt -> getTexCoords () ;

    int i1 = (i+1) * (nstrips+1) ;
    int i2 =   i   * (nstrips+1) ;

    for ( int j = 0, jj = 0 ; j < nstacks + 1 ; j++, jj += 2, i1++, i2++ )
    {
      vv -> set ( vertices [i1], jj   ) ; vv -> set ( vertices [i2], jj+1 ) ;
      nn -> set ( normals  [i1], jj   ) ; nn -> set ( normals  [i2], jj+1 ) ;
      cc -> set ( colours  [i1], jj   ) ; cc -> set ( colours  [i2], jj+1 ) ;
      tt -> set ( texcoords[i1], jj   ) ; tt -> set ( texcoords[i2], jj+1 ) ;
    }
  }
}


void ssgaWaveSystem::copy_from ( ssgaWaveSystem *src, int clone_flags )
{
  ssgaShape::copy_from ( src, clone_flags ) ;
 
  setDepthCallback ( src -> getDepthCallback () ) ;
  setWindSpeed     ( src -> getWindSpeed     () ) ;
  setWindDirn      ( src -> getWindDirn      () ) ;
  setEdgeFlatten   ( src -> getEdgeFlatten   () ) ;
} 


ssgBase *ssgaWaveSystem::clone ( int clone_flags )
{
  ssgaWaveSystem *b = new ssgaWaveSystem ( getNumTris() ) ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgaWaveSystem::ssgaWaveSystem ( int np ) : ssgaShape ( np )
{
  type=ssgaTypeWaveSystem ();

  setDepthCallback ( NULL ) ;
  setWindSpeed     ( 1.0f ) ;
  setWindDirn      ( 0.0f ) ;
  setEdgeFlatten   ( 0.0f ) ;

  nstrips = nstacks = 0 ;

  normals   = NULL ;
  colours   = NULL ;
  texcoords = NULL ;
  vertices  = NULL ;
  orig_vertices  = NULL ;

  tu = tv = 1.0f ;

  for ( int i = 0 ; i < SSGA_MAX_WAVETRAIN ; i++ )
    train [ i ] = NULL ;

  regenerate();
}


ssgaWaveSystem::~ssgaWaveSystem (void) {}

const char *ssgaWaveSystem::getTypeName(void) { return "ssgaWaveSystem" ; }


void ssgaWaveSystem::regenerate ()
{
  delete[] normals   ;
  delete[] colours   ;
  delete[] texcoords ;
  delete[] vertices  ;
  delete[] orig_vertices  ;

  normals   = NULL ;
  colours   = NULL ;
  texcoords = NULL ;
  vertices  = NULL ;
  orig_vertices  = NULL ;

  nstrips = nstacks = 0 ;

  if ( kidState != NULL ) kidState -> ref () ;
  removeAllKids () ;
  if ( kidState != NULL ) kidState -> deRef () ;

  if ( ntriangles <= 0 )
    return ;

  int gridSize = (int) sqrt ( (float) ntriangles / 2.0f ) ;
 
  nstacks = gridSize ;
  nstrips = gridSize ;

  if ( nstacks < 1 ) nstacks = 1 ;
  if ( nstrips < 1 ) nstrips = 1 ;

  normals   = new sgVec3 [ (nstacks+1) * (nstrips+1) ] ;
  colours   = new sgVec4 [ (nstacks+1) * (nstrips+1) ] ;
  texcoords = new sgVec2 [ (nstacks+1) * (nstrips+1) ] ;
  vertices  = new sgVec3 [ (nstacks+1) * (nstrips+1) ] ;
  orig_vertices = new sgVec3 [ (nstacks+1) * (nstrips+1) ] ;

  int i;
  for ( i = 0 ; i <= nstrips ; i++ )
    for ( int j = 0 ; j <= nstacks ; j++ )
    {
      int idx = i * (nstrips+1) + j ;

      float x = (float) j / (float) nstacks ;
      float y = (float) i / (float) nstrips ;

      if ( j == 0 ) x = -500.0f ;
      if ( j == nstacks ) x = 500.0f ;
      if ( i == 0 ) y = -500.0f ;
      if ( i == nstacks ) y = 500.0f ;

      sgSetVec3  ( vertices [idx], (x-0.5f) * size[0],
                                   (y-0.5f) * size[1], 0.0f ) ;
      sgSetVec3  ( normals  [idx], 0.0f, 0.0f, 1.0f ) ;
      sgSetVec2  ( texcoords[idx], x * tu, y * tv ) ;
      sgCopyVec4 ( colours  [idx], colour ) ;
      sgCopyVec3 ( orig_vertices [ idx ], vertices [idx] ) ;
    }

  for ( i = 0 ; i < nstrips ; i++ )
  {
    ssgVtxTable      *vt = new ssgVtxTable ;
    ssgVertexArray   *vv = new ssgVertexArray   ( nstacks * 2 + 2 ) ;
    ssgNormalArray   *nn = new ssgNormalArray   ( nstacks * 2 + 2 ) ;
    ssgColourArray   *cc = new ssgColourArray   ( nstacks * 2 + 2 ) ;
    ssgTexCoordArray *tt = new ssgTexCoordArray ( nstacks * 2 + 2 ) ;

    addKid ( vt ) ;

    vt -> setState    ( getKidState () ) ;
    vt -> setCallback ( SSG_CALLBACK_PREDRAW , getKidPreDrawCB  () ) ;
    vt -> setCallback ( SSG_CALLBACK_POSTDRAW, getKidPostDrawCB () ) ;

    vt -> setPrimitiveType ( GL_TRIANGLE_STRIP ) ;

    for ( int j = 0 ; j < nstacks + 1 ; j++ )
    {
      int idx = (i+1) * (nstrips+1) + j ;

      vv -> add ( vertices [ idx ] ) ; nn -> add ( normals  [ idx ] ) ;
      cc -> add ( colours  [ idx ] ) ; tt -> add ( texcoords[ idx ] ) ;

      idx = i * (nstrips+1) + j ;

      vv -> add ( vertices [ idx ] ) ; nn -> add ( normals  [ idx ] ) ;
      cc -> add ( colours  [ idx ] ) ; tt -> add ( texcoords[ idx ] ) ;
    }

    vt -> setVertices  ( vv ) ;
    vt -> setNormals   ( nn ) ;
    vt -> setColours   ( cc ) ;
    vt -> setTexCoords ( tt ) ;

    vt -> recalcBSphere () ;
  }

  recalcBSphere () ;
}


// XXX really need these (and ssgLocal.h is not accessible):
extern int _ssgLoadObject ( FILE *, ssgBase **, int ) ;
extern int _ssgSaveObject ( FILE *, ssgBase * ) ;


#define load_field(fp, name) (fread(&(name), 1, sizeof(name), fp) == sizeof(name))
#define save_field(fp, name) (fwrite(&(name), 1, sizeof(name), fp) == sizeof(name))


int ssgaWaveSystem::load ( FILE *fp )
{
   return ( load_field ( fp, windSpeed ) &&
            load_field ( fp, windHeading ) &&
            load_field ( fp, edgeFlatten ) &&
            load_field ( fp, tu ) &&
            load_field ( fp, tv ) &&
	    ssgaShape::load ( fp ) ) ;
}



int ssgaWaveSystem::save ( FILE *fp )
{
   return ( save_field ( fp, windSpeed ) &&
            save_field ( fp, windHeading ) &&
            save_field ( fp, edgeFlatten ) &&
            save_field ( fp, tu ) &&
            save_field ( fp, tv ) &&
	    ssgaShape::save ( fp ) ) ;
}



