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

#include "slMODPrivate.h"

static int stereo ;
static int glob_vol = 0x40 * 0x40 ; /* default g.v = m.v = 64 */

#define VOL_MAX        (64*64*128)    /* vol max * g.v max * m.v max */
#define VOL_MAX_LOG    ( 6 + 6 + 7 )  /* 1 << VOL_MAX_LOG == VOL_MAX */
#define LEV_MAX        (128*VOL_MAX)
#define MAX_FRAMELEN   (MAX_OUTRATE * 60 / (MIN_TEMPO * 24))

static inline unsigned char to8bit ( int x )
{
  int tmpvar = ( x * glob_vol + LEV_MAX ) >> VOL_MAX_LOG ;
 
  return (tmpvar & ~255) ? ~tmpvar >> 16 : tmpvar ;
}


struct ucharBuffer
{
  unsigned char data [ MAX_FRAMELEN * 4 ] ;
  unsigned char *ptr ;
  unsigned char *end ;
  int size ;

public:

  ucharBuffer ()
  {
    ptr = data ;
    size = sizeof ( data ) ;
    end = data + size ;
  }

  void copyFrom ( int *inbufp, int n )
  {
    if ( n == 0 )
      return ;

    if ( stereo )
      for (; n > 0; n--)
      {
	*ptr++ = to8bit ( *inbufp++ ) ; /* L */
	*ptr++ = to8bit ( *inbufp++ ) ; /* R */
      }
    else
      for (; n > 0; n--)
      {
	*ptr++ = to8bit ( *inbufp ) ;
	inbufp += 2 ;
      }
  }

  int getLen () { return ptr - data ; }

  unsigned char *getBuffer () { return data ; }

  void empty ()
  {
    ptr = data ;
  }

  void subtract ( int n )
  {
    /* Delete n bytes from the front of the buffer */

    /* WARNING - NOT memcpy OK? */
    memmove ( data, data+n, ptr - data - n ) ;
    ptr -= n ;
  }
} ;


struct intBuffer
{
  int data [ MAX_FRAMELEN * 2 ] ;
  int len  ;
} ;


static intBuffer    inbuf ;
static ucharBuffer outbuf ;

int *dacioGetBuffer ()
{
  return inbuf.data ;
}

void dacioIncomingBufLen ( int len ) { inbuf.len  = len ; } 
void dacioGlobalVol(int v) { glob_vol = v; } 
int  dacioGetLen () { return outbuf.getLen () ; }
void dacioOut ( void ) { outbuf.copyFrom ( inbuf.data, inbuf.len ) ; }
void dacioEmpty () { outbuf.empty (); }
void dacioSubtract ( int n ) { outbuf.subtract(n) ; }
unsigned char *dacioGetOutBuffer () { return outbuf.getBuffer() ; }
