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

*/

#include "ssgAux.h"

#undef _SSG_PUBLIC
#include "../ssg/ssgLocal.h"

/*
  Lens flares are an effect that happens in the
  lens - your eye or a camera lens.  As such, they
  are completely invisible if the light itself is
  not visible - which should mean that if the light
  is culled then so is the lens flare.  This suggests
  that the bsphere center should be at the light point
  and the radius should be tiny.

  However, if you have a multi-facetted display,
  pretending to be a single display rather than
  a lot of separate 'cameras' then you may want to
  set the SSGA_CONTINUOUS_DISPLAY_SURFACE flag.

  This should probably be realtime settable - but
  to do it 'right', you'd need to know which edges
  of the screen were 'shared' with other screens
  and only allow the lens flare to cross those.
*/

// #define CONTINUOUS_DISPLAY_SURFACE 1




sgMat4 _ssgaIdentity =
{
  {  1.0f,  0.0f,  0.0f,  0.0f },
  {  0.0f,  1.0f,  0.0f,  0.0f },
  {  0.0f,  0.0f,  1.0f,  0.0f },
  {  0.0f,  0.0f,  0.0f,  1.0f }
} ;                                                                             

/*
  This is based on code written by Stephen Coy - subsequently
  re-written by Mark Kilgard - and now re-re-written by me.
  Actually, only the original numbers for the dimensions of
  the flare planes are from the original code.  All the rest
  is a complete rewrite because it has to work within a scene
  graph API.
*/

struct _ssgaFlarePlane
{
  int     type  ;    /* flare texture index, 0..5 */
  float   loc   ;    /* postion on flare_axis */
  float   scale ;
  sgVec4  color ;
} ;


static _ssgaFlarePlane flare [] =
{
  { -1,  0.00f, 0.20f, {  0  , 0, 1, 1 }},
  { -1,  0.00f, 0.10f, {  0  , 1, 0, 1 }},
  { -1,  0.00f, 0.12f, { 1.0f, 0, 0, 1 }},
  {  1,  0.50f, 0.20f, { 0.3f, 0, 0, 1 }},
  {  2, -0.20f, 0.04f, { 0.6f, 0, 0, 1 }},
  {  3,  0.00f, 0.10f, { 0.4f, 0, 0, 1 }},
  {  3,  0.30f, 0.03f, { 0.3f, 0, 0, 1 }},
  {  3,  0.80f, 0.05f, { 0.3f, 0, 0, 1 }},
  {  0,  1.00f, 0.04f, { 0.3f, 0, 0, 1 }},
  {  4,  1.25f, 0.07f, { 0.5f, 0, 0, 1 }},
  {  5,  1.40f, 0.02f, { 0.6f, 0, 0, 1 }},
  {  5,  1.60f, 0.04f, { 0.4f, 0, 0, 1 }},
  {  5,  2.00f, 0.03f, { 0.2f, 0, 0, 1 }},
  { -999, 0, 0, { 0, 0, 0, 0 } }  /* End marker */
} ;


static float shineTexCoords [ 12 ][ 4 ][ 2 ] =
{
  { { 0.00, 0.00 }, { 0.25, 0.00 }, { 0.25, 0.50 }, { 0.00, 0.50 } },
  { { 0.25, 0.00 }, { 0.50, 0.00 }, { 0.50, 0.50 }, { 0.25, 0.50 } },
  { { 0.50, 0.00 }, { 0.75, 0.00 }, { 0.75, 0.50 }, { 0.50, 0.50 } },
  { { 0.00, 0.50 }, { 0.25, 0.50 }, { 0.25, 0.00 }, { 0.00, 0.00 } },
  { { 0.25, 0.50 }, { 0.50, 0.50 }, { 0.50, 0.00 }, { 0.25, 0.00 } },
  { { 0.50, 0.50 }, { 0.75, 0.50 }, { 0.75, 0.00 }, { 0.50, 0.00 } },
  { { 0.25, 0.00 }, { 0.00, 0.00 }, { 0.00, 0.50 }, { 0.25, 0.50 } },
  { { 0.50, 0.00 }, { 0.25, 0.00 }, { 0.25, 0.50 }, { 0.50, 0.50 } },
  { { 0.75, 0.00 }, { 0.50, 0.00 }, { 0.50, 0.50 }, { 0.75, 0.50 } },
  { { 0.25, 0.50 }, { 0.00, 0.50 }, { 0.00, 0.00 }, { 0.25, 0.00 } },
  { { 0.50, 0.50 }, { 0.25, 0.50 }, { 0.25, 0.00 }, { 0.50, 0.00 } },
  { { 0.75, 0.50 }, { 0.50, 0.50 }, { 0.50, 0.00 }, { 0.75, 0.00 } }
} ;


