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

static int patch [][ 17 ] =
{ 
  /* Rim: */
    { 2, 102, 103, 104, 105,
        4,   5,   6,   7,
        8,   9,  10,  11,
       12,  13,  14,  15 },
  /* Body: */
    {  2, 12,  13,  14,  15,
       16,  17,  18,  19,
       20,  21,  22,  23,
       24,  25,  26,  27 },
    {  2, 24,  25,  26,  27,
       29,  30,  31,  32,
       33,  34,  35,  36,
       37,  38,  39,  40 },
  /* Lid: */
    {  2, 96,  96,  96,  96,
       97,  98,  99, 100,
      101, 101, 101, 101,
        0,   1,   2,   3 },
    {  2,  0,   1,   2,   3,
      106, 107, 108, 109,
      110, 111, 112, 113,
      114, 115, 116, 117 },
  /* Handle: */
    {  1, 41,  42,  43,  44,
       45,  46,  47,  48,
       49,  50,  51,  52,
       53,  54,  55,  56 },
    {  1, 53,  54,  55,  56,
       57,  58,  59,  60,
       61,  62,  63,  64,
       28,  65,  66,  67 },
  /* Spout: */
    {  1, 68,  69,  70,  71,
       72,  73,  74,  75,
       76,  77,  78,  79,
       80,  81,  82,  83 },
    {  1, 80,  81,  82,  83,
       84,  85,  86,  87,
       88,  89,  90,  91,
       92,  93,  94,  95 },
  /* Bottom: */
    { 2, 118, 118, 118, 118,
      124, 122, 119, 121,
      123, 126, 125, 120,
       40,  39,  38,  37 },

  /* End Marker: */
    { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }
} ; 
 
