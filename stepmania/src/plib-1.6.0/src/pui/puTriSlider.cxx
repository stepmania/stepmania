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

UL_RTTI_DEF1(puTriSlider,puBiSlider)


void puTriSlider::draw ( int dx, int dy )
{
  if ( !visible || ( window != puGetWindow () ) ) return ;

  // Draw the slider box itself

  if ( ( style == PUSTYLE_BEVELLED ) ||
       ( style == PUSTYLE_SHADED ) )
    abox.draw ( dx, dy, -PUSTYLE_BOXED, colour, FALSE, 2 ) ;
  else
    abox.draw ( dx, dy, -style, colour, FALSE, border_thickness ) ;

  if ( r_cb )
    r_cb ( this, dx, dy, render_data ) ;
  else
  {
    // Draw the central slider and label it

    float val;
    if ( getMaxValue() > getMinValue() )
      val = (float)(getFloatValue () - getMinValue()) / (float)(getMaxValue() - getMinValue()) ;
    else
      val = 0.5f ;

    char str_value[10] ;
    sprintf (str_value, "%g", getFloatValue () ) ;

    draw_slider_box ( dx, dy, abox, val, str_value ) ;

    if ( val < 0.0f ) val = 0.0f ;
    if ( val > 1.0f ) val = 1.0f ;

    // Draw the current_max slider and label it

    if ( getMaxValue() > getMinValue() )
      val = (float)(getCurrentMax() - getMinValue()) / (float)(getMaxValue() - getMinValue()) ;
    else
      val = 1.0f ;

    sprintf (str_value, "%g", getCurrentMax() ) ;

    draw_slider_box ( dx, dy, abox, val, str_value ) ;

    // Draw the current_min slider and label it

    if ( getMaxValue() > getMinValue() )
      val = (float)(getCurrentMin() - getMinValue()) / (float)(getMaxValue() - getMinValue()) ;
    else
      val = 0.0f ;

    sprintf (str_value, "%g", getCurrentMin() ) ;

    draw_slider_box ( dx, dy, abox, val, str_value ) ;

    draw_legend ( dx, dy ) ;
  }

  draw_label ( dx, dy ) ;
}


void puTriSlider::doHit ( int button, int updown, int x, int y )
{
  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    puActiveWidget() -> invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( updown != PU_DRAG )
  {
    puMoveToLast ( this );
  /* Bug Fixed: If you moved a slider while dragging the mouse (click and drag), then move   */
  /* the cursor outside of the widget and released the mouse button, the active_button would */
  /* be locked to the last slider you moved, and unable to release. - JCJ 11 June 2002       */
    setActiveButton ( 0 ) ;
  }

  if ( button == PU_LEFT_BUTTON && updown == PU_UP )
  {
    setActiveButton ( 0 ) ;
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

    next_value = (next_value < 0.0f) ? 0.0f : (next_value > 1.0) ? 1.0f : next_value ;

    float new_value = getMinValue() + next_value * ( getMaxValue() - getMinValue() ) ;

    if ( getFreezeEnds() )  // Cannot move end sliders, must move middle one
    {
      setActiveButton ( 2 ) ;  
      setValue ( checkStep(new_value) ) ; /* Ensure that the middle slider can't move beyond the barriers - JCJ 10 Jun 2002 */
      if ( new_value < getCurrentMin() ) setValue ( getCurrentMin() ) ;
      if ( new_value > getCurrentMax() ) setValue ( getCurrentMax() ) ;
    }
    else
    {
      if ( getActiveButton() == 0 )  // No currently-active slider, set whichever is closest
      {
        if ( (new_value-getCurrentMin()) < (getFloatValue()-new_value) ) // Closest to current_min
        {
          setCurrentMin ( checkStep(new_value) ) ;
          setActiveButton ( 1 ) ;
        }
        else if ( (new_value-getFloatValue()) > (getCurrentMax()-new_value) ) // Closest to current_max
        {
          setCurrentMax ( checkStep(new_value) ) ;
          setActiveButton ( 3 ) ;
        }
        else  // closest to the center slider -- ties go to the center slider
        {
          setValue ( checkStep(new_value) ) ;
          setActiveButton ( 2 ) ;
        }
      }
      else if ( getActiveButton() == 1 )  // Currently moving current_min
      {
        setCurrentMin ( checkStep(new_value) ) ;
        if ( getFloatValue() < getCurrentMin() ) setValue ( getCurrentMin() ) ;
        if ( getCurrentMax() < getCurrentMin() ) setCurrentMax ( getCurrentMin() ) ;
      }
      else if ( getActiveButton() == 2 )  // Currently moving central value
      {
        setValue ( checkStep(new_value) ) ;
        if ( getCurrentMin() > new_value ) setCurrentMin ( new_value ) ;
        if ( getCurrentMax() < new_value ) setCurrentMax ( new_value ) ;
      }
      else if ( getActiveButton() == 3 )  // Currently moving current_max
      {
        setCurrentMax ( checkStep(new_value) ) ;
        if ( getFloatValue() > getCurrentMax() ) setValue ( getCurrentMax() ) ;
        if ( getCurrentMax() < getCurrentMin() ) setCurrentMin ( getCurrentMax() ) ;
      }
    }

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

      case PUSLIDER_DELTA :/* Deprecated! */
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
