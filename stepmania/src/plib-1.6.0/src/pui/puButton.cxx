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

UL_RTTI_DEF1(puButton,puObject)


void puButton::draw ( int dx, int dy )
{
  if ( !visible || ( window != puGetWindow () ) ) return ;

  if ( button_type == PUBUTTON_NORMAL )
  {
    /* If button is pushed or highlighted - use inverse style */

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
  }
  else if ( ( button_type == PUBUTTON_VCHECK ) ||
            ( button_type == PUBUTTON_XCHECK ) )
    abox.draw ( dx, dy, PUSTYLE_BOXED, colour, isReturnDefault(), 1 ) ;

  if ( r_cb )
    r_cb ( this, dx, dy, render_data ) ;
  else
  {
    switch ( button_type )
    {
      case PUBUTTON_NORMAL :
        draw_legend ( dx, dy ) ;
        break ;

      case PUBUTTON_RADIO :
      {
        int btn_width  = abox.max[0] - abox.min[0],
            btn_height = abox.max[1] - abox.min[1] ;

        glColor4fv ( ( getIntegerValue () ^ highlighted ) ?
                     colour [ PUCOL_BACKGROUND ] : colour [ PUCOL_LEGEND ] ) ;

        glBegin    ( GL_LINE_LOOP ) ;
        glVertex2i ( dx+abox.min[0] + btn_width/2, dy+abox.min[1]                ) ;
        glVertex2i ( dx+abox.min[0] + btn_width,   dy+abox.min[1] + btn_height/2 ) ;
        glVertex2i ( dx+abox.min[0] + btn_width/2, dy+abox.min[1] + btn_height   ) ;
	glVertex2i ( dx+abox.min[0],               dy+abox.min[1] + btn_height/2 ) ;
        glEnd () ;

        if ( getIntegerValue () ^ highlighted )
        {
          /* If greyed out then halve the opacity when drawing the widget */

          if ( active )
            glColor4fv ( colour [ PUCOL_HIGHLIGHT ] ) ;
          else
            glColor4f ( colour [ PUCOL_HIGHLIGHT ][0],
                        colour [ PUCOL_HIGHLIGHT ][1],
                        colour [ PUCOL_HIGHLIGHT ][2],
                        colour [ PUCOL_HIGHLIGHT ][3] / 2.0f ) ; /* 50% more transparent */

          glBegin    ( GL_QUADS ) ;
          glVertex2i ( dx+abox.min[0] + btn_width/2,
                       dy+abox.min[1] + 2 ) ;
          glVertex2i ( dx+abox.min[0] + btn_width - 2,
                       dy+abox.min[1] + btn_height/2 ) ;
          glVertex2i ( dx+abox.min[0] + btn_width/2,
                       dy+abox.min[1] + btn_height - 2 ) ;
          glVertex2i ( dx+abox.min[0] + 2,
                       dy+abox.min[1] + btn_height/2 ) ;
          glEnd () ;
        }

        break ;
      }

      case PUBUTTON_CIRCLE :
      {
        float rad = ( (abox.max[0]-abox.min[0]) < (abox.max[1]-abox.min[1]) ) ?
                    (abox.max[0]-abox.min[0])/2.0f : (abox.max[1]-abox.min[1])/2.0f ;
        float dtheta = 2.0f / rad ;
        float theta ;

        /* Draw the outer circle */

        glColor4fv ( colour [ PUCOL_FOREGROUND ] ) ;

        glBegin ( GL_POLYGON ) ;
        for ( theta = -SG_PI ; theta <= SG_PI ; theta += dtheta )
          glVertex2f ( dx + abox.min[0] + rad + (rad * cos ( theta )),
                       dy + abox.min[1] + rad + (rad * sin ( theta )) ) ;
        glEnd () ;

        if ( getIntegerValue () ^ highlighted )
        /* If clicked, draw the inner circle with half the radius */
        {
          rad /= 2 ;
          dtheta = 2.0f / rad ;

          /* If greyed out then halve the opacity when drawing the widget */

          if ( active )
            glColor4fv ( colour [ PUCOL_MISC ] ) ;
          else
            glColor4f ( colour [ PUCOL_MISC ][0],
                        colour [ PUCOL_MISC ][1],
                        colour [ PUCOL_MISC ][2],
                        colour [ PUCOL_MISC ][3] / 2.0f ) ; /* 50% more transparent */

          glBegin ( GL_POLYGON ) ;
          for ( theta = -SG_PI ; theta <= SG_PI ; theta += dtheta )
            glVertex2f ( dx + abox.min[0] + rad*2 + (rad * cos ( theta )),
                         dy + abox.min[1] + rad*2 + (rad * sin ( theta )) ) ;
          glEnd () ;
        }

        break ;
      }

      case PUBUTTON_VCHECK :
      case PUBUTTON_XCHECK :
        if ( getIntegerValue () ^ highlighted )
        {
          /* If greyed out then halve the opacity when drawing the widget */

          if ( active )
            glColor4fv ( colour [ PUCOL_MISC ] ) ;
          else
            glColor4f ( colour [ PUCOL_MISC ][0],
                        colour [ PUCOL_MISC ][1],
                        colour [ PUCOL_MISC ][2],
                        colour [ PUCOL_MISC ][3] / 2.0f ) ; /* 50% more transparent */

          glPushAttrib ( GL_LINE_BIT ) ;

          glLineWidth ( 2.0f ) ;

          if ( button_type == PUBUTTON_VCHECK )
          {
            glBegin ( GL_LINE_STRIP ) ;
            glVertex2i ( dx + abox.min[0],
                         dy + abox.min[1] + (abox.max[1] - abox.min[1])/2 ) ;
            glVertex2i ( dx + abox.min[0] + (abox.max[0] - abox.min[0])/3,
                         dy + abox.min[1] ) ;
            glVertex2i ( dx + abox.max[0],
                         dy + abox.max[1] ) ;
            glEnd () ;
          }
          else if ( button_type == PUBUTTON_XCHECK )
          {
            glBegin ( GL_LINES ) ;
            glVertex2i ( dx + abox.min[0] + 1, dy + abox.min[1] + 1 ) ;
            glVertex2i ( dx + abox.max[0] - 1, dy + abox.max[1] - 1 ) ;
            glVertex2i ( dx + abox.max[0] - 1, dy + abox.min[1] + 1 ) ;
            glVertex2i ( dx + abox.min[0] + 1, dy + abox.max[1] - 1 ) ;
            glEnd () ;
          }

          glPopAttrib () ;
        }

        break ;

      default :
        ulSetError ( UL_WARNING, "PUI: Unrecognised 'button_type' %d", button_type ) ;
        break;
    }
  }

  draw_label ( dx, dy ) ;
}


void puButton::doHit ( int button, int updown, int x, int y )
{
  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    puActiveWidget() -> invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( updown != PU_DRAG )
    puMoveToLast ( this );

  if ( button == PU_LEFT_BUTTON )
  {
    if ( updown == active_mouse_edge || active_mouse_edge == PU_UP_AND_DOWN )
    {
      lowlight () ;
      setValue ( (int) ! getIntegerValue () ) ;
      puSetActiveWidget ( this, x, y ) ;
      invokeCallback () ;
    }
    else
      highlight () ;
  }
  else
    lowlight () ;
}


