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

#ifndef _INCLUDED_SSGCONF_H_
#define _INCLUDED_SSGCONF_H_

/* ssgconf.h - machine configurable file for libssg */

#define _SSG_USE_DLIST  1

/* The following defines give you the ability to remove code from the
 * library that you will not be using.  I wish I could figure out how to
 * automate this, but I can't do that without making it seriously hard
 * on the users.  So if you are not using an ability, change the #define
 * to and #undef, and that part of the library will not be compiled.  If
 * your linker can't find a function, you may want to make sure the
 * ability is defined here.
 */

/*
  For optional use of PNG textures, download the glpng library from
  http://www.wyatt100.freeserve.co.uk/download.htm and un-comment
  the following line.
 */

#undef  SSG_LOAD_PNG_SUPPORTED

#define SSG_LOAD_SGI_SUPPORTED
#define SSG_LOAD_TGA_SUPPORTED
#define SSG_LOAD_BMP_SUPPORTED
#define SSG_LOAD_MDL_SUPPORTED
#define SSG_LOAD_MDL_BGL_TEXTURE_SUPPORTED
#endif
