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


void ssgTexture::copy_from ( ssgTexture *src, int clone_flags )
{
  ssgBase::copy_from ( src, clone_flags ) ;

  wrapu  = src -> wrapu  ;
  wrapv  = src -> wrapv  ;
  mipmap = src -> mipmap ;

  setFilename ( src -> getFilename () ) ;

  alloc_handle () ;
  ssgTextureInfo info ;
  ssgLoadTexture( filename, &info ) ;
  has_alpha = (info.alpha != 0) ;
  setDefaultGlParams(wrapu, wrapv, mipmap);
}


ssgBase *ssgTexture::clone ( int clone_flags )
{
  ssgTexture *b = new ssgTexture ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


void ssgTexture::setDefaultGlParams(int wrapu, int wrapv, int mipmap)
{
  glTexEnvi       ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE ) ;
  
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) ;
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR ) ;
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    wrapu ? GL_REPEAT : GL_CLAMP ) ;
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    wrapv ? GL_REPEAT : GL_CLAMP ) ;

#ifdef GL_VERSION_1_1
  glBindTexture   ( GL_TEXTURE_2D, 0 ) ;
#else
  glBindTextureEXT ( GL_TEXTURE_2D, 0 ) ;
#endif
}


ssgTexture::ssgTexture ()
{
  type = ssgTypeTexture () ;

  filename = NULL ;

  own_handle = FALSE ;
  handle = 0 ;

  wrapu = TRUE ;
  wrapv = TRUE ;
  mipmap = TRUE ;
  has_alpha = false ;
}


ssgTexture::ssgTexture ( const char *fname, GLubyte *image,
                         int xsize, int ysize, int zsize, 
		         int _wrapu, int _wrapv )
// fname is used when saving the ssgSimpleState to disk.
// If there is no associated file, you can use a dummy name.
{
  type = ssgTypeTexture () ;

  filename = NULL ;

  own_handle = FALSE ;
  handle = 0 ;

  wrapu = _wrapu ;
  wrapv = _wrapv ;
  mipmap = TRUE ;

  setFilename ( fname ) ;

  alloc_handle () ;
#if 0
  ssgTextureInfo info ;
  ssgLoadTexture( filename, &info ) ;
  has_alpha = (info.alpha != 0) ;
#else
  has_alpha = (zsize == 4) ;
  ssgMakeMipMaps ( image, xsize, ysize, zsize );
#endif
  setDefaultGlParams(wrapu, wrapv, TRUE);
}


ssgTexture::ssgTexture ( const char *fname, int _wrapu, int _wrapv, int _mipmap )
{
  type = ssgTypeTexture () ;

  filename = NULL ;

  own_handle = FALSE ;
  handle = 0 ;

  wrapu  = _wrapu  ;
  wrapv  = _wrapv  ;
  mipmap = _mipmap ;

  setFilename ( fname ) ;

  alloc_handle () ;
  ssgTextureInfo info ;
  ssgLoadTexture( filename, &info ) ;
  has_alpha = (info.alpha != 0) ;
  setDefaultGlParams(wrapu, wrapv, mipmap);
}


ssgTexture::~ssgTexture (void)
{
  setFilename ( NULL ) ;
  free_handle () ;
}


void ssgTexture::alloc_handle ()
{
  free_handle () ;

  own_handle = TRUE ;

#ifdef GL_VERSION_1_1
  glGenTextures ( 1, & handle ) ;
  glBindTexture ( GL_TEXTURE_2D, handle ) ;
#else
  /* This is only useful on some ancient SGI hardware */
  glGenTexturesEXT ( 1, & handle ) ;
  glBindTextureEXT ( GL_TEXTURE_2D, handle ) ;
#endif
}


void ssgTexture::free_handle ()
{
  if ( handle != 0 )
  {
    if ( own_handle )
    {
#ifdef GL_VERSION_1_1
      glDeleteTextures ( 1, & handle ) ;
#else
      /* This is only useful on some ancient SGI hardware */
      glDeleteTexturesEXT ( 1, & handle ) ;
#endif
    }

    own_handle = FALSE ;
    handle = 0 ;
  }
}


void ssgTexture::setHandle ( GLuint _handle )
{
  free_handle () ;

  own_handle = FALSE ;
  handle = _handle ;
}


void ssgTexture::print ( FILE *fd, char *ident, int how_much )
{
  fprintf ( fd, "%s%s: %s\n", ident, getTypeName (), getFilename () ) ;
}


int ssgTexture::load ( FILE *fd )
{
  delete [] filename ;

  _ssgReadString ( fd, & filename ) ;
  _ssgReadInt    ( fd, & wrapu    ) ;
  _ssgReadInt    ( fd, & wrapv    ) ;
  _ssgReadInt    ( fd, & mipmap   ) ;

  alloc_handle () ;
  ssgTextureInfo info ;
  ssgLoadTexture( filename, &info ) ;
  has_alpha = (info.alpha != 0) ;
  setDefaultGlParams(wrapu, wrapv, mipmap);

  return ssgBase::load ( fd ) ;
}


int ssgTexture::save ( FILE *fd )
{
  _ssgWriteString ( fd, filename ) ;
  _ssgWriteInt    ( fd, wrapu    ) ;
  _ssgWriteInt    ( fd, wrapv    ) ;
  _ssgWriteInt    ( fd, mipmap   ) ;

  return ssgBase::save ( fd ) ;
}


void ssgTextureArray::add ( ssgTexture* tex )
{
  if ( tex )
  {
    tex -> ref () ;
    raw_add ( (char *) &tex ) ;
  }
}


void ssgTextureArray::removeAll ()
{
  for ( int i = 0; i < getNum (); i++ )
    ssgDeRefDelete ( get (i) ) ;
  ssgSimpleList::removeAll () ;
}


ssgTexture* ssgTextureArray::findByFilename ( const char* fname )
{
  for ( int i = 0; i < getNum (); i++ )
  {
    ssgTexture *tex = get (i) ;
    if ( ulStrEqual ( fname, tex->getFilename() ) )
      return tex ;
  }
  return NULL ;
}