static float flareTexCoords [ 6 ][ 4 ][ 2 ] =
{
  { { 0.00f, 0.50f }, { 0.25f, 0.50f }, { 0.25f, 1.00f }, { 0.00f, 1.00f } },
  { { 0.25f, 0.50f }, { 0.50f, 0.50f }, { 0.50f, 1.00f }, { 0.25f, 1.00f } },
  { { 0.27f, 0.70f }, { 0.48f, 0.70f }, { 0.48f, 0.80f }, { 0.27f, 0.80f } },
  { { 0.50f, 0.50f }, { 0.75f, 0.50f }, { 0.75f, 1.00f }, { 0.50f, 1.00f } },
  { { 0.75f, 0.50f }, { 1.00f, 0.50f }, { 1.00f, 1.00f }, { 0.75f, 1.00f } },
  { { 0.75f, 0.00f }, { 1.00f, 0.00f }, { 1.00f, 0.50f }, { 0.75f, 0.50f } }
} ;


void ssgaLensFlare::copy_from ( ssgaLensFlare *src, int clone_flags )
{
  ssgaShape::copy_from ( src, clone_flags ) ;
}

ssgBase *ssgaLensFlare::clone ( int clone_flags )
{
  ssgaLensFlare *b = new ssgaLensFlare ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgaLensFlare::ssgaLensFlare (void):ssgaShape ()
{
  type = ssgaTypeLensFlare () ;
  regenerate () ;
}

ssgaLensFlare::ssgaLensFlare (int nt):ssgaShape (nt)
{
  type = ssgaTypeLensFlare () ;
  regenerate () ;
}

ssgaLensFlare::~ssgaLensFlare (void) {}

static ssgSimpleState *flareState   = NULL ;
static ssgTexture     *flareTexture = NULL ;

const char *ssgaLensFlare::getTypeName(void) { return "ssgaLensFlare" ; }

static int preDraw ( ssgEntity * )
{
  glDisable   ( GL_DEPTH_TEST  ) ;
  glBlendFunc ( GL_ONE, GL_ONE ) ;
  return TRUE ;
}


static int postDraw ( ssgEntity * )
{
  glEnable    ( GL_DEPTH_TEST  ) ;
  glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;
  return TRUE ;
}


void ssgaLensFlare::regenerate ()
{
  if ( kidState != NULL ) kidState -> ref () ;
  removeAllKids () ;
  if ( kidState != NULL ) kidState -> deRef () ;
 
  for ( ntriangles = 0 ; flare[ntriangles/2].type >= -1 ; ntriangles += 2 )
    /* Count the number of triangles */ ;

  vt = new ssgVtxTable () ;
  v0 = new ssgVertexArray   ( 2 * ntriangles ) ;
  n0 = new ssgNormalArray   ( 2 * ntriangles ) ;
  c0 = new ssgColourArray   ( 2 * ntriangles ) ;
  t0 = new ssgTexCoordArray ( 2 * ntriangles ) ;
 
  vt -> setPrimitiveType ( GL_QUADS ) ;
 
  addKid ( vt ) ;
 
  if ( flareState == NULL )
  {
    /*
      The texture constructor deletes the texture from
      main memory after it's finished with it!!  YIKES!!
    */

    unsigned char *t = new unsigned char [ 256 * 128 ] ;
    memcpy ( t, _ssgaGetLensFlareTexture(), 256 * 128 ) ;

    flareTexture = new ssgTexture ( "NONE", t, 256, 128, 1 ) ;

    flareState = new ssgSimpleState () ;
    flareState -> setTexture        ( flareTexture ) ;
    flareState -> setTranslucent    () ;
    flareState -> enable            ( GL_TEXTURE_2D ) ;
    flareState -> enable            ( GL_BLEND ) ;
    flareState -> disable           ( GL_LIGHTING ) ;
    flareState -> ref () ;
    setKidState ( flareState ) ;
  }


  vt -> setState    ( getKidState () ) ;
  vt -> setCallback ( SSG_CALLBACK_PREDRAW , preDraw  /* getKidPreDrawCB */ );
  vt -> setCallback ( SSG_CALLBACK_POSTDRAW, postDraw /* getKidPostDrawCB*/ );
 
  sgVec3 v ; sgVec3 n ; sgVec2 t ; sgVec4 c ;
 
  for ( int i = 0 ; i < ntriangles * 2 ; i++ )
  {
    sgSetVec3 ( v, 0,  0, 0 ) ; v0->add ( v ) ;
    sgSetVec3 ( n, 0, -1, 0 ) ; n0->add ( n ) ;
    sgSetVec4 ( c, 1,1,1,1  ) ; c0->add ( c ) ;
    sgSetVec2 ( t, 0, 0     ) ; t0->add ( t ) ;
  }
 
  vt -> setVertices  ( v0 ) ;
  vt -> setNormals   ( n0 ) ;
  vt -> setColours   ( c0 ) ;
  vt -> setTexCoords ( t0 ) ;

#ifndef CONTINUOUS_DISPLAY_SURFACE
  vt -> getBSphere () -> setCenter ( 0, 0, 0 ) ;
  vt -> getBSphere () -> setRadius ( 0.1f ) ;
        getBSphere () -> setCenter ( 0, 0, 0 ) ;
        getBSphere () -> setRadius ( 0.1f ) ;
#else
  vt -> recalcBSphere () ;
	recalcBSphere () ;
#endif
}


void ssgaLensFlare::update ( sgMat4 mat )
{
  static int shine_tic = 0 ;

  sgVec3 flare_axis ;

  float *texCoords ;

  sgVec3 light_pos ;
  float  znear ;

  ssgGetNearFar ( & znear, NULL ) ;
  sgNormalizeVec3 ( light_pos, mat[3] ) ;
  sgScaleVec3 ( light_pos, znear * 2.0f ) ;  /* To avoid being near-clipped */

  sgSetVec3 ( flare_axis, -light_pos[0], -light_pos[1], 0.0 ) ;

  int vv = 0 ;

  for ( int i = 0 ; flare[ i ].type >= -1 ; i++ )
  {
    sgVec3 position ;
    float sz ;

    sz = flare[i].scale * znear * 2.0f ;

    sgAddScaledVec3 ( position, light_pos, flare_axis, flare[i].loc ) ;

    if ( flare[i].type < 0 )
    {
      shine_tic = (shine_tic + 1) % 12 ;

      texCoords = & ( shineTexCoords [ shine_tic ][ 0 ][ 0 ] ) ;
    }
    else
      texCoords = & ( flareTexCoords [ flare[i].type ][ 0 ][ 0 ] ) ;

    sgVec3 vx ;

    sgSetVec3 ( vx, position[0]+sz, position[1]-sz, position[2] ) ; 
    c0 -> set ( flare[i].color, vv ) ;
    t0 -> set ( & texCoords[0], vv ) ;
    v0 -> set ( vx, vv++ ) ;

    sgSetVec3 ( vx, position[0]+sz, position[1]+sz, position[2] ) ; 
    c0 -> set ( flare[i].color, vv ) ;
    t0 -> set ( & texCoords[2], vv ) ;
    v0 -> set ( vx, vv++ ) ;

    sgSetVec3 ( vx, position[0]-sz, position[1]+sz, position[2] ) ; 
    c0 -> set ( flare[i].color, vv ) ;
    t0 -> set ( & texCoords[4], vv ) ;
    v0 -> set ( vx, vv++ ) ;

    sgSetVec3 ( vx, position[0]-sz, position[1]-sz, position[2] ) ; 
    c0 -> set ( flare[i].color, vv ) ;
    t0 -> set ( & texCoords[6], vv ) ;
    v0 -> set ( vx, vv++ ) ;
  }
}


void ssgaLensFlare::cull ( sgFrustum *f, sgMat4 m, int /* test_needed */ )
{
  /*
    Lens flares are best generated in eye-space (because they actually
    occur inside the lens of your eye).  Hence we call 'update' with
    the current modelview matrix - it generates the geometry in eye-space -
    which means that we have to render it with the identity matrix on the
    stack.
  */

  update ( m ) ;

  _ssgPushMatrix ( _ssgaIdentity ) ;
 
  glPushMatrix   () ;
  glLoadMatrixf  ( (float *) _ssgaIdentity ) ;
  ssgBranch::cull ( f, _ssgaIdentity, FALSE ) ;
  glPopMatrix () ;
 
  _ssgPopMatrix  () ;                                                           
}

