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

#define MAX_HITS  100
static ssgHit hitlist [ MAX_HITS ] ;
static int next_hit = 0 ;
static ssgEntity *pathlist [ SSG_MAXPATH ] ;
static int next_path = 0 ;

int _ssgIsHotTest = FALSE ;
int _ssgIsLosTest = FALSE ;

void _ssgPushPath ( ssgEntity *e )
{
  if ( next_path + 1 >= SSG_MAXPATH )
  {
    next_path++ ;  /* So pop works! */
    return ;
  }

  pathlist [ next_path++ ] = e ;
}


void _ssgPopPath ()
{
  next_path-- ;
}


void _ssgAddHit ( ssgLeaf *l, int trinum, sgMat4 mat, sgVec4 pl )
{
  if ( next_hit + 1 >= MAX_HITS )
    return ;

  ssgHit *h = & hitlist [ next_hit++ ] ;

  h -> leaf = l ;
  h -> triangle = trinum ;

  h -> num_entries = (next_path>=SSG_MAXPATH) ? SSG_MAXPATH : next_path ;
  memcpy ( h -> path, pathlist, h->num_entries * sizeof ( ssgEntity * ) ) ;

  sgCopyMat4 ( h -> matrix, mat ) ;
  sgCopyVec4 ( h -> plane, pl ) ;
}


int ssgIsect ( ssgRoot *root, sgSphere *s, sgMat4 mat, ssgHit **results )
{
  _ssgIsHotTest = FALSE ;
  _ssgIsLosTest = FALSE ;
  next_hit  = 0 ;
  next_path = 0 ;
  root -> isect ( s, mat, TRUE ) ;
  *results = & hitlist [ 0 ] ;
  return next_hit ;
}


int ssgHOT ( ssgRoot *root, sgVec3 s, sgMat4 mat, ssgHit **results )
{
  _ssgIsHotTest = TRUE ;
  _ssgIsLosTest = FALSE ;
  next_hit  = 0 ;
  next_path = 0 ;
  root -> hot ( s, mat, TRUE ) ;
  *results = & hitlist [ 0 ] ;
  return next_hit ;
}


int ssgLOS ( ssgRoot *root, sgVec3 s, sgMat4 mat, ssgHit **results )
{
  _ssgIsHotTest = FALSE ;
  _ssgIsLosTest = TRUE ;
  next_hit  = 0 ;
  next_path = 0 ;
  root -> los ( s, mat, TRUE ) ;
  *results = & hitlist [ 0 ] ;
  return next_hit ;
}


