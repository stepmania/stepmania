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

typedef float sgVec9 [9] ;

void ssgaPatch::regenerate ()
{
  if ( kidState != NULL ) kidState -> ref () ;
  removeAllKids () ;
  if ( kidState != NULL ) kidState -> deRef () ;

  if ( ntriangles <=       2 ) levels = -1 ; else
  if ( ntriangles <=      18 ) levels =  0 ; else
  if ( ntriangles <=    4*18 ) levels =  1 ; else
  if ( ntriangles <=   16*18 ) levels =  2 ; else
  if ( ntriangles <=   64*18 ) levels =  3 ; else
  if ( ntriangles <=  256*18 ) levels =  4 ; else
  if ( ntriangles <= 1024*18 ) levels =  5 ; else
  if ( ntriangles <= 4096*18 ) levels =  6 ; else levels =  7 ;

  if ( ntriangles == 0 || control_points == NULL )
    return ;

  makePatch ( control_points, levels ) ;

  /*
  vt0 -> setState    ( getKidState () ) ;
  vt0 -> setCallback ( SSG_CALLBACK_PREDRAW , getKidPreDrawCB  () ) ;
  vt0 -> setCallback ( SSG_CALLBACK_POSTDRAW, getKidPostDrawCB () ) ;
  vt0 -> recalcBSphere () ;
  */

  recalcBSphere () ;
}


static float *sgMidPointVec9 ( sgVec9 a, sgVec9 b )
{
  static sgVec9 r ;

  r[0] = (a[0] + b[0])/2.0f ;
  r[1] = (a[1] + b[1])/2.0f ;
  r[2] = (a[2] + b[2])/2.0f ;
  r[3] = (a[3] + b[3])/2.0f ;
  r[4] = (a[4] + b[4])/2.0f ;
  r[5] = (a[5] + b[5])/2.0f ;
  r[6] = (a[6] + b[6])/2.0f ;
  r[7] = (a[7] + b[7])/2.0f ;
  r[8] = (a[8] + b[8])/2.0f ;

  return r ;
}
 
 
void ssgaPatch::makeHSpline ( sgVec9 points[4], sgVec9 newVts[7] )
{
  sgVec9 temp ;
 
  memcpy ( newVts[0], points[0],sizeof(sgVec9));
  memcpy ( newVts[1], sgMidPointVec9 ( points[0], points[1] ),sizeof(sgVec9));
  memcpy ( temp     , sgMidPointVec9 ( points[1], points[2] ),sizeof(sgVec9));
  memcpy ( newVts[2], sgMidPointVec9 ( newVts[1], temp      ),sizeof(sgVec9));
  memcpy ( newVts[5], sgMidPointVec9 ( points[2], points[3] ),sizeof(sgVec9));
  memcpy ( newVts[4], sgMidPointVec9 ( temp     , newVts[5] ),sizeof(sgVec9));
  memcpy ( newVts[3], sgMidPointVec9 ( newVts[2], newVts[4] ),sizeof(sgVec9));
  memcpy ( newVts[6], points[3],sizeof(sgVec9));
}
 
 
void ssgaPatch::makeVSplines ( sgVec9 hv[4][7],  sgVec9 nv[7][7]  )
{
  sgVec9 temp ;
 
  for ( int col = 0 ; col < 7 ; col++ )
  {
    memcpy ( nv[0][col], hv[0][col], sizeof(sgVec9) ) ;
    memcpy ( nv[1][col],sgMidPointVec9(hv[0][col], hv[1][col]), sizeof(sgVec9));
    memcpy ( temp      ,sgMidPointVec9(hv[1][col], hv[2][col]), sizeof(sgVec9));
    memcpy ( nv[2][col],sgMidPointVec9(nv[1][col], temp      ), sizeof(sgVec9));
    memcpy ( nv[5][col],sgMidPointVec9(hv[2][col], hv[3][col]), sizeof(sgVec9));
    memcpy ( nv[4][col],sgMidPointVec9(temp      , nv[5][col]), sizeof(sgVec9));
    memcpy ( nv[3][col],sgMidPointVec9(nv[2][col], nv[4][col]), sizeof(sgVec9));
    memcpy ( nv[6][col], hv[3][col], sizeof(sgVec9) ) ;
  }
}                                                                               


