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

void ssgTransform::copy_from ( ssgTransform *src, int clone_flags )
{
  ssgBaseTransform::copy_from ( src, clone_flags ) ;
}

ssgBase *ssgTransform::clone ( int clone_flags )
{
  ssgTransform *b = new ssgTransform ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgTransform::ssgTransform ( sgCoord *c )
{
  type = ssgTypeTransform () ;
  setTransform ( c ) ;
}

ssgTransform::ssgTransform (void)
{
  type = ssgTypeTransform () ;
}

ssgTransform::~ssgTransform (void)
{
}

void ssgTransform::recalcBSphere (void)
{
  ssgBranch::recalcBSphere () ;

  if ( ! bsphere . isEmpty () )
    bsphere . orthoXform ( transform ) ;
}

void ssgTransform::cull ( sgFrustum *f, sgMat4 m, int test_needed )
{
  if ( ! preTravTests ( &test_needed, SSGTRAV_CULL ) )
    return ;

  int cull_result = cull_test ( f, m, test_needed ) ;

  if ( cull_result == SSG_OUTSIDE )
    return ;

  sgMat4 tmp ;

  sgCopyMat4 ( tmp, m ) ;
  sgPreMultMat4 ( tmp, transform ) ;

  _ssgPushMatrix ( tmp ) ;
  glPushMatrix () ;
  glLoadMatrixf ( (float *) tmp ) ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid() )
    e -> cull ( f, tmp, cull_result != SSG_INSIDE ) ;

  glPopMatrix () ;
  _ssgPopMatrix () ;

  postTravTests ( SSGTRAV_CULL ) ; 
}

void ssgTransform::hot ( sgVec3 s, sgMat4 m, int test_needed )
{
  if ( ! preTravTests ( &test_needed, SSGTRAV_HOT ) )
    return ;

  int hot_result = hot_test ( s, m, test_needed ) ;

  if ( hot_result == SSG_OUTSIDE )
    return ;

  sgMat4 tmp ;

  sgCopyMat4 ( tmp, m ) ;
  sgPreMultMat4 ( tmp, transform ) ;

  _ssgPushPath ( this ) ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid() )
    e -> hot ( s, tmp, hot_result != SSG_INSIDE ) ;

  _ssgPopPath () ;

  postTravTests ( SSGTRAV_HOT ) ;
}

void ssgTransform::los ( sgVec3 s, sgMat4 m, int test_needed )
{
  if ( ! preTravTests ( &test_needed, SSGTRAV_LOS ) )
    return ;

  int los_result = los_test ( s, m, test_needed ) ;

  if ( los_result == SSG_OUTSIDE )
    return ;

  sgMat4 tmp ;

  sgCopyMat4 ( tmp, m ) ;
  sgPreMultMat4 ( tmp, transform ) ;

  _ssgPushPath ( this ) ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid() )
    e -> los ( s, tmp, los_result != SSG_INSIDE ) ;

  _ssgPopPath () ;

  postTravTests ( SSGTRAV_LOS ) ;
}

void ssgTransform::isect ( sgSphere *s, sgMat4 m, int test_needed )
{
  if ( ! preTravTests ( &test_needed, SSGTRAV_ISECT ) )
    return ;

  int isect_result = isect_test ( s, m, test_needed ) ;

  if ( isect_result == SSG_OUTSIDE )
    return ;

  sgMat4 tmp ;

  sgCopyMat4 ( tmp, m ) ;
  sgPreMultMat4 ( tmp, transform ) ;

  _ssgPushPath ( this ) ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid() )
    e -> isect ( s, tmp, isect_result != SSG_INSIDE ) ;

  _ssgPopPath () ;

  postTravTests ( SSGTRAV_ISECT ) ; 
}

void ssgTransform::setTransform ( sgVec3 xyz )
{
  updateTransform () ;
  sgMakeTransMat4 ( transform, xyz ) ;
  firsttime       () ; 
  dirtyBSphere    () ;
}

void ssgTransform::setTransform ( sgCoord *xform )
{
  updateTransform () ;
  sgMakeCoordMat4 ( transform, xform ) ;
  firsttime       () ; 
  dirtyBSphere () ;
}

void ssgTransform::setTransform ( sgCoord *xform, float sx, float sy, float sz )
{
  updateTransform () ;
  sgMakeCoordMat4 ( transform, xform ) ;
  sgScaleVec3     ( transform[0], sx ) ;
  sgScaleVec3     ( transform[1], sy ) ;
  sgScaleVec3     ( transform[2], sz ) ;
  firsttime       () ; 
  dirtyBSphere () ;
}

void ssgTransform::setTransform ( sgMat4 xform )
{
  updateTransform () ;
  sgCopyMat4      ( transform, xform ) ;
  firsttime       () ; 
  dirtyBSphere () ;
}



int ssgTransform::load ( FILE *fd )
{
  return ssgBaseTransform::load(fd) ;
}

int ssgTransform::save ( FILE *fd )
{
  return ssgBaseTransform::save(fd) ;
}


