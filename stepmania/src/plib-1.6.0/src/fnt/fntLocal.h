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


#include "fnt.h"


extern int   isSwapped     ;
extern FILE *curr_image_fd ;


inline void _fnt_swab_short ( unsigned short *x )
{
  if ( isSwapped )
    *x = (( *x >>  8 ) & 0x00FF ) | 
         (( *x <<  8 ) & 0xFF00 ) ;
}


inline void _fnt_swab_int ( unsigned int *x )
{
  if ( isSwapped )
    *x = (( *x >> 24 ) & 0x000000FF ) | 
         (( *x >>  8 ) & 0x0000FF00 ) | 
         (( *x <<  8 ) & 0x00FF0000 ) | 
         (( *x << 24 ) & 0xFF000000 ) ;
}


inline void _fnt_swab_int_array ( int *x, int leng )
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
}


inline unsigned char _fnt_readByte ()
{
  unsigned char x ;
  fread ( & x, sizeof(unsigned char), 1, curr_image_fd ) ;
  return x ;
}

inline unsigned short _fnt_readShort ()
{
  unsigned short x ;
  fread ( & x, sizeof(unsigned short), 1, curr_image_fd ) ;
  _fnt_swab_short ( & x ) ;
  return x ;
}

inline unsigned int _fnt_readInt ()
{
  unsigned int x ;
  fread ( & x, sizeof(unsigned int), 1, curr_image_fd ) ;
  _fnt_swab_int ( & x ) ;
  return x ;
}


#define FNT_BYTE_FORMAT		0
#define FNT_BITMAP_FORMAT	1

struct TXF_Glyph
{
  unsigned short ch ;
  unsigned char  w  ;
  unsigned char  h  ;
  signed char x_off ;
  signed char y_off ;
  signed char step  ;
  signed char unknown ;
  short x ;
  short y ;

  sgVec2 tx0 ; sgVec2 vx0 ;
  sgVec2 tx1 ; sgVec2 vx1 ;
  sgVec2 tx2 ; sgVec2 vx2 ;
  sgVec2 tx3 ; sgVec2 vx3 ;
}  ;


