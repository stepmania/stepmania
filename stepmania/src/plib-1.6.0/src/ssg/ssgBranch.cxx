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

void ssgBranch::copy_from ( ssgBranch *src, int clone_flags )
{
  ssgEntity::copy_from ( src, clone_flags ) ;

  for ( int i = 0 ; i < src -> getNumKids () ; i++ )
  {
    ssgEntity *k = src -> getKid ( i ) ;

    if ( k != NULL && ( clone_flags & SSG_CLONE_RECURSIVE ) )
      addKid ( (ssgEntity *)( k -> clone ( clone_flags )) ) ;
    else
      addKid ( k ) ;
  }
}

ssgBase *ssgBranch::clone ( int clone_flags )
{
  ssgBranch *b = new ssgBranch ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}



ssgBranch::ssgBranch (void)
{
  type = ssgTypeBranch () ;
}


ssgBranch::~ssgBranch (void)
{
  removeAllKids () ;
}


void ssgBranch::zeroSpareRecursive ()
{
  zeroSpare () ;

  for ( ssgEntity *k = getKid ( 0 ) ; k != NULL ; k = getNextKid () )
    k -> zeroSpareRecursive () ;
}


void ssgBranch::recalcBSphere (void)
{
  emptyBSphere () ;

  for ( ssgEntity *k = getKid ( 0 ) ; k != NULL ; k = getNextKid () )
    extendBSphere ( k -> getBSphere () ) ;

  bsphere_is_invalid = FALSE ;
}


void ssgBranch::addKid ( ssgEntity *entity )
{
  kids. addEntity ( entity ) ;
  entity -> addParent ( this ) ;
  dirtyBSphere () ;
}


void ssgBranch::removeKid  ( int n )
{
  ssgEntity *k = kids.getEntity ( n ) ;

  if ( k != NULL )
    removeKid ( k ) ;
}


void ssgBranch::removeKid ( ssgEntity *entity )
{
  entity -> removeParent ( this ) ;
  kids.removeEntity ( entity ) ;
  dirtyBSphere () ;
}


void ssgBranch::removeAllKids (void)
{
  ssgEntity *k ;

  while ( ( k = getKid ( 0 ) ) != NULL )
    removeKid ( k ) ;
}


void ssgBranch::replaceKid ( int n, ssgEntity *new_entity )
{
  if ( n >= 0 && n < getNumKids () )
  {
    getKid ( n ) -> removeParent ( this ) ;
    kids.replaceEntity ( n, new_entity ) ;
    new_entity -> addParent ( this ) ;
    dirtyBSphere () ;
  }
}

void ssgBranch::replaceKid ( ssgEntity *old_entity, ssgEntity *new_entity )
{
  replaceKid ( searchForKid( old_entity ), new_entity ) ;
}


void ssgBranch::print ( FILE *fd, char *indent, int how_much )
{
  ssgEntity::print ( fd, indent, how_much ) ;
  fprintf ( fd, "%s  Num Kids=%d\n", indent, getNumKids() ) ;

  if ( getNumParents() != getRef() )
    ulSetError ( UL_WARNING, "Ref count doesn't tally with parent count" ) ;

	if ( how_much > 1 )
  {	if ( bsphere.isEmpty() )
			fprintf ( fd, "%s  BSphere is Empty.\n", indent ) ;
		else
			fprintf ( fd, "%s  BSphere  R=%g, C=(%g,%g,%g)\n", indent,
				bsphere.getRadius(), bsphere.getCenter()[0], bsphere.getCenter()[1], bsphere.getCenter()[2] ) ;
	}

  char in [ 100 ] ;
  sprintf ( in, "%s  ", indent ) ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid() )
    e -> print ( fd, in, how_much ) ;
}


ssgEntity *ssgBranch::getByName ( char *match )
{
  if ( getName() != NULL && strcmp ( getName(), match ) == 0 )
    return this ;

  /* Otherwise check the kids for a match */

  for ( ssgEntity* k = getKid ( 0 ) ; k != NULL ; k = getNextKid() )
  {
    ssgEntity *e = k -> getByName ( match ) ;

    if ( e != NULL )
      return e ;
  }

  return NULL ;
}


