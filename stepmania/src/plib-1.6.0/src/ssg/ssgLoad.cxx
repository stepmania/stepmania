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


static ssgLoaderOptions default_options ;

ssgLoaderOptions *_ssgCurrentOptions = &default_options ;



char* ssgLoaderOptions::make_path ( char* path,
      const char* dir, const char* fname ) const
{
  if ( fname != NULL && fname [ 0 ] != '\0' )
  {
    if ( ! ulIsAbsolutePathName ( fname ) &&
       dir != NULL && dir[0] != '\0' )
    {
      strcpy ( path, dir ) ;
      strcat ( path, "/" ) ;
      strcat ( path, fname ) ;
    }
    else
      strcpy ( path, fname ) ;

    //convert backward slashes to forward slashes
    for ( char* ptr = path ; *ptr ; ptr ++ )
    {
      if ( *ptr == '\\' )
        *ptr = '/' ;
    }
  }
  else
     path [0] = 0 ;
  return( path );
}


void ssgLoaderOptions::makeModelPath ( char *path, const char *fname ) const
{
  make_path ( path, model_dir, fname ) ;
}


void ssgLoaderOptions::makeTexturePath ( char *path, const char *fname ) const
{
  /* Remove all leading path information. */
  const char* seps = "\\/" ;
  const char* fn = & fname [ strlen ( fname ) - 1 ] ;
  for ( ; fn != fname && strchr(seps,*fn) == NULL ; fn-- )
    /* Search back for a seperator */ ;
  if ( strchr(seps,*fn) != NULL )
    fn++ ;
  fname = fn ;

  make_path ( path, texture_dir, fname ) ;
}


ssgLeaf* ssgLoaderOptions::createLeaf ( ssgLeaf* leaf,
                                        const char* parent_name )
{
  if ( leaf != NULL )
  {
    /* try to do some state sharing */
    ssgState* st = leaf -> getState () ;
    if ( st != NULL && st -> isA ( ssgTypeSimpleState () ) )
    {
      ssgSimpleState *ss = (ssgSimpleState*) st ;
      ssgSimpleState *match = shared_states.findMatch ( ss ) ;
      if ( match != NULL )
        leaf -> setState ( match ) ;
      else
        shared_states.add ( ss ) ;
    }
  }
  return leaf ;
}


ssgTexture* ssgLoaderOptions::createTexture ( char* tfname,
						     int wrapu,
						     int wrapv,
						     int mipmap ) 
{
  
  char filename [ 1024 ] ;
  makeTexturePath ( filename, tfname ) ;

  ssgTexture *tex = shared_textures.findByFilename ( filename ) ;
  if ( tex )
    return tex ;
  
  tex = new ssgTexture ( filename, wrapu, wrapv, mipmap ) ;
  if ( tex )
    shared_textures.add ( tex ) ;
  return tex ;
}

ssgTransform* ssgLoaderOptions::createTransform ( ssgTransform* tr,
      ssgTransformArray* ta ) const
{
  if ( ta != NULL )
    tr -> setUserData ( ta ) ;
  return tr ;
}

ssgSelector* ssgLoaderOptions::createSelector ( ssgSelector* s ) const
{
  return s ;
}


void ssgLoaderOptions::setModelDir ( const char *s )
{
  delete [] model_dir ;
  model_dir = new char [ strlen ( s ) + 1 ] ;
  strcpy ( model_dir, s ) ;
}

void ssgLoaderOptions::setTextureDir ( const char *s )
{
  delete [] texture_dir ;
  texture_dir = new char [ strlen ( s ) + 1 ] ;
  strcpy ( texture_dir, s ) ;
}


static const char *file_extension ( const char *fname )
{
  const char *p = & ( fname [ strlen(fname) ] ) ;

  while ( p != fname && *p != '/' && *p != '.' )
    p-- ;

  return p ;
}


struct _ssgModelFormat
{
  const char *extension ;
  ssgLoadFunc *loadfunc ;
  ssgSaveFunc *savefunc ;
} ;


enum { MAX_FORMATS = 100 } ;

static _ssgModelFormat formats [ MAX_FORMATS ] ;
static int num_formats = 0 ;


void ssgAddModelFormat ( const char* extension,
                        ssgLoadFunc *loadfunc , ssgSaveFunc  *savefunc )
{
  for ( int i = 0 ; i < num_formats ; i++ ) 
  {
    if ( ulStrEqual ( formats [ i ] . extension, extension ) )
    {
      formats [ i ] . extension = extension ;
      formats [ i ] . loadfunc = loadfunc ;
      formats [ i ] . savefunc = savefunc ;
      return ;
    }
  }

  if ( num_formats < MAX_FORMATS )
  {
    formats [ num_formats ] . extension = extension ;
    formats [ num_formats ] . loadfunc = loadfunc ;
    formats [ num_formats ] . savefunc = savefunc ;
    num_formats ++ ;
  }
  else
  {
    ulSetError ( UL_WARNING, "ssgAddModelFormat: too many formats" );
  }
}



ssgEntity *ssgLoad ( const char *fname, const ssgLoaderOptions* options )
{
  if ( fname == NULL || *fname == '\0' )
    return NULL ;

	// find appropiate loader and call its loadfunc
  const char *extn = file_extension ( fname ) ;
  if ( *extn != '.' )
  {
    ulSetError ( UL_WARNING, "ssgLoad: Cannot determine file type for '%s'", fname );
    return NULL ;
  }

  _ssgModelFormat *f = formats ;
  for ( int i=0; i<num_formats; i++, f++ )
  {
    if ( f->loadfunc != NULL &&
	       ulStrEqual ( extn, f->extension ) )
    {
      ssgEntity* entity = f -> loadfunc( fname, options ) ;
      ssgGetCurrentOptions () -> endLoad () ;
      return entity ;
    }
  }

  ulSetError ( UL_WARNING, "ssgLoad: Unrecognised file type '%s'", extn ) ;
  return NULL ;
}


int ssgSave ( const char *fname, ssgEntity *ent )
{
  if ( fname == NULL || ent == NULL || *fname == '\0' )
    return FALSE ;

  const char *extn = file_extension ( fname ) ;

  if ( *extn != '.' )
  {
    ulSetError ( UL_WARNING, "ssgSave: Cannot determine file type for '%s'", fname );
    return FALSE ;
  }

  _ssgModelFormat *f = formats ;
  for ( int i=0; i<num_formats; i++, f++ )
  {
    if ( f->savefunc != NULL &&
         ulStrEqual ( extn, f->extension ) )
      return f->savefunc( fname, ent ) ;
  }

  ulSetError ( UL_WARNING, "ssgSave: Unrecognised file type '%s'", extn ) ;
  return FALSE ;
}
