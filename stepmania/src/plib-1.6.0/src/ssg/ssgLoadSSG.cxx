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

// code for loading and writing .ssg files

#include "ssgLocal.h"

// used for reading only:
int _ssgFileVersionNumber = 0 ;

// simple list of ssgBase derived objects:
class _ssgBaseList : public ssgSimpleList
{
public:

  _ssgBaseList() : ssgSimpleList( sizeof(ssgBase *), 16 ) {}

  ~_ssgBaseList() {}

  ssgBase *get ( unsigned n )
  {
    return n < total ? ((ssgBase **) list) [ n ] : NULL;
  }
  
  void add ( ssgBase *obj )
  {
    sizeChk ( 1 ) ;
    ((ssgBase **) list) [ total++ ] = obj ;
  }

  int find ( ssgBase *obj ) 
  {
    for ( unsigned i = 0 ; i < total ; i++ )
      if ( ((ssgBase **) list) [ i ] == obj )
	return i ;
    return -1 ;
  }

};

// list of ssgBase objects for instance referencing:
static _ssgBaseList *_ssgInstanceList ;


int _ssgLoadObject ( FILE *f, ssgBase **objp, int type_mask )
{
  int type = 0, key = 0;
  ssgBase *obj;
  
  _ssgReadInt ( f, &type ) ;
  
  if ( type == _SSG_BACKWARDS_REFERENCE ) 
  {
    _ssgReadInt ( f, &key ) ;
    
    obj = _ssgInstanceList -> get ( key ) ;
    if ( obj == NULL )
    {
      if ( key != 0 )
      {
	ulSetError ( UL_WARNING, 
		     "ssgLoadObject: Unexpected null object for key %d.", key ) ;
	return FALSE ;
      }
    }
    else if ( ! obj -> isAKindOf ( type_mask ) )
    {
      ulSetError ( UL_WARNING, "ssgLoadObject: Bad type %#x (%s), expected %#x.", 
		   obj -> getType (), obj -> getTypeName (), type_mask ) ;
      return FALSE ;
    }
  }
  else
  {
    if ( ( type & type_mask ) != type_mask )
    {
      ulSetError ( UL_WARNING, "ssgLoadObject: Bad type %#x, expected %#x.",
		   type, type_mask ) ;
      return FALSE ;
    }
    
    obj = ssgCreateOfType ( type ) ;
    if ( obj == NULL )
       return FALSE ;

    _ssgInstanceList -> add ( obj ) ;
    
    if ( ! obj -> load ( f ) )
    {
      ulSetError ( UL_DEBUG, "ssgLoadObject: Failed to load object of type %s.",
		   obj -> getTypeName () ) ;
      return FALSE ;
    }

    if ( obj -> isAKindOf ( ssgTypeEntity () ) )
    {
      ((ssgEntity *) obj) -> recalcBSphere () ;
    }
  }
  
  if ( _ssgReadError () )
  {
    ulSetError ( UL_WARNING, "ssgLoadObject: Read error." ) ;
    return FALSE ;
  }
  
  *objp = obj ;
  
  return TRUE ;
}


int _ssgSaveObject ( FILE *f, ssgBase *obj )
{
  int key = _ssgInstanceList -> find ( obj ) ;

  if ( key >= 0 )
  {
    _ssgWriteInt ( f, _SSG_BACKWARDS_REFERENCE ) ;
    _ssgWriteInt ( f, key ) ;
  }
  else
  {
    _ssgWriteInt ( f, obj -> getType () ) ;

    _ssgInstanceList -> add ( obj ) ;
    
    if ( ! obj -> save ( f ) )
    {
      ulSetError ( UL_DEBUG, "ssgSaveObject: Failed to save object of type %s.", 
		   obj -> getTypeName () ) ;
      return FALSE ;
    }
  }

  if ( _ssgWriteError () )
  {
    ulSetError ( UL_WARNING, "ssgSaveObject: Write error." ) ;
    return FALSE ;
  }

  return TRUE ;
}


