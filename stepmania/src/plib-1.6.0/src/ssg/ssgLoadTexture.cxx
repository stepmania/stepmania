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


struct _ssgTextureFormat
{
  const char *extension ;
  bool (*loadfunc) ( const char *, ssgTextureInfo* info ) ;
} ;


enum { MAX_FORMATS = 100 } ;

static _ssgTextureFormat formats [ MAX_FORMATS ] ;
static int num_formats = 0 ;

static int total_texels_loaded = 0 ;


int ssgGetNumTexelsLoaded ()
{
  return total_texels_loaded ;
}


bool ssgMakeMipMaps ( GLubyte *image, int xsize, int ysize, int zsize )
{
  if ( ! ((xsize & (xsize-1))==0) ||
       ! ((ysize & (ysize-1))==0) )
  {
    ulSetError ( UL_WARNING, "Map is not a power-of-two in size!" ) ;
    return false ;
  }

  GLubyte *texels [ 20 ] ;   /* One element per level of MIPmap */

  for ( int l = 0 ; l < 20 ; l++ )
    texels [ l ] = NULL ;

  texels [ 0 ] = image ;

  int lev ;

  for ( lev = 0 ; (( xsize >> (lev+1) ) != 0 ||
                   ( ysize >> (lev+1) ) != 0 ) ; lev++ )
  {
    /* Suffix '1' is the higher level map, suffix '2' is the lower level. */

    int l1 = lev   ;
    int l2 = lev+1 ;
    int w1 = xsize >> l1 ;
    int h1 = ysize >> l1 ;
    int w2 = xsize >> l2 ;
    int h2 = ysize >> l2 ;

    if ( w1 <= 0 ) w1 = 1 ;
    if ( h1 <= 0 ) h1 = 1 ;
    if ( w2 <= 0 ) w2 = 1 ;
    if ( h2 <= 0 ) h2 = 1 ;

    texels [ l2 ] = new GLubyte [ w2 * h2 * zsize ] ;

    for ( int x2 = 0 ; x2 < w2 ; x2++ )
      for ( int y2 = 0 ; y2 < h2 ; y2++ )
        for ( int c = 0 ; c < zsize ; c++ )
        {
          int x1   = x2 + x2 ;
          int x1_1 = ( x1 + 1 ) % w1 ;
          int y1   = y2 + y2 ;
          int y1_1 = ( y1 + 1 ) % h1 ;

	  int t1 = texels [ l1 ] [ (y1   * w1 + x1  ) * zsize + c ] ;
	  int t2 = texels [ l1 ] [ (y1_1 * w1 + x1  ) * zsize + c ] ;
	  int t3 = texels [ l1 ] [ (y1   * w1 + x1_1) * zsize + c ] ;
	  int t4 = texels [ l1 ] [ (y1_1 * w1 + x1_1) * zsize + c ] ;

          texels [ l2 ] [ (y2 * w2 + x2) * zsize + c ] =
                                           ( t1 + t2 + t3 + t4 ) / 4 ;
        }
  }

  texels [ lev+1 ] = NULL ;

  glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 ) ;

  int map_level = 0 ;

#ifndef PROXY_TEXTURES_ARE_BROKEN
  GLint ww ;

  do
  {
    glTexImage2D  ( GL_PROXY_TEXTURE_2D,
                     map_level, zsize, xsize, ysize, FALSE /* Border */,
                            (zsize==1)?GL_LUMINANCE:
                            (zsize==2)?GL_LUMINANCE_ALPHA:
                            (zsize==3)?GL_RGB:
                                       GL_RGBA,
                            GL_UNSIGNED_BYTE, NULL ) ;

    glGetTexLevelParameteriv ( GL_PROXY_TEXTURE_2D, 0,GL_TEXTURE_WIDTH, &ww ) ;

    if ( ww == 0 )
    {
      delete [] texels [ 0 ] ;
      xsize >>= 1 ;
      ysize >>= 1 ;

      for ( int l = 0 ; texels [ l ] != NULL ; l++ )
	texels [ l ] = texels [ l+1 ] ;

      if ( xsize < 64 && ysize < 64 )
      {
        ulSetError ( UL_FATAL,
           "SSG: OpenGL will not accept a downsized version ?!?" ) ;
      }
    }
  } while ( ww == 0 ) ;
#endif

  for ( int i = 0 ; texels [ i ] != NULL ; i++ )
  {
    int w = xsize>>i ;
    int h = ysize>>i ;

    if ( w <= 0 ) w = 1 ;
    if ( h <= 0 ) h = 1 ;

    total_texels_loaded += w * h ;

    glTexImage2D  ( GL_TEXTURE_2D,
                     map_level, zsize, w, h, FALSE /* Border */,
                            (zsize==1)?GL_LUMINANCE:
                            (zsize==2)?GL_LUMINANCE_ALPHA:
                            (zsize==3)?GL_RGB:
                                       GL_RGBA,
                            GL_UNSIGNED_BYTE, (GLvoid *) texels[i] ) ;
    map_level++ ;
    delete [] texels [ i ] ;
  }

  return true ;
}


