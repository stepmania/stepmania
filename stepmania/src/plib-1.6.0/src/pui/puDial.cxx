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

UL_RTTI_DEF1(puDial,puRange)


void puDial::draw ( int dx, int dy )
{
  if ( !visible || ( window != puGetWindow () ) ) return ;

  // Draw the active box.

  if ( ( style == PUSTYLE_BEVELLED ) ||
       ( style == PUSTYLE_SHADED ) )
    abox.draw ( dx, dy, -PUSTYLE_BOXED, colour, FALSE, 2 ) ;
  else
    abox.draw ( dx, dy, -style, colour, FALSE, border_thickness ) ;

  if ( r_cb )
    r_cb ( this, dx, dy, render_data ) ;
  else
  {
    // If greyed out then halve the opacity when drawing the widget

    if ( active )
      glColor4fv ( colour [ PUCOL_MISC ] ) ;
    else
      glColor4f ( colour [ PUCOL_MISC ][0],
                  colour [ PUCOL_MISC ][1],
                  colour [ PUCOL_MISC ][2],
                  colour [ PUCOL_MISC ][3] / 2.0f ) ; // 50% more transparent

    // Draw the surrounding circle.

    float rad = (float)( abox.max [0] - abox.min [0] ) / 2.0f - 3.0f ;
    int x_cen = dx + ( abox.max [0] + abox.min [0] ) / 2 ;
    int y_cen = dy + ( abox.max [1] + abox.min [1] ) / 2 ;

    float dtheta = 3.0f / rad ;   // three pixels per segment

    glPushAttrib ( GL_LINE_BIT ) ;

    glLineWidth ( 2.0f ) ;   // set line width to two pixels

    glBegin ( GL_LINE_STRIP ) ;

    float theta ;
    for ( theta = -SG_PI; theta < SG_PI+dtheta; theta+= dtheta )
    {
      float x = (float)x_cen + rad * (float)cos ( (double)theta ) ;
      float y = (float)y_cen + rad * (float)sin ( (double)theta ) ;

      glVertex2f ( x, y ) ;
    }

    glEnd () ;

    // Draw the line from the center.

    glLineWidth ( 4.0f ) ;  // four pixels wide

    float val ;
    getValue ( &val ) ;
    val = ( val - minimum_value) / (maximum_value - minimum_value) ;

    if ( val < 0.0 ) val = 0.0 ;
    if ( val > 1.0 ) val = 1.0 ;

    val = ( 2.0f * val - 1.0f ) * SG_PI ;

    glBegin ( GL_LINES ) ;

    glVertex2f ( (float)x_cen, (float)y_cen ) ;
    glVertex2f ( (float)x_cen + rad * (float)sin ( (double)val ), (float)y_cen + rad * (float)cos ( (double)val ) ) ;

    glEnd () ;

    glPopAttrib () ;

    draw_legend ( dx, dy ) ;
  }

  draw_label ( dx, dy ) ;
}


void puDial::doHit ( int button, int updown, int x, int y )
{
  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    puActiveWidget() -> invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( updown != PU_DRAG )
    puMoveToLast ( this );

  if ( button == PU_LEFT_BUTTON && updown == PU_UP )
  {
    puDeactivateWidget () ;
    return ;
  }                                                                             

  if ( button == PU_LEFT_BUTTON )
  {
    int x_cen = ( abox.max [0] + abox.min [0] ) / 2 ;
    int y_cen = ( abox.max [1] + abox.min [1] ) / 2 ;
    float angle = (float)atan2 ( (double)(x-x_cen), (double)(y-y_cen) ) / SG_PI ;

    // Move to within the (0,1) interval

    if ( angle < -1.0f )
      angle += 2.0 ;
    else if (angle > 1.0f )
      angle -= 2.0 ;

    angle = ( angle + 1.0f ) / 2.0f ;

    // Check for hitting the limits (user has dragged the mouse around the bottom
    // of the widget)
    if ( !wrap )
    {
      if ( ( angle > 0.75 ) && ( getFloatValue () < 0.25 ) )
        angle = 0.0 ;
      else if ( ( angle < 0.25 ) && ( getFloatValue () > 0.75 ) )
        angle = 1.0 ;
    }
    
    angle = angle * (maximum_value - minimum_value) + minimum_value ;
    setValue( checkStep(angle) ) ;

    switch ( cb_mode )
    {
      case PUSLIDER_CLICK :
        if ( updown == active_mouse_edge )
        {
	  last_cb_value = angle ;
    puSetActiveWidget ( this, x, y ) ;
	  invokeCallback () ;
        }
        break ;

      case PUSLIDER_DELTA :/* Deprecated! */
        if ( fabs ( last_cb_value - angle ) >= cb_delta )
        {
	  last_cb_value = angle ;
    puSetActiveWidget ( this, x, y ) ;
	  invokeCallback () ;
        }
        break ;

      case PUSLIDER_ALWAYS :
      default :
        last_cb_value = angle ;
        puSetActiveWidget ( this, x, y ) ;
        invokeCallback () ;
        break ;
    }
  }
}


