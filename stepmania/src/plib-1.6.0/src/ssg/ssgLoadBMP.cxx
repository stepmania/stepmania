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

#ifdef SSG_LOAD_BMP_SUPPORTED

static FILE          *curr_image_fd ;
static char           curr_image_fname [ 512 ] ;
static int            isSwapped ;


static void swab_short ( unsigned short *x )
{
  if ( isSwapped )
    *x = (( *x >>  8 ) & 0x00FF ) | 
         (( *x <<  8 ) & 0xFF00 ) ;
}

static void swab_int ( unsigned int *x )
{
  if ( isSwapped )
    *x = (( *x >> 24 ) & 0x000000FF ) | 
         (( *x >>  8 ) & 0x0000FF00 ) | 
         (( *x <<  8 ) & 0x00FF0000 ) | 
         (( *x << 24 ) & 0xFF000000 ) ;
}

/*static void swab_int_array ( int *x, int leng )
{
  if ( ! isSwapped )
    return ;

  for ( int i = 0 ; i < leng ; i++ )
  {
    *x = (( *x >> 24 ) & 0x000000FF ) | 
         (( *x >>  8 ) & 0x0000FF00 ) | 
         (( *x <<  8 ) & 0x00FF0000 ) | 
         (( *x << 24 ) & 0xFF000000 ) ;
    x++ ;
  }
}*/


static unsigned char readByte ()
{
  unsigned char x ;
  fread ( & x, sizeof(unsigned char), 1, curr_image_fd ) ;
  return x ;
}

static unsigned short readShort ()
{
  unsigned short x ;
  fread ( & x, sizeof(unsigned short), 1, curr_image_fd ) ;
  swab_short ( & x ) ;
  return x ;
}

static unsigned int readInt ()
{
  unsigned int x ;
  fread ( & x, sizeof(unsigned int), 1, curr_image_fd ) ;
  swab_int ( & x ) ;
  return x ;
}


/*
  Original source for BMP loader kindly
  donated by "Sean L. Palmer" <spalmer@pobox.com>
*/


struct BMPHeader
{
  unsigned short  FileType      ;
  unsigned int    FileSize      ;
  unsigned short  Reserved1     ;
  unsigned short  Reserved2     ;
  unsigned int    OffBits       ;
  unsigned int    Size          ;
  unsigned int    Width         ;
  unsigned int    Height        ;
  unsigned short  Planes        ;
  unsigned short  BitCount      ;
  unsigned int    Compression   ;
  unsigned int    SizeImage     ;
  unsigned int    XPelsPerMeter ;
  unsigned int    YPelsPerMeter ;
  unsigned int    ClrUsed       ;
  unsigned int    ClrImportant  ;
} ;


struct RGBA
{
  unsigned char r,g,b,a ;
} ;