static void ssgLoadDummyTexture ( ssgTextureInfo* info )
{
  GLubyte *image = new GLubyte [ 2 * 2 * 3 ] ;

  /* Red and white chequerboard */

  image [ 0 ] = 255 ; image [ 1 ] =  0  ; image [ 2 ] =  0  ;
  image [ 3 ] = 255 ; image [ 4 ] = 255 ; image [ 5 ] = 255 ;
  image [ 6 ] = 255 ; image [ 7 ] = 255 ; image [ 8 ] = 255 ;
  image [ 9 ] = 255 ; image [ 10] =  0  ; image [ 11] =  0  ;

  if ( info != NULL )
  {
    info -> width = 2 ;
    info -> height = 2 ;
    info -> depth = 3 ;
    info -> alpha = 0 ;
  }

  ssgMakeMipMaps ( image, 2, 2, 3 ) ;
}


void ssgAddTextureFormat ( const char* extension,
          bool (*loadfunc) (const char*, ssgTextureInfo* info) )
{
  for ( int i = 0 ; i < num_formats ; i++ )
  {
    if ( ulStrEqual ( formats [ i ] . extension, extension ) )
    {
      formats [ i ] . extension = extension ;
      formats [ i ] . loadfunc = loadfunc ;
      return ;
    }
  }

  if ( num_formats < MAX_FORMATS )
  {
    formats [ num_formats ] . extension = extension ;
    formats [ num_formats ] . loadfunc = loadfunc ;
    num_formats ++ ;
  }
  else
  {
    ulSetError ( UL_WARNING, "ssgAddTextureFormat: too many formats" );
  }
}

bool ssgConvertTexture( char * fname_output, const char * fname_input ) 
// converts file to .rgb (Silicon Graphics) format
// returns true if the file has been converted to rgb, or already exists as rgb
// if it returns false, then it has already output an error message
{
	char *extension ;

	strcpy( fname_output, fname_input); // copy so that a) there is enough room for .rgb and b) we don't change the buffer of fname_input
	extension = strrchr(fname_output, '.');
	if ( extension == NULL )
	{
		ulSetError(UL_WARNING, "There is no extension in the texture '%s'.", fname_input);
		return false; // no extension -> give up
	}
	extension[1] = 'r';
	extension[2] = 'g';
	extension[3] = 'b';
	extension[4] = 0;

	if ( ulFileExists ( fname_output ) )
		return true; // found *.rgb-file

	// look for original, non-rgb - file
	if ( !ulFileExists ( fname_input ) )
	{
		ulSetError(UL_WARNING, "Can't find the texture file '%s'.", fname_input);
		return false; // no input file => we can't convert it
	}

	// ****** found original file. convert it. ******
#ifdef WIN32
  char command [ 1024 ] ;
	sprintf(command, "imconvert -verbose %s sgi:%s", fname_input, fname_output);
	unsigned int ui = WinExec(command, SW_HIDE );	
	if ( ui < 32 )
	{	ulSetError(UL_WARNING, "Couldn't convert texture '%s'. Did you install ImageMagick?"
		                       " You may also convert it manually to '%s' and reload the model.", 
													 fname_input, fname_output);
		return false;
	}
#else
	ulSetError(UL_WARNING, "Converting textures not yet implemented under Linux."
		                     "You may convert '%s' manually to '%s' and reload the model.", 
													 fname_input, fname_output);
  //sprintf(command, "-verbose %s sgi:%s", fname_input, fname_output);
	//execlp ( "convert", "convert",  command, NULL ) ;

#endif
	return true;
}



bool ssgLoadTexture ( const char * filename, ssgTextureInfo* info )
{
  if ( info != NULL )
  {
    info -> width = 0 ;
    info -> height = 0 ;
    info -> depth = 0 ;
    info -> alpha = 0 ;
  }

  if ( filename == NULL || *filename == '\0' )
    return false ;

  //find extension
  const char *extn = & ( filename [ strlen(filename) ] ) ;
  while ( extn != filename && *extn != '/' && *extn != '.' )
    extn-- ;

  if ( *extn != '.' )
  {
    ulSetError ( UL_WARNING, "ssgLoadTexture: Cannot determine file type for '%s'", filename );
    ssgLoadDummyTexture ( info ) ;
    return false ;
  }

  _ssgTextureFormat *f = formats ;
  for ( int i=0; i<num_formats; i++, f++ )
  {
    if ( f->loadfunc != NULL &&
         ulStrNEqual ( extn, f->extension, strlen(f->extension) ) )
    {
      if ( f->loadfunc( filename, info ) )
        return true ;

      ssgLoadDummyTexture ( info ) ; /* fail */
      return false ;
    }
  }
#ifdef SSG_LOAD_SGI_SUPPORTED
	char * fname_output = new char [ strlen(filename) + 4 ]; // +4 as reserve for .rgb
	if ( ssgConvertTexture( fname_output, filename) )
		if ( ssgLoadSGI ( fname_output, info ) )
		{ delete [] fname_output;
			return true;
		}
	delete [] fname_output;

#else
  ulSetError ( UL_WARNING, "ssgLoadTexture: Unrecognised file type '%s'", extn ) ;
#endif

	ssgLoadDummyTexture ( info ) ;
  return false ;
}


