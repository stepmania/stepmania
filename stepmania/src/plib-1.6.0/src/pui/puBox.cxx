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


#include "puLocal.h"

#define PU_DFLT_OFFSET        8

void puBox::extend ( puBox *bx )
{
  if ( bx -> isEmpty () ) return ;

  if ( min[0]>bx->min[0] ) min[0] = bx->min[0] ;
  if ( min[1]>bx->min[1] ) min[1] = bx->min[1] ;
  if ( max[0]<bx->max[0] ) max[0] = bx->max[0] ;
  if ( max[1]<bx->max[1] ) max[1] = bx->max[1] ;
}
 

 
void puBox::draw ( int dx, int dy, int style, puColour colour[], int am_default, int border )
{
  int mid = PUCOL_FOREGROUND ;
  int hi  = PUCOL_HIGHLIGHT  ;
  int lo  = PUCOL_BACKGROUND ;

  /* Colour assignments */

  switch ( style )
  {
    case  PUSTYLE_NONE       :
      return ;

    case  PUSTYLE_PLAIN      :
    case  PUSTYLE_DROPSHADOW :
      mid = PUCOL_FOREGROUND ;
      lo  = PUCOL_BACKGROUND ;
      break ;

    case  PUSTYLE_SMALL_SHADED :
    case  PUSTYLE_SHADED     :
    case  PUSTYLE_SMALL_BEVELLED :
    case  PUSTYLE_BEVELLED   :
    case  PUSTYLE_BOXED      :
    case  PUSTYLE_SPECIAL_UNDERLINED :
      mid = PUCOL_FOREGROUND ;
      hi  = PUCOL_HIGHLIGHT  ;
      lo  = PUCOL_BACKGROUND ;
      break ;

    case  PUSTYLE_RADIO      :
      hi  = PUCOL_HIGHLIGHT  ;
      lo  = PUCOL_LEGEND     ;
      break ;

    case -PUSTYLE_RADIO      :
      hi  = PUCOL_HIGHLIGHT  ;
      lo  = PUCOL_BACKGROUND ;
      break ;

    case -PUSTYLE_PLAIN      :
    case -PUSTYLE_DROPSHADOW :
      mid = PUCOL_HIGHLIGHT  ;
      lo  = PUCOL_BACKGROUND ;
      break ;

    case -PUSTYLE_SMALL_BEVELLED :
    case -PUSTYLE_BEVELLED   :
    case -PUSTYLE_SMALL_SHADED :
    case -PUSTYLE_SHADED     :
    case -PUSTYLE_BOXED      :
    case -PUSTYLE_SPECIAL_UNDERLINED :
      mid = PUCOL_FOREGROUND ;
      hi  = PUCOL_BACKGROUND ;
      lo  = PUCOL_HIGHLIGHT  ;
      break ;

    default :
      ulSetError ( UL_WARNING, "PUI: Unrecognised 'style' %d", style ) ;
      return ;
  }

  switch ( abs(style) )
  {
    case  PUSTYLE_PLAIN      :
      glColor4fv ( colour [ mid ] ) ;
      glRecti ( dx + min[0], dy + min[1],
		dx + max[0], dy + max[1] ) ;
      break ;

    case  PUSTYLE_SMALL_BEVELLED   :
    case  PUSTYLE_SMALL_SHADED     :
    case  PUSTYLE_BEVELLED         :
    case  PUSTYLE_SHADED           :
      glColor4fv ( colour [ hi  ] ) ;
      glBegin ( GL_QUAD_STRIP ) ;
	glVertex2i ( dx + min[0] + border, dy + min[1] + border ) ;
	glVertex2i ( dx + min[0], dy + min[1] ) ;
	glVertex2i ( dx + min[0] + border, dy + max[1] - border ) ;
	glVertex2i ( dx + min[0], dy + max[1] ) ;
	glVertex2i ( dx + max[0] - border, dy + max[1] - border ) ;
	glVertex2i ( dx + max[0], dy + max[1] ) ;
      glEnd () ;
	glColor4fv ( colour [ lo  ] ) ;
      glBegin ( GL_QUAD_STRIP ) ;
	glVertex2i ( dx + min[0], dy + min[1] ) ;
	glVertex2i ( dx + min[0] + border, dy + min[1] + border ) ;
	glVertex2i ( dx + max[0], dy + min[1] ) ;
	glVertex2i ( dx + max[0] - border, dy + min[1] + border ) ;
	glVertex2i ( dx + max[0], dy + max[1] ) ;
	glVertex2i ( dx + max[0] - border, dy + max[1] - border ) ;
      glEnd () ;

      if ( ( abs(style) == PUSTYLE_SMALL_BEVELLED ) ||
           ( abs(style) == PUSTYLE_BEVELLED ) )
      {
        glColor4fv ( colour [ mid ] ) ;
        glRecti ( dx + min[0] + border, dy + min[1] + border,
		  dx + max[0] - border, dy + max[1] - border ) ;
      }
      else
      {
	glShadeModel ( GL_SMOOTH ) ;
	glBegin ( GL_POLYGON ) ;
	  glColor4fv ( colour [ mid ] ) ;
	  glVertex2i ( dx + min[0] + border, dy + min[1] + border ) ;
	  if ( ( style == PUSTYLE_SMALL_SHADED ) ||
               ( style == PUSTYLE_SHADED ) )
	    glColor4f ( colour [mid][0] + (colour[lo][0] - colour[mid][0])/2.0f,
		        colour [mid][1] + (colour[lo][1] - colour[mid][1])/2.0f,
		        colour [mid][2] + (colour[lo][2] - colour[mid][2])/2.0f,
		        colour [lo][3] ) ;
	  else
	    glColor4f ( colour [mid][0] + (colour[hi][0] - colour[mid][0])/2.0f,
		        colour [mid][1] + (colour[hi][1] - colour[mid][1])/2.0f,
		        colour [mid][2] + (colour[hi][2] - colour[mid][2])/2.0f,
		        colour [hi][3] ) ;
	  glVertex2i ( dx + min[0] + border, dy + max[1] - border ) ;
	  glColor4fv ( colour [ mid ] ) ;
	  glVertex2i ( dx + max[0] - border, dy + max[1] - border ) ;
	  if ( ( style == -PUSTYLE_SMALL_SHADED ) ||
               ( style == -PUSTYLE_SHADED ) )
	    glColor4f ( colour [mid][0] + (colour[lo][0] - colour[mid][0])/2.0f,
		        colour [mid][1] + (colour[lo][1] - colour[mid][1])/2.0f,
		        colour [mid][2] + (colour[lo][2] - colour[mid][2])/2.0f,
		        colour [lo][3] ) ;
	  else
	    glColor4f ( colour [mid][0] + (colour[hi][0] - colour[mid][0])/2.0f,
		        colour [mid][1] + (colour[hi][1] - colour[mid][1])/2.0f,
		        colour [mid][2] + (colour[hi][2] - colour[mid][2])/2.0f,
		        colour [hi][3] ) ;
	  glVertex2i ( dx + max[0] - border, dy + min[1] + border ) ;
	glEnd () ;
	glShadeModel (GL_FLAT) ;

	if ( ( style == -PUSTYLE_SMALL_SHADED ) ||
             ( style == -PUSTYLE_SHADED ) )
	{
	  glColor4fv ( colour [ lo  ] ) ;
	  glBegin ( GL_QUAD_STRIP ) ;
	    glVertex2i ( dx + min[0] + border   , dy + min[1] + border   ) ;
	    glVertex2i ( dx + min[0] + border/2 , dy + min[1] + border/2 ) ;
	    glVertex2i ( dx + min[0] + border   , dy + max[1] - border   ) ;
	    glVertex2i ( dx + min[0] + border/2 , dy + max[1] - border/2 ) ;
	    glVertex2i ( dx + max[0] - border   , dy + max[1] - border   ) ;
	    glVertex2i ( dx + max[0] - border/2 , dy + max[1] - border/2 ) ;
	  glEnd () ;
	  glColor4fv ( colour [ hi  ] ) ;
	  glBegin ( GL_QUAD_STRIP ) ;
	    glVertex2i ( dx + min[0] + border/2 , dy + min[1] + border/2 ) ;
	    glVertex2i ( dx + min[0] + border   , dy + min[1] + border   ) ;
	    glVertex2i ( dx + max[0] - border/2 , dy + min[1] + border/2 ) ;
	    glVertex2i ( dx + max[0] - border   , dy + min[1] + border   ) ;
	    glVertex2i ( dx + max[0] - border/2 , dy + max[1] - border/2 ) ;
	    glVertex2i ( dx + max[0] - border   , dy + max[1] - border   ) ;
	  glEnd () ;
	}	
      }
      break ;

    case  PUSTYLE_BOXED      :
      glColor4fv ( colour [ hi  ] ) ;
      glRecti ( dx + min[0], dy + min[1],
		dx + max[0], dy + max[1] ) ;
      glColor4fv ( colour [ mid ] ) ;
      glRecti ( dx + min[0]+border, dy + min[1]+border,
		dx + max[0]-border, dy + max[1]-border ) ;
      break ;

    case  PUSTYLE_RADIO      :
      glColor4fv ( colour [ lo  ] ) ;
      glBegin ( GL_LINE_LOOP ) ;
        glVertex2i ( dx + min[0] + PU_RADIO_BUTTON_SIZE/2, dy + min[1]                          ) ;
        glVertex2i ( dx + min[0] + PU_RADIO_BUTTON_SIZE  , dy + min[1] + PU_RADIO_BUTTON_SIZE/2 ) ;
        glVertex2i ( dx + min[0] + PU_RADIO_BUTTON_SIZE/2, dy + min[1] + PU_RADIO_BUTTON_SIZE   ) ;
        glVertex2i ( dx + min[0]                         , dy + min[1] + PU_RADIO_BUTTON_SIZE/2 ) ;
      glEnd () ;

      if ( style < 0 )
      {
	glColor4fv ( colour [ hi  ] ) ;
	glBegin ( GL_QUADS ) ;
	  glVertex2i ( dx + min[0] + PU_RADIO_BUTTON_SIZE/2, dy + min[1] + 2                      ) ;
	  glVertex2i ( dx + min[0] + PU_RADIO_BUTTON_SIZE-2, dy + min[1] + PU_RADIO_BUTTON_SIZE/2 ) ;
	  glVertex2i ( dx + min[0] + PU_RADIO_BUTTON_SIZE/2, dy + min[1] + PU_RADIO_BUTTON_SIZE-2 ) ;
	  glVertex2i ( dx + min[0] + 2                     , dy + min[1] + PU_RADIO_BUTTON_SIZE/2 ) ;
	glEnd () ;
      }
      break ;

    case  PUSTYLE_SPECIAL_UNDERLINED :
      glColor4fv ( colour [ hi  ] ) ;
      glRecti ( dx + min[0], dy + min[1],
		dx + max[0], dy + min[1]+border ) ;
      glColor4fv ( colour [ mid ] ) ;
      glRecti ( dx + min[0], dy + min[1]+border,
		dx + max[0], dy + max[1] ) ;
      break ;

    case  PUSTYLE_DROPSHADOW :
      glColor4fv ( colour [ lo  ] ) ;
      glRecti ( dx + min[0] + border, dy + min[1] - border,
		dx + max[0] + border, dy + max[1] - border ) ;
      glColor4fv ( colour [ mid ] ) ;
      glRecti ( dx + min[0], dy + min[1],
		dx + max[0], dy + max[1] ) ;
      break ;
  }

  if ( am_default )
  {
    glColor4fv ( colour [ PUCOL_BACKGROUND ] ) ;
    glLineStipple ( 1, 0xF0F0 ) ;
    glEnable ( GL_LINE_STIPPLE ) ;
    glBegin ( GL_LINE_LOOP ) ;
      glVertex2i ( dx + min[0] + PU_DFLT_OFFSET, dy + min[1] + PU_DFLT_OFFSET ) ;
      glVertex2i ( dx + min[0] + PU_DFLT_OFFSET, dy + max[1] - PU_DFLT_OFFSET ) ;
      glVertex2i ( dx + max[0] - PU_DFLT_OFFSET, dy + max[1] - PU_DFLT_OFFSET ) ;
      glVertex2i ( dx + max[0] - PU_DFLT_OFFSET, dy + min[1] + PU_DFLT_OFFSET ) ;
    glEnd () ;
    glDisable ( GL_LINE_STIPPLE ) ;
  }
}



