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

#ifdef SSG_LOAD_PNG_SUPPORTED

#include <gl/glpng.h>

bool ssgLoadPNG ( const char *fname, ssgTextureInfo* info )
{
  pngInfo png_info;
  if (!pngLoad(fname, PNG_BUILDMIPMAP, PNG_ALPHA, &png_info)) {
    ulSetError ( UL_WARNING, "ssgLoadTexture: Failed to load '%s'.", fname ) ;
    return false ;
  }
  if ( info != NULL )
  {
    info -> width = png_info.Width ;
    info -> height = png_info.Height ;
    info -> depth = png_info.Depth ;
    info -> alpha = png_info.Alpha ;
  }
  return true ;
}

#else

bool ssgLoadPNG ( const char *fname, ssgTextureInfo* info )
{
  ulSetError ( UL_WARNING, "ssgLoadTexture: '%s' - you need glpng for PNG format support",
        fname ) ;
  return false ;
}

#endif