ssgEntity *ssgLoadSSG ( const char *fname, const ssgLoaderOptions* options )
{
  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  const ssgLoaderOptions* current_options = ssgGetCurrentOptions () ;

  char filename [ 1024 ] ;
  current_options -> makeModelPath ( filename, fname ) ;

  FILE *fd = fopen ( filename, "rb" ) ;

  if ( fd == NULL )
  {
    perror ( filename ) ;
    ulSetError ( UL_WARNING, 
		 "ssgLoadSSG: Failed to open '%s' for reading.", filename ) ;
    return NULL ;
  }

  int magic ;
  ssgEntity *kid ;

  _ssgReadInt ( fd, & magic ) ;

  if ( ( magic & 0xFFFFFF00 ) != ( SSG_FILE_MAGIC_NUMBER & 0xFFFFFF00 ) )
  {
    if (((magic & 0x0000FF)>> 0)==((SSG_FILE_MAGIC_NUMBER & 0xFF000000)>>24) &&
        ((magic & 0x00FF00)>> 8)==((SSG_FILE_MAGIC_NUMBER & 0x00FF0000)>>16) &&
        ((magic & 0xFF0000)>>16)==((SSG_FILE_MAGIC_NUMBER & 0x0000FF00)>> 8) ) 
      ulSetError ( UL_WARNING, "ssgLoadSSG: File appears to be byte swapped!" ) ;
    else
      ulSetError ( UL_WARNING, "ssgLoadSSG: File has incorrect magic number!" ) ;

    return NULL ;
  }

  /*
    Save the old version number so we can do recursive loads
  */

  int oldFileVersion = _ssgFileVersionNumber ;
  _ssgFileVersionNumber = ( magic & 0xFF ) ;

  if ( _ssgFileVersionNumber == 0 )
  {
    ulSetError ( UL_WARNING, 
		 "ssgLoadSSG: SSG file format version zero is no longer supported, sorry! For more, see the docs." ) ;
    _ssgFileVersionNumber = oldFileVersion ;
    return NULL ;
  }

  if ( _ssgFileVersionNumber > SSG_FILE_VERSION )
  {
    ulSetError ( UL_WARNING, 
		 "ssgLoadSSG: This version of SSG is too old to load this file!" ) ;
    _ssgFileVersionNumber = oldFileVersion ;
    return NULL ;
  }

  _ssgBaseList *oldInstanceList = _ssgInstanceList ; // in case of recursive loads
  _ssgInstanceList = new _ssgBaseList ;
  _ssgInstanceList -> add ( NULL ) ; // index 0 --> NULL

  int success = _ssgLoadObject ( fd, (ssgBase **) &kid, ssgTypeEntity () ) ;

  if ( ! success )
  {
    ulSetError ( UL_WARNING, "ssgLoadSSG: Failed to load object." ) ;
    kid = NULL ;
  }

  delete _ssgInstanceList ;
  _ssgInstanceList = oldInstanceList ;
  _ssgFileVersionNumber = oldFileVersion ;

  fclose ( fd ) ;

  return kid ;
}


int ssgSaveSSG ( const char *filename, ssgEntity *ent )
{
  FILE *fd = fopen ( filename, "wb" ) ;

  if ( fd == NULL )
  {
    perror ( filename ) ;
    ulSetError ( UL_WARNING, 
		 "ssgSaveSSG: Failed to open '%s' for writing.", filename ) ;
    return FALSE ;
  }

  _ssgBaseList *oldInstanceList = _ssgInstanceList ; // for recursive saves
  _ssgInstanceList = new _ssgBaseList ;
  _ssgInstanceList -> add ( NULL ) ; // index 0 --> NULL

  _ssgWriteInt ( fd, SSG_FILE_MAGIC_NUMBER ) ;

  int success = _ssgSaveObject ( fd, ent ) ;

  if ( ! success ) 
    ulSetError ( UL_WARNING, "ssgSaveSSG: Failed to write object." ) ;

  delete _ssgInstanceList ;
  _ssgInstanceList = oldInstanceList ;

  fclose ( fd ) ;  

  return success ;
}