ssgEntity *ssgBranch::getByPath ( char *path )
{
  /* Ignore leading '/' */

  if ( *path == '/' )
    ++path ;

  char *n = getName () ;

  /*
    If this node has no name then pass the request down the tree
  */

  if ( n == NULL )
  {
    for ( ssgEntity* k = getKid ( 0 ) ; k != NULL ; k = getNextKid () )
    {
      ssgEntity *e = k -> getByPath ( path ) ;

      if ( e != NULL )
        return e ;
    }

    return NULL ;
  }

  /*
    If this node does have a name - but it doesn't match the
    next part of the path then punt.
  */

  unsigned int l = strlen ( n ) ;

  if ( strlen ( path ) < l || strncmp ( n, path, l ) != 0 )
    return NULL ;

  /*
    If the first part of the path is this ssgBranch, we
    may have a winner.
  */

  char c = path [ l ] ;

  /* If we reached the end of the path - we win! */

  if ( c == '\0' )
    return this ;

  if ( c == '/' )
  {
    /* If the path continues, try to follow the path to the kids */

    for ( ssgEntity* k = getKid ( 0 ) ; k != NULL ; k = getNextKid () )
    {
      ssgEntity *e = k -> getByPath ( path + l ) ;

      if ( e != NULL )
        return e ;
    }
  }

  return NULL ;
}
 


void ssgBranch::cull ( sgFrustum *f, sgMat4 m, int test_needed )
{
  if ( ! preTravTests ( &test_needed, SSGTRAV_CULL ) )
    return ;

  int cull_result = cull_test ( f, m, test_needed ) ;

  if ( cull_result == SSG_OUTSIDE )
    return ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid() )
    e -> cull ( f, m, cull_result != SSG_INSIDE ) ;

  postTravTests ( SSGTRAV_CULL ) ; 
}



void ssgBranch::hot ( sgVec3 s, sgMat4 m, int test_needed )
{
  if ( ! preTravTests ( &test_needed, SSGTRAV_HOT ) )
    return ;

  int hot_result = hot_test ( s, m, test_needed ) ;

  if ( hot_result == SSG_OUTSIDE )
    return ;

  _ssgPushPath ( this ) ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid() )
    e -> hot ( s, m, hot_result != SSG_INSIDE ) ;

  _ssgPopPath () ;

  postTravTests ( SSGTRAV_HOT ) ;
}



void ssgBranch::los ( sgVec3 s, sgMat4 m, int test_needed )
{
  if ( ! preTravTests ( &test_needed, SSGTRAV_LOS ) )
    return ;

  int los_result = los_test ( s, m, test_needed ) ;

  if ( los_result == SSG_OUTSIDE )
    return ;

  _ssgPushPath ( this ) ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid() )
    e -> los ( s, m, los_result != SSG_INSIDE ) ;

  _ssgPopPath () ;

  postTravTests ( SSGTRAV_LOS) ;
}


void ssgBranch::isect ( sgSphere *s, sgMat4 m, int test_needed )
{
  if ( ! preTravTests ( &test_needed, SSGTRAV_ISECT ) )
    return ;

  int isect_result = isect_test ( s, m, test_needed ) ;

  if ( isect_result == SSG_OUTSIDE )
    return ;

  _ssgPushPath ( this ) ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid() )
    e -> isect ( s, m, isect_result != SSG_INSIDE ) ;

  _ssgPopPath () ;

  postTravTests ( SSGTRAV_ISECT ) ; 
}


int ssgBranch::load ( FILE *fd )
{
  int nkids ;

  _ssgReadInt ( fd, & nkids ) ;

  if ( ! ssgEntity::load ( fd ) )
    return FALSE ;

  for ( int i = 0 ; i < nkids ; i++ )
  {
    ssgEntity *kid ;

    if ( ! _ssgLoadObject ( fd, (ssgBase **) &kid, ssgTypeEntity () ) )
      return FALSE ;

    addKid ( kid ) ;
  }

  return TRUE ;
}


int ssgBranch::save ( FILE *fd )
{
  _ssgWriteInt ( fd, getNumKids() ) ;

  if ( ! ssgEntity::save ( fd ) )
    return FALSE ;

  for ( int i = 0 ; i < getNumKids() ; i++ )
  {
    if ( ! _ssgSaveObject ( fd, getKid ( i ) ) )
       return FALSE ;
  }

  return TRUE ;
}