static sgVec3 vertex [] =
{
  {  0.2000f,  0.0000f, 2.70000f },  /* 00 */
  {  0.2000f, -0.1120f, 2.70000f },
  {  0.1120f, -0.2000f, 2.70000f },
  {  0.0000f, -0.2000f, 2.70000f },
  {  1.3375f,  0.0000f, 2.53125f },
  {  1.3375f, -0.7490f, 2.53125f },
  {  0.7490f, -1.3375f, 2.53125f },
  {  0.0000f, -1.3375f, 2.53125f },
  {  1.4375f,  0.0000f, 2.53125f },
  {  1.4375f, -0.8050f, 2.53125f },

  {  0.8050f, -1.4375f, 2.53125f },  /* 10 */
  {  0.0000f, -1.4375f, 2.53125f },
  {  1.5000f,  0.0000f, 2.40000f },
  {  1.5000f, -0.8400f, 2.40000f },
  {  0.8400f, -1.5000f, 2.40000f },
  {  0.0000f, -1.5000f, 2.40000f },
  {  1.7500f,  0.0000f, 1.87500f },
  {  1.7500f, -0.9800f, 1.87500f },
  {  0.9800f, -1.7500f, 1.87500f },
  {  0.0000f, -1.7500f, 1.87500f },

  {  2.0000f,  0.0000f, 1.35000f },  /* 20 */
  {  2.0000f, -1.1200f, 1.35000f },
  {  1.1200f, -2.0000f, 1.35000f },
  {  0.0000f, -2.0000f, 1.35000f },
  {  2.0000f,  0.0000f, 0.90000f },
  {  2.0000f, -1.1200f, 0.90000f },
  {  1.1200f, -2.0000f, 0.90000f },
  {  0.0000f, -2.0000f, 0.90000f },
  { -2.0000f,  0.0000f, 0.90000f },
  {  2.0000f,  0.0000f, 0.45000f },
  
  {  2.0000f, -1.1200f, 0.45000f },  /* 30 */
  {  1.1200f, -2.0000f, 0.45000f },
  {  0.0000f, -2.0000f, 0.45000f },
  {  1.5000f,  0.0000f, 0.22500f },
  {  1.5000f, -0.8400f, 0.22500f },
  {  0.8400f, -1.5000f, 0.22500f },
  {  0.0000f, -1.5000f, 0.22500f },
  {  1.5000f,  0.0000f, 0.15000f },
  {  1.5000f, -0.8400f, 0.15000f },
  {  0.8400f, -1.5000f, 0.15000f },
  
  {  0.0000f, -1.5000f, 0.15000f },  /* 40 */
  { -1.6000f,  0.0000f, 2.02500f },
  { -1.6000f, -0.3000f, 2.02500f },
  { -1.5000f, -0.3000f, 2.25000f },
  { -1.5000f,  0.0000f, 2.25000f },
  { -2.3000f,  0.0000f, 2.02500f },
  { -2.3000f, -0.3000f, 2.02500f },
  { -2.5000f, -0.3000f, 2.25000f },
  { -2.5000f,  0.0000f, 2.25000f },
  { -2.7000f,  0.0000f, 2.02500f },

  { -2.7000f, -0.3000f, 2.02500f },  /* 50 */
  { -3.0000f, -0.3000f, 2.25000f },
  { -3.0000f,  0.0000f, 2.25000f },
  { -2.7000f,  0.0000f, 1.80000f },
  { -2.7000f, -0.3000f, 1.80000f },
  { -3.0000f, -0.3000f, 1.80000f },
  { -3.0000f,  0.0000f, 1.80000f },
  { -2.7000f,  0.0000f, 1.57500f },
  { -2.7000f, -0.3000f, 1.57500f },
  { -3.0000f, -0.3000f, 1.35000f },
  
  { -3.0000f,  0.0000f, 1.35000f },  /* 60 */
  { -2.5000f,  0.0000f, 1.12500f },
  { -2.5000f, -0.3000f, 1.12500f },
  { -2.6500f, -0.3000f, 0.93750f },
  { -2.6500f,  0.0000f, 0.93750f },
  { -2.0000f, -0.3000f, 0.90000f },
  { -1.9000f, -0.3000f, 0.60000f },
  { -1.9000f,  0.0000f, 0.60000f },
  {  1.7000f,  0.0000f, 1.42500f },
  {  1.7000f, -0.6600f, 1.42500f },

  {  1.7000f, -0.6600f, 0.60000f },  /* 70 */
  {  1.7000f,  0.0000f, 0.60000f },
  {  2.6000f,  0.0000f, 1.42500f },
  {  2.6000f, -0.6600f, 1.42500f },
  {  3.1000f, -0.6600f, 0.82500f },
  {  3.1000f,  0.0000f, 0.82500f },
  {  2.3000f,  0.0000f, 2.10000f },
  {  2.3000f, -0.2500f, 2.10000f },
  {  2.4000f, -0.2500f, 2.02500f },
  {  2.4000f,  0.0000f, 2.02500f },

  {  2.7000f,  0.0000f, 2.40000f },  /* 80 */
  {  2.7000f, -0.2500f, 2.40000f },
  {  3.3000f, -0.2500f, 2.40000f },
  {  3.3000f,  0.0000f, 2.40000f },
  {  2.8000f,  0.0000f, 2.47500f },
  {  2.8000f, -0.2500f, 2.47500f },
  {  3.5250f, -0.2500f, 2.49375f },
  {  3.5250f,  0.0000f, 2.49375f },
  {  2.9000f,  0.0000f, 2.47500f },
  {  2.9000f, -0.1500f, 2.47500f },
  
  {  3.4500f, -0.1500f, 2.51250f },  /* 90 */
  {  3.4500f,  0.0000f, 2.51250f },
  {  2.8000f,  0.0000f, 2.40000f },
  {  2.8000f, -0.1500f, 2.40000f },
  {  3.2000f, -0.1500f, 2.40000f },
  {  3.2000f,  0.0000f, 2.40000f },
  {  0.0000f,  0.0000f, 3.15000f },
  {  0.8000f,  0.0000f, 3.15000f },
  {  0.8000f, -0.4500f, 3.15000f },
  {  0.4500f, -0.8000f, 3.15000f },

  {  0.0000f, -0.8000f, 3.15000f },  /* 100 */
  {  0.0000f,  0.0000f, 2.85000f },
  {  1.4000f,  0.0000f, 2.40000f },
  {  1.4000f, -0.7840f, 2.40000f },
  {  0.7840f, -1.4000f, 2.40000f },
  {  0.0000f, -1.4000f, 2.40000f },
  {  0.4000f,  0.0000f, 2.55000f },
  {  0.4000f, -0.2240f, 2.55000f },
  {  0.2240f, -0.4000f, 2.55000f },
  {  0.0000f, -0.4000f, 2.55000f },

  {  1.3000f,  0.0000f, 2.55000f },  /* 110 */
  {  1.3000f, -0.7280f, 2.55000f },
  {  0.7280f, -1.3000f, 2.55000f },
  {  0.0000f, -1.3000f, 2.55000f }, /* Next four verts kludged to make lid fit */
  {  1.4000f,  0.0000f, 2.40000f }, /*  {  1.3000,  0.0000, 2.40000 }, */
  {  1.4000f, -0.7840f, 2.40000f }, /*  {  1.3000, -0.7280, 2.40000 }, */
  {  0.7840f, -1.4000f, 2.40000f }, /*  {  0.7280, -1.3000, 2.40000 }, */
  {  0.0000f, -1.4000f, 2.40000f }, /*  {  0.0000, -1.3000, 2.40000 }, */
  {  0.0000f,  0.0000f, 0.00000f },
  {  1.4250f, -0.7980f, 0.00000f },

  {  1.5000f,  0.0000f, 0.07500f },  /* 120 */
  {  1.4250f,  0.0000f, 0.00000f },
  {  0.7980f, -1.4250f, 0.00000f },
  {  0.0000f, -1.5000f, 0.07500f },
  {  0.0000f, -1.4250f, 0.00000f },
  {  1.5000f, -0.8400f, 0.07500f },
  {  0.8400f, -1.5000f, 0.07500f }
} ;


