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

UL_RTTI_DEF1(puScrollBar,puSlider)


void puScrollBar::draw ( int dx, int dy )
{
  extern void puDrawArrowButtonTriangle ( int pos_x, int pos_y, int size_x, int size_y,
                                          puColour colour, int arrow_type, int active ) ;

  if ( !visible || ( window != puGetWindow () ) ) return ;

  if ( r_cb )
    r_cb ( this, dx, dy, render_data ) ;
  else
  {
    puBox box = abox ;

    int width = isVertical () ? abox.max[0] - abox.min[0] : abox.max[1] - abox.min[1] ;

    /* Draw the arrow buttons */

    if ( arrow_count == 2 )  /* Double-arrow buttons */
    {
      if ( isVertical () )
      {
        box.min[1] = abox.max[1] - width ;
        box.draw ( dx, dy, style, colour, FALSE, border_thickness ) ;
        puDrawArrowButtonTriangle ( dx+box.min[0]+width/2, dy+box.min[1]+width/2, width, width, colour [ PUCOL_MISC ],
                                    PUARROW_FASTUP, fast_up_arrow_active ) ;
        box.min[1] = abox.min[1] ;
        box.max[1] = box.min[1] + width ;
        box.draw ( dx, dy, style, colour, FALSE, border_thickness ) ;
        puDrawArrowButtonTriangle ( dx+box.min[0]+width/2, dy+box.min[1]+width/2, width, width, colour [ PUCOL_MISC ],
                                    PUARROW_FASTDOWN, fast_down_arrow_active ) ;
      }
      else
      {
        box.min[0] = abox.max[0] - width ;
        box.draw ( dx, dy, style, colour, FALSE, border_thickness ) ;
        puDrawArrowButtonTriangle ( dx+box.min[0]+width/2, dy+box.min[1]+width/2, width, width, colour [ PUCOL_MISC ],
                                    PUARROW_FASTRIGHT, fast_up_arrow_active ) ;
        box.min[0] = abox.min[0] ;
        box.max[0] = box.min[0] + width ;
        box.draw ( dx, dy, style, colour, FALSE, border_thickness ) ;
        puDrawArrowButtonTriangle ( dx+box.min[0]+width/2, dy+box.min[1]+width/2, width, width, colour [ PUCOL_MISC ],
                                    PUARROW_FASTLEFT, fast_down_arrow_active ) ;
      }
    }

    if ( arrow_count > 0 )  /* Single-arrow buttons */
    {
      if ( isVertical () )
      {
        box.min[1] = abox.max[1] - arrow_count * width ;
        box.max[1] = box.min[1] + width ;
        box.draw ( dx, dy, style, colour, FALSE, border_thickness ) ;
        puDrawArrowButtonTriangle ( dx+box.min[0]+width/2, dy+box.min[1]+width/2, width, width, colour [ PUCOL_MISC ],
                                    PUARROW_UP, up_arrow_active ) ;
        box.max[1] = abox.min[1] + arrow_count * width ;
        box.min[1] = box.max[1] - width ;
        box.draw ( dx, dy, style, colour, FALSE, border_thickness ) ;
        puDrawArrowButtonTriangle ( dx+box.min[0]+width/2, dy+box.min[1]+width/2, width, width, colour [ PUCOL_MISC ],
                                    PUARROW_DOWN, down_arrow_active ) ;
      }
      else
      {
        box.min[0] = abox.max[0] - arrow_count * width ;
        box.max[0] = box.min[0] + width ;
        box.draw ( dx, dy, style, colour, FALSE, border_thickness ) ;
        puDrawArrowButtonTriangle ( dx+box.min[0]+width/2, dy+box.min[1]+width/2, width, width, colour [ PUCOL_MISC ],
                                    PUARROW_RIGHT, up_arrow_active ) ;
        box.max[0] = abox.min[0] + arrow_count * width ;
        box.min[0] = box.max[0] - width ;
        box.draw ( dx, dy, style, colour, FALSE, border_thickness ) ;
        puDrawArrowButtonTriangle ( dx+box.min[0]+width/2, dy+box.min[1]+width/2, width, width, colour [ PUCOL_MISC ],
                                    PUARROW_LEFT, down_arrow_active ) ;
      }
    }

    /* Draw the surrounding box */

    box.min[0] = abox.min[0] + ( isVertical () ? 0 : arrow_count * width ) ;
    box.max[0] = abox.max[0] - ( isVertical () ? 0 : arrow_count * width ) ;
    box.min[1] = abox.min[1] + ( isVertical () ? arrow_count * width : 0 ) ;
    box.max[1] = abox.max[1] - ( isVertical () ? arrow_count * width : 0 ) ;

    if ( ( style == PUSTYLE_BEVELLED ) ||
         ( style == PUSTYLE_SHADED ) )
      box.draw ( dx, dy, -PUSTYLE_BOXED, colour, FALSE, 2 ) ;
    else
      box.draw ( dx, dy, -style, colour, FALSE, border_thickness ) ;

    /* Draw the slider box */

    float val = getFloatValue () ;

    draw_slider_box ( dx, dy, box, ( val - getMinValue () ) / ( getMaxValue () - getMinValue () ) ) ;

    draw_legend ( dx, dy ) ;
  }

  draw_label ( dx, dy ) ;
}


void puScrollBar::doHit ( int button, int updown, int x, int y )
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
    int width = isVertical () ? abox.max[0] - abox.min[0] : abox.max[1] - abox.min[1] ;

    float next_value = getFloatValue () ;
    float step = getStepSize () ;
    if ( step == 0.0 ) step = ( getMaxValue () - getMinValue () ) / 10.0 ;

    /* Check for hitting a button */

    int sd = isVertical() ;
    int coord = ( isVertical() ? y : x ) ;

    if( arrow_count == 2 )
    {
      if ( coord < abox.min[sd] + width )  /* Fast down button */
        next_value -= 10.0 * step ;

      if ( coord > abox.max[sd] - width )  /* Fast up button */
        next_value += 10.0 * step ;
    }

    if ( arrow_count > 0 )
    {
      if ( ( coord < abox.min[sd] + arrow_count*width ) && ( coord > abox.min[sd] + (arrow_count-1)*width ) )  /* Down button */
        next_value -= step ;

      if ( ( coord > abox.max[sd] - arrow_count*width ) && ( coord < abox.max[sd] - (arrow_count-1)*width ) )  /* Down button */
        next_value += step ;
    }

    /* Check for hitting the slider bar */

    if ( ( coord > abox.min[sd]+arrow_count*width ) && ( coord < abox.max[sd]-arrow_count*width ) )
    {
      int sz = abox.max [sd] - abox.min [sd] - 2 * arrow_count * width ;

      if ( sz <= 0 )
        next_value = 0.5f ;
      else
      {
        next_value = ( float(coord - arrow_count * width - abox.min[sd]) - float(sz) * slider_fraction / 2.0f ) /
                     ( float(sz) * (1.0f - slider_fraction) ) ;
      }

      next_value = (next_value < 0.0f) ? 0.0f : (next_value > 1.0f) ? 1.0f : next_value ;
      next_value = getMinValue () + next_value * ( getMaxValue () - getMinValue () ) ;
    }

    if ( next_value < getMinValue () ) next_value = getMinValue () ;
    if ( next_value > getMaxValue () ) next_value = getMaxValue () ;
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