void ssgaPatch::writePatch ( sgVec9 points[4][4] )
{
  sgVec3 nn = { 0,0,1 } ;

  /*
    Write three strips of 8 vertices each
  */

  for ( int i = 0 ; i < 3 ; i++ )
  {
    ssgVertexArray   *va = new ssgVertexArray   ( 8 ) ;
    ssgTexCoordArray *ta = new ssgTexCoordArray ( 8 ) ;
    ssgColourArray   *ca = new ssgColourArray   ( 8 ) ;
    ssgNormalArray   *na = new ssgNormalArray   ( 8 ) ;
 
    for ( int j = 0 ; j < 4 ; j++ )
    {
      sgVec3 vv ;

      sgSetVec3 ( vv, points [ i ][j][0] * size[0] + center[0],
                      points [ i ][j][1] * size[1] + center[1],
                      points [ i ][j][2] * size[2] + center[2] ) ;

      va -> add ( vv ) ;
      ta -> add ( & ( points [ i ][j][ 3 ] ) ) ;
      ca -> add ( & ( points [ i ][j][ 5 ] ) ) ;
      na -> add ( nn ) ;

      sgSetVec3 ( vv, points [i+1][j][0] * size[0] + center[0],
                      points [i+1][j][1] * size[1] + center[1],
                      points [i+1][j][2] * size[2] + center[2] ) ;

      va -> add ( vv ) ;
      ta -> add ( & ( points [i+1][j][ 3 ] ) ) ;
      ca -> add ( & ( points [i+1][j][ 5 ] ) ) ;
      na -> add ( nn ) ;
    }
 
    ssgVtxTable *vt = new ssgVtxTable ( GL_TRIANGLE_STRIP, va,na,ta,ca ) ;
    vt -> setState ( getKidState () ) ;
    vt -> setCallback ( SSG_CALLBACK_PREDRAW , getKidPreDrawCB  () ) ;
    vt -> setCallback ( SSG_CALLBACK_POSTDRAW, getKidPostDrawCB () ) ;

    addKid ( vt ) ;
  }
}



void ssgaPatch::makePatch ( float points[4][4][9], int level )
{
  if ( level <= 0 )
  {
    writePatch ( points ) ;
    return ;
  }
 
  sgVec9 newHVerts   [ 4 ][ 7 ] ;
  sgVec9 newVertices [ 7 ][ 7 ] ;
 
  makeHSpline ( points[0], newHVerts[0] ) ;
  makeHSpline ( points[1], newHVerts[1] ) ;
  makeHSpline ( points[2], newHVerts[2] ) ;
  makeHSpline ( points[3], newHVerts[3] ) ;
 
  makeVSplines ( newHVerts, newVertices ) ;
 
  sgVec9 patch[4][4] ;
  int i ;
 
  for ( i = 0 ; i < 16 ; i++ )
   memcpy ( patch[i>>2][i&3], newVertices[i>>2][i&3], 9*sizeof(float) ) ;
 
  makePatch ( patch, level-1 ) ;
 
  for ( i = 0 ; i < 16 ; i++ )
   memcpy ( patch[i>>2][i&3], newVertices[i>>2][3+(i&3)], 9*sizeof(float) ) ;
 
  makePatch ( patch, level-1 ) ;
 
  for ( i = 0 ; i < 16 ; i++ )
   memcpy ( patch[i>>2][i&3], newVertices[3+(i>>2)][i&3], 9*sizeof(float) ) ;
 
  makePatch ( patch, level-1 ) ;
 
  for ( i = 0 ; i < 16 ; i++ )
   memcpy ( patch[i>>2][i&3], newVertices[3+(i>>2)][3+(i&3)], 9*sizeof(float));
 
  makePatch ( patch, level-1 ) ;
}


void ssgaPatch::setControlPoint ( int s, int t,
                         float x, float y, float z,
                         float u, float v,
                         float r, float g, float b, float a )
{
  control_points[s][t][0] = x ;
  control_points[s][t][1] = y ;
  control_points[s][t][2] = z ;
  control_points[s][t][3] = u ;
  control_points[s][t][4] = v ;
  control_points[s][t][5] = r ;
  control_points[s][t][6] = g ;
  control_points[s][t][7] = b ;
  control_points[s][t][8] = a ;
}


void ssgaPatch::setControlPoint ( int s, int t,
                                  sgVec3 xyz, sgVec2 uv, sgVec4 rgba )
{
  memcpy ( &(control_points[s][t][0]), xyz , sizeof(sgVec3) ) ;
  memcpy ( &(control_points[s][t][3]), uv  , sizeof(sgVec2) ) ;
  memcpy ( &(control_points[s][t][5]), rgba, sizeof(sgVec4) ) ;
}


void ssgaPatch::getControlPoint ( int s, int t,
                                  sgVec3 xyz, sgVec2 uv, sgVec4 rgba )
{
  memcpy ( xyz,  &(control_points[s][t][0]), sizeof(sgVec3) ) ;
  memcpy ( uv ,  &(control_points[s][t][3]), sizeof(sgVec2) ) ;
  memcpy ( rgba, &(control_points[s][t][5]), sizeof(sgVec4) ) ;
}