void ssgaTeapot::regenerate ()
{
  if ( kidState != NULL ) kidState -> ref () ;
  removeAllKids () ;
  if ( kidState != NULL ) kidState -> deRef () ;

  for ( int i = 0 ; patch[i][0] >= 0 ; i++ )
  {
    ssgaPatch *p = new ssgaPatch ( ntriangles / 32 ) ;
    int j ;

    for ( j = 0 ; j < 16 ; j++ )
    {
      sgVec3 xyz ;
      sgVec4 rgba = { 1,1,1,1 } ;
      sgVec2 uv ;

      uv [ 0 ] = (float)(j&3)/3.0f ;
      uv [ 1 ] = (float)(j>>2)/3.0f ;

      sgScaleVec3 ( xyz, vertex[patch[i][j+1]], 1.0f/2.5f ) ;
      xyz [ 0 ] *= -1.0f ;

      p -> setControlPoint ( j>>2, j&3, xyz, uv, rgba ) ;
    }

    p -> setKidState ( getKidState () ) ;
    p -> setKidCallback ( SSG_CALLBACK_PREDRAW , getKidPreDrawCB  () ) ;
    p -> setKidCallback ( SSG_CALLBACK_POSTDRAW, getKidPostDrawCB () ) ;
    p -> setColour  ( colour ) ;
    p -> setCenter  ( getCenter () ) ;
    p -> setSize    ( getSize   () ) ;
    p -> regenerate () ;

    /* Make a mirror image in the Y axis */

    sgVec3 xyz ;
    sgVec2 uv ;
    sgVec4 rgba ;

    ssgaPatch *p2 = new ssgaPatch ( ntriangles / 32 ) ;

    for ( j = 0 ; j < 16 ; j++ )
    {
      p  -> getControlPoint ( j>>2, 3-(j&3), xyz, uv, rgba ) ; 
      xyz [ 1 ] *= -1.0f ;
      p2 -> setControlPoint ( j>>2, j&3, xyz, uv, rgba ) ; 
    }

    p2 -> setKidState ( getKidState () ) ;
    p2 -> setKidCallback ( SSG_CALLBACK_PREDRAW , getKidPreDrawCB  () ) ;
    p2 -> setKidCallback ( SSG_CALLBACK_POSTDRAW, getKidPostDrawCB () ) ;
    p2 -> setColour  ( colour ) ;
    p2 -> setCenter  ( getCenter () ) ;
    p2 -> setSize    ( getSize   () ) ;
    p2 -> regenerate () ;

    if ( patch[i][0] == 2 )
    {
      /* Make a mirror images in the X axis */

      ssgaPatch *p3 = new ssgaPatch ( ntriangles / 32 ) ;
      ssgaPatch *p4 = new ssgaPatch ( ntriangles / 32 ) ;

      for ( j = 0 ; j < 16 ; j++ )
      {
        p  -> getControlPoint ( j>>2, 3-(j&3), xyz, uv, rgba ) ; 
        xyz [ 0 ] *= -1.0f ;
        p3 -> setControlPoint ( j>>2, j&3, xyz, uv, rgba ) ; 

        p2 -> getControlPoint ( j>>2, 3-(j&3), xyz, uv, rgba ) ; 
        xyz [ 0 ] *= -1.0f ;
        p4 -> setControlPoint ( j>>2, j&3, xyz, uv, rgba ) ; 
      }

      p3 -> setKidState ( getKidState () ) ;
      p3 -> setKidCallback ( SSG_CALLBACK_PREDRAW , getKidPreDrawCB  () ) ;
      p3 -> setKidCallback ( SSG_CALLBACK_POSTDRAW, getKidPostDrawCB () ) ;
      p3 -> setColour  ( colour ) ;
      p3 -> setCenter  ( getCenter () ) ;
      p3 -> setSize    ( getSize   () ) ;
      p3 -> regenerate () ;

      p4 -> setKidState ( getKidState () ) ;
      p4 -> setKidCallback ( SSG_CALLBACK_PREDRAW , getKidPreDrawCB  () ) ;
      p4 -> setKidCallback ( SSG_CALLBACK_POSTDRAW, getKidPostDrawCB () ) ;
      p4 -> setColour  ( colour ) ;
      p4 -> setCenter  ( getCenter () ) ;
      p4 -> setSize    ( getSize   () ) ;
      p4 -> regenerate () ;

      for ( j = 0 ; j < p3 -> getNumKids () ; j++ )
      {
        addKid ( p3->getKid(j) ) ;
        addKid ( p4->getKid(j) ) ;
      }

      p3 -> removeAllKids () ;
      p4 -> removeAllKids () ;
      delete p3 ;
      delete p4 ;
    }

    for ( j = 0 ; j < p -> getNumKids () ; j++ )
    {
      addKid ( p->getKid(j) ) ;
      addKid ( p2->getKid(j) ) ;
    }

    p  -> removeAllKids () ;
    p2 -> removeAllKids () ;
    delete p ;
    delete p2 ;
  }

  recalcBSphere () ;
}