bool ssgLoadBMP ( const char *fname, ssgTextureInfo* info )
{
  int w, h, bpp ;
  int index=0;
  bool old_format = false;
  RGBA pal [ 256 ] ;

  BMPHeader bmphdr ;

  /* Open file & get size */

  strcpy ( curr_image_fname, fname ) ;

  if ( ( curr_image_fd = fopen ( curr_image_fname, "rb" ) ) == NULL )
  {
    char *p = strrchr(curr_image_fname,'_');
    if (p != 0) {
      *p = '\0';
      p++;
      index = atoi (p);
      old_format = true;
      if ( ( curr_image_fd = fopen(curr_image_fname, "rb")) == NULL) {
        perror ( "ssgLoadTexture" ) ;
        ulSetError( UL_WARNING, "ssgLoadTexture: Failed to load '%s' for reading.", curr_image_fname );
        return false ;
      }
      p--;
      *p = '_';
    }
    else {
      perror ( "ssgLoadTexture" ) ;
      ulSetError( UL_WARNING, "ssgLoadTexture: Failed to open '%s' for reading.", curr_image_fname );
      return false ;
    }
  }

  /*
    Load the BMP piecemeal to avoid struct packing issues
  */

  isSwapped = FALSE ;
  bmphdr.FileType = readShort () ;

  if ( bmphdr.FileType == ((int)'B' + ((int)'M'<<8)) )
    isSwapped = FALSE ;
  else
  if ( bmphdr.FileType == ((int)'M' + ((int)'B'<<8)) )
    isSwapped = TRUE  ;
  else
  {
    ulSetError ( UL_WARNING, "%s: Unrecognised magic number 0x%04x",
                            curr_image_fname, bmphdr.FileType ) ;
    return false ;
  }

  bmphdr.FileSize      = readInt   () ;
  bmphdr.Reserved1     = readShort () ;
  bmphdr.Reserved2     = readShort () ;
  bmphdr.OffBits       = readInt   () ;
  bmphdr.Size          = readInt   () ;
  bmphdr.Width         = readInt   () ;
  bmphdr.Height        = readInt   () ;
  bmphdr.Planes        = readShort () ;
  bmphdr.BitCount      = readShort () ;
  bmphdr.Compression   = readInt   () ;
  bmphdr.SizeImage     = readInt   () ;
  bmphdr.XPelsPerMeter = readInt   () ;
  bmphdr.YPelsPerMeter = readInt   () ;
  bmphdr.ClrUsed       = readInt   () ;
  bmphdr.ClrImportant  = readInt   () ;
 
  w   = bmphdr.Width  ;
  h   = bmphdr.Height ;
  bpp = bmphdr.Planes * bmphdr.BitCount ;

#ifdef PRINT_BMP_HEADER_DEBUG
  ulSetError ( UL_DEBUG, "Filetype %04x",      bmphdr.FileType      ) ;
  ulSetError ( UL_DEBUG, "Filesize %08x",      bmphdr.FileSize      ) ;
  ulSetError ( UL_DEBUG, "R1 %04x",            bmphdr.Reserved1     ) ;
  ulSetError ( UL_DEBUG, "R2 %04x",            bmphdr.Reserved2     ) ;
  ulSetError ( UL_DEBUG, "Offbits %08x",       bmphdr.OffBits       ) ;
  ulSetError ( UL_DEBUG, "Size %08x",          bmphdr.Size          ) ;
  ulSetError ( UL_DEBUG, "Width %08x",         bmphdr.Width         ) ;
  ulSetError ( UL_DEBUG, "Height %08x",        bmphdr.Height        ) ;
  ulSetError ( UL_DEBUG, "Planes %04x",        bmphdr.Planes        ) ;
  ulSetError ( UL_DEBUG, "Bitcount %04x",      bmphdr.BitCount      ) ;
  ulSetError ( UL_DEBUG, "Compression %08x",   bmphdr.Compression   ) ;
  ulSetError ( UL_DEBUG, "SizeImage %08x",     bmphdr.SizeImage     ) ;
  ulSetError ( UL_DEBUG, "XPelsPerMeter %08x", bmphdr.XPelsPerMeter ) ;
  ulSetError ( UL_DEBUG, "YPelsPerMeter %08x", bmphdr.YPelsPerMeter ) ;
  ulSetError ( UL_DEBUG, "ClrUsed %08x",       bmphdr.ClrUsed       ) ;
  ulSetError ( UL_DEBUG, "ClrImportant %08x",  bmphdr.ClrImportant  ) ;
#endif

  int isMonochrome = TRUE ;
  int isOpaque     = TRUE ;

  if ( bpp <= 8 )
  {
    for ( int i = 0 ; i < 256 ; i++ )
    {
      pal[i].b = readByte () ;
      pal[i].g = readByte () ;
      pal[i].r = readByte () ;

      /* According to BMP specs, this fourth value is not really alpha value
	 but just a filler byte, so it is ignored for now. */
      pal[i].a = readByte () ;
      if (old_format == true) {
        pal[i].a = (i<index)?0:255;

      }

      if ( pal[i].r != pal[i].g ||
           pal[i].g != pal[i].b ) isMonochrome = FALSE ;
    }
  }

  fseek ( curr_image_fd, bmphdr.OffBits, SEEK_SET ) ;

  bmphdr.SizeImage = w * h * (bpp / 8) ;
  GLubyte *data = new GLubyte [ bmphdr.SizeImage ] ;

  /* read and flip image */
  {
    int row_size = w * (bpp / 8) ;
    for ( int y = h-1 ; y >= 0 ; y-- )
    {
      GLubyte *row_ptr = &data [ y * row_size ] ;
      if ( fread ( row_ptr, 1, row_size, curr_image_fd ) != (unsigned)row_size )
      {
        ulSetError ( UL_WARNING, "Premature EOF in '%s'", curr_image_fname ) ;
        return false ;
      }
    }
  }

  fclose ( curr_image_fd ) ;

  GLubyte *image ;
  int z ;

  if ( bpp == 8 )
  {
    int i ;

    // check for diffrent alpha values in the bitmap
    // assume blending if that's the case
    for ( i = 1 ; i < w * h ; i++ ) {
      if  (pal[data[i]].a != pal[data[i-1]].a) {
        isOpaque = FALSE ;
        break;
      }  
    }

    if ( isMonochrome )
      z = isOpaque ? 1 : 2 ;
    else
      z = isOpaque ? 3 : 4 ;

    image = new GLubyte [ w * h * z ] ;

    for ( i = 0 ; i < w * h ; i++ )
      switch ( z )
      {
        case 1 : image [ i       ] = pal[data[i]].r ; break ;
        case 2 : image [ i*2     ] = pal[data[i]].r ;
                 image [ i*2 + 1 ] = pal[data[i]].a ; break ;
        case 3 : image [ i*3     ] = pal[data[i]].r ;
                 image [ i*3 + 1 ] = pal[data[i]].g ;
                 image [ i*3 + 2 ] = pal[data[i]].b ; break ;
        case 4 : image [ i*4     ] = pal[data[i]].r ;
                 image [ i*4 + 1 ] = pal[data[i]].g ;
                 image [ i*4 + 2 ] = pal[data[i]].b ;
                 image [ i*4 + 3 ] = pal[data[i]].a ; break ;
        default : break ;
      }

    delete [] data ;
  }
  else
  if ( bpp == 24 )
  {
    z = 3 ;
    image = data ;

    /* BGR --> RGB */

    for ( int i = 0 ; i < w * h ; i++ )
    {
      GLubyte tmp = image [ 3 * i ] ;
      image [ 3 * i ] = image [ 3 * i + 2 ];
      image [ 3 * i + 2 ] = tmp ;
    }
  }
  else
  if ( bpp == 32 )
  {
    z = 4 ;
    image = data ;

    /* BGRA --> RGBA */

    for ( int i = 0 ; i < w * h ; i++ )
    {
      GLubyte tmp = image [ 4 * i ] ;
      image [ 4 * i ] = image [ 4 * i + 2 ];
      image [ 4 * i + 2 ] = tmp ;
    }
  }
  else
  {
    ulSetError ( UL_WARNING, "ssgLoadTexture: Can't load %d bpp BMP textures.", bpp ) ;
    return false ;
  }

  if ( info != NULL )
  {
    info -> width = w ;
    info -> height = h ;
    info -> depth = z ;
    info -> alpha = ( isOpaque == FALSE ) ;
  }

  return ssgMakeMipMaps ( image, w, h, z ) ;
}

#else

bool ssgLoadBMP ( const char *fname, ssgTextureInfo* info )
{
  ulSetError ( UL_WARNING,
    "ssgLoadTexture: '%s' - BMP support not configured", fname ) ;
  return false ;
}

#endif
