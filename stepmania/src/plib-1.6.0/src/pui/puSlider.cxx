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

UL_RTTI_DEF1(puSlider,puRange)


void puSlider::draw_slider_box ( int dx, int dy, const puBox &box, float val, const char *box_label )
{
  int sd, od ;
  if ( isVertical() ) { sd = 1 ; od = 0 ; } else { sd = 0 ; od = 1 ; }

  int sz = box.max [sd] - box.min [sd] ;  // Size of slider box, in pixels

  if ( val < 0.0f ) val = 0.0f ;
  if ( val > 1.0f ) val = 1.0f ;

  val *= (float) sz * (1.0f - slider_fraction) ;

  puBox bx ;
    
  bx.min [ sd ] = box.min [ sd ] + (int) val ;
  bx.max [ sd ] = (int) ( (float) bx.min [ sd ] + (float) sz * slider_fraction ) ;
  bx.min [ od ] = box.min [ od ] + 2 ;
  bx.max [ od ] = box.max [ od ] - 2 ;

  bx.draw ( dx, dy, PUSTYLE_SMALL_SHADED, colour, FALSE, 2 ) ;

  if ( box_label )
  {
    int xx ;
    int yy ;
    if ( isVertical () )  // Vertical slider, text goes to the right of it
    {
      xx = bx.max[0] + PUSTR_LGAP ;
      yy = ( bx.max[1] + bx.min[1] - legendFont.getStringHeight ( box_label ) ) / 2 ;
    }
    else  // Horizontal slider, text goes above it
    {
      xx = ( bx.max[0] + bx.min[0] - legendFont.getStringWidth ( box_label ) ) / 2 ;
      yy = bx.max[1] + PUSTR_BGAP ;
    }

    /* If greyed out then halve the opacity when drawing the label */

    if ( active )
      glColor4fv ( colour [ PUCOL_LABEL ] ) ;
    else
      glColor4f ( colour [ PUCOL_LABEL ][0],
                  colour [ PUCOL_LABEL ][1],
                  colour [ PUCOL_LABEL ][2],
                  colour [ PUCOL_LABEL ][3] / 2.0f ) ; /* 50% more transparent */

    legendFont.drawString ( box_label, dx + xx, dy + yy ) ;
  }
}

void puSlider::draw ( int dx, int dy )
{
  if ( !visible || ( window != puGetWindow () ) ) return ;

  if ( ( style == PUSTYLE_BEVELLED ) ||
       ( style == PUSTYLE_SHADED ) )
    abox.draw ( dx, dy, -PUSTYLE_BOXED, colour, FALSE, 2 ) ;
  else
    abox.draw ( dx, dy, -style, colour, FALSE, border_thickness ) ;

  if ( r_cb )
    r_cb ( this, dx, dy, render_data ) ;
  else
  {
    float val = getFloatValue () ;

    draw_slider_box ( dx, dy, abox, ( val - minimum_value ) / ( maximum_value - minimum_value ) ) ;

    draw_legend ( dx, dy ) ;
  }

  draw_label ( dx, dy ) ;
}


void puSlider::doHit ( int button, int updown, int x, int y )
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
    int sd = isVertical() ;
    int sz = abox.max [sd] - abox.min [sd] ;
    int coord = isVertical() ? y : x ;

    float next_value ;

    if ( sz == 0 )
      next_value = 0.5f ;
    else
    {
      next_value = ( (float)coord - (float)abox.min[sd] - (float)sz * slider_fraction / 2.0f ) /
                   ( (float) sz * (1.0f - slider_fraction) ) ;
    }

    next_value = (next_value < 0.0f) ? 0.0f : (next_value > 1.0f) ? 1.0f : next_value ;
    next_value = next_value * ( maximum_value - minimum_value ) + minimum_value ;

    setValue ( checkStep (next_value) );
    
    switch ( cb_mode )
    {
      case PUSLIDER_CLICK :
        if ( updown == active_mouse_edge )
        {
	  last_cb_value = next_value ;
    puSetActiveWidget ( this, x, y ) ;
	  invokeCallback () ;
        }
        break ;

      case PUSLIDER_DELTA : /* Deprecated! */
        if ( fabs ( last_cb_value - next_value ) >= cb_delta )
        {
	  last_cb_value = next_value ;
    puSetActiveWidget ( this, x, y ) ;
	  invokeCallback () ;
        }
        break ;

      case PUSLIDER_ALWAYS :
      default :
        last_cb_value = next_value ;
        puSetActiveWidget ( this, x, y ) ;
        invokeCallback () ;
        break ;
    }
  }
}


