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

UL_RTTI_DEF1(puArrowButton,puButton)


void puDrawArrowButtonTriangle ( int pos_x, int pos_y, int size_x, int size_y,
                                 puColour colour, int arrow_type, int active )
{
  /*
    If greyed out then halve the opacity when drawing
    the widget
  */

  if ( active )
    glColor4fv ( colour ) ;
  else
    glColor4f ( colour [0],
                colour [1],
                colour [2],
                colour [3] / 2.0f ) ; // 50% more transparent

  switch ( arrow_type )
  {
  case PUARROW_UP :
    glBegin    ( GL_TRIANGLES ) ;
	glVertex2i ( pos_x - size_x/4, pos_y - size_y/4 ) ;
	glVertex2i ( pos_x           , pos_y + size_y/4 ) ;
	glVertex2i ( pos_x + size_x/4, pos_y - size_y/4 ) ;
    glEnd      () ;
    break;

  case PUARROW_DOWN :
    glBegin    ( GL_TRIANGLES ) ;
	glVertex2i ( pos_x - size_x/4, pos_y + size_y/4 ) ;
	glVertex2i ( pos_x           , pos_y - size_y/4 ) ;
	glVertex2i ( pos_x + size_x/4, pos_y + size_y/4 ) ;
    glEnd      () ;
    break;

  case PUARROW_FASTUP :
    glBegin    ( GL_TRIANGLES ) ;
	glVertex2i ( pos_x - size_x/4, pos_y - size_y/4 ) ;
	glVertex2i ( pos_x           , pos_y            ) ;
	glVertex2i ( pos_x + size_x/4, pos_y - size_y/4 ) ;

	glVertex2i ( pos_x - size_x/4, pos_y            ) ;
	glVertex2i ( pos_x           , pos_y + size_y/4 ) ;
	glVertex2i ( pos_x + size_x/4, pos_y            ) ;
    glEnd      () ;
    break;

  case PUARROW_FASTDOWN :
    glBegin    ( GL_TRIANGLES ) ;
	glVertex2i ( pos_x - size_x/4, pos_y + size_y/4 ) ;
	glVertex2i ( pos_x           , pos_y            ) ;
	glVertex2i ( pos_x + size_x/4, pos_y + size_y/4 ) ;

	glVertex2i ( pos_x - size_x/4, pos_y            ) ;
	glVertex2i ( pos_x           , pos_y - size_y/4 ) ;
	glVertex2i ( pos_x + size_x/4, pos_y            ) ;
    glEnd      () ;
    break;

  case PUARROW_LEFT :
    glBegin    ( GL_TRIANGLES ) ;
	glVertex2i ( pos_x + size_x/4, pos_y - size_y/4 ) ;
	glVertex2i ( pos_x - size_x/4, pos_y            ) ;
	glVertex2i ( pos_x + size_x/4, pos_y + size_y/4 ) ;
    glEnd      () ;
    break;

  case PUARROW_RIGHT :
    glBegin    ( GL_TRIANGLES ) ;
	glVertex2i ( pos_x - size_x/4, pos_y - size_y/4 ) ;
	glVertex2i ( pos_x + size_x/4, pos_y            ) ;
	glVertex2i ( pos_x - size_x/4, pos_y + size_y/4 ) ;
    glEnd      () ;
    break;

  case PUARROW_FASTLEFT :
    glBegin    ( GL_TRIANGLES ) ;
	glVertex2i ( pos_x + size_x/4, pos_y - size_y/4 ) ;
	glVertex2i ( pos_x           , pos_y            ) ;
	glVertex2i ( pos_x + size_x/4, pos_y + size_y/4 ) ;

	glVertex2i ( pos_x           , pos_y - size_y/4 ) ;
	glVertex2i ( pos_x - size_x/4, pos_y            ) ;
	glVertex2i ( pos_x           , pos_y + size_y/4 ) ;
    glEnd      () ;
    break;

  case PUARROW_FASTRIGHT :
    glBegin    ( GL_TRIANGLES ) ;
	glVertex2i ( pos_x - size_x/4, pos_y - size_y/4 ) ;
	glVertex2i ( pos_x           , pos_y            ) ;
	glVertex2i ( pos_x - size_x/4, pos_y + size_y/4 ) ;

	glVertex2i ( pos_x           , pos_y - size_y/4 ) ;
	glVertex2i ( pos_x + size_x/4, pos_y            ) ;
	glVertex2i ( pos_x           , pos_y + size_y/4 ) ;
    glEnd      () ;
    break;

  default :
    ulSetError ( UL_WARNING, "PUI: Unrecognised 'arrow_type' %d", arrow_type ) ;
    break;
  }
}

void puArrowButton::draw ( int dx, int dy )
{
  if ( !visible || ( window != puGetWindow () ) ) return ;

  /*
    If arrow button is pushed or highlighted -
    use inverse style for button itself
  */

  if ( getIntegerValue() ^ highlighted )
  {
    if ( parent && ( ( parent->getType() & PUCLASS_POPUPMENU ) ||
                     ( parent->getType() & PUCLASS_MENUBAR   ) ) )
      abox.draw ( dx, dy, PUSTYLE_SMALL_SHADED, colour, isReturnDefault(), 2 ) ;
    else
      abox.draw ( dx, dy, -style, colour, isReturnDefault(), border_thickness ) ;
  }
  else
    abox.draw ( dx, dy, style, colour, isReturnDefault(), border_thickness ) ;

  if ( r_cb )
    r_cb ( this, dx, dy, render_data ) ;
  else
  {
    int size_x = abox.max[0] - abox.min[0] ;
    int size_y = abox.max[1] - abox.min[1] ;

    int pos_x = dx + ( abox.max[0] + abox.min[0] ) / 2 ;
    int pos_y = dy + ( abox.max[1] + abox.min[1] ) / 2 ;

    puDrawArrowButtonTriangle ( pos_x, pos_y, size_x, size_y, colour [ PUCOL_MISC ], arrow_type, active ) ;
  }

  draw_label ( dx, dy ) ;
}


