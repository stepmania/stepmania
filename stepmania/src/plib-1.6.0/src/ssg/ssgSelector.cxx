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

void ssgSelector::copy_from ( ssgSelector *src, int clone_flags )
{
  ssgBranch::copy_from ( src, clone_flags ) ;

  max_kids = src -> max_kids ;
  selection = new unsigned char [ max_kids ] ;
  memcpy ( selection, src -> selection, max_kids ) ;
}


ssgBase *ssgSelector::clone ( int clone_flags )
{
  ssgSelector *b = new ssgSelector ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgSelector::ssgSelector ( int _max_kids )
{
  type = ssgTypeSelector () ;

  max_kids = _max_kids ;
  selection = new unsigned char [ max_kids ] ;
  memset ( selection, 1, max_kids ) ;
}

ssgSelector::~ssgSelector (void)
{
  delete [] selection ;
}

void ssgSelector::cull ( sgFrustum *f, sgMat4 m, int test_needed )
{
  if ( ! preTravTests ( &test_needed, SSGTRAV_CULL ) )
    return ;

  int cull_result = cull_test ( f, m, test_needed ) ;

  if ( cull_result == SSG_OUTSIDE )
    return ;

  int s = 0 ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid(), s++ )
    if ( selection [s] )
      e -> cull ( f, m, cull_result != SSG_INSIDE ) ;

  postTravTests ( SSGTRAV_CULL ) ; 
}

void ssgSelector::hot ( sgVec3 sp, sgMat4 m, int test_needed )
{
  if ( ! preTravTests ( &test_needed, SSGTRAV_HOT ) )
    return ;

  int hot_result = hot_test ( sp, m, test_needed ) ;

  if ( hot_result == SSG_OUTSIDE )
    return ;

  int s = 0 ;

  _ssgPushPath ( this ) ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid(), s++ )
    if ( selection [s] )
      e -> hot ( sp, m, hot_result != SSG_INSIDE ) ;

  _ssgPopPath () ;

  postTravTests ( SSGTRAV_HOT ) ;
}


void ssgSelector::los ( sgVec3 sp, sgMat4 m, int test_needed )
{
  if ( ! preTravTests ( &test_needed, SSGTRAV_LOS ) )
    return ;

  int los_result = los_test ( sp, m, test_needed ) ;

  if ( los_result == SSG_OUTSIDE )
    return ;

  int s = 0 ;

  _ssgPushPath ( this ) ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid(), s++ )
    if ( selection [s] )
      e -> los ( sp, m, los_result != SSG_INSIDE ) ;

  _ssgPopPath () ;

  postTravTests ( SSGTRAV_LOS ) ;
}


void ssgSelector::isect ( sgSphere *sp, sgMat4 m, int test_needed )
{
  if ( ! preTravTests ( &test_needed, SSGTRAV_ISECT ) )
    return ;

  int isect_result = isect_test ( sp, m, test_needed ) ;

  if ( isect_result == SSG_OUTSIDE )
    return ;

  int s = 0 ;

  _ssgPushPath ( this ) ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid(), s++ )
    if ( selection [s] )
      e -> isect ( sp, m, isect_result != SSG_INSIDE ) ;

  _ssgPopPath () ;

  postTravTests ( SSGTRAV_ISECT ) ; 
}


int ssgSelector::load ( FILE *fd )
{
  _ssgReadInt ( fd, & max_kids ) ;
  delete [] selection ;
  selection = new unsigned char [ max_kids ] ;
  for ( int i=0; i<max_kids; i++ )
  {
    int temp ;
    _ssgReadInt ( fd, & temp ) ;
    selection [i] = (unsigned char)temp ;
  }
  return ssgBranch::load(fd) ;
}

int ssgSelector::save ( FILE *fd )
{
  _ssgWriteInt ( fd, max_kids ) ;
  for ( int i=0; i<max_kids; i++ )
    _ssgWriteInt ( fd, (int)selection [i] ) ;
  return ssgBranch::save(fd) ;
}


