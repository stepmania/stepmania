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

UL_RTTI_DEF1(puSpinBox,puRange)


void puSpinBox::draw ( int dx, int dy )
{
  extern void puDrawArrowButtonTriangle ( int pos_x, int pos_y, int size_x, int size_y,
                                          puColour colour, int arrow_type, int active ) ;

  if ( !visible || ( window != puGetWindow () ) ) return ;

  if ( r_cb )
    r_cb ( this, dx, dy, render_data ) ;
  else
  {
    //input_box->draw ( dx, dy ) ;

    int size = int(inbox_height * getArrowHeight()) ;
    int pos_y = dy + abox.min[1] + size / 2 ;
    int pos_x = dx ;
    if ( getArrowPosition() == 1 ) {
        /* Arrows lie on the right side of the box */
        pos_x += abox.max[0] - size / 2 ;
    } else {
        /* Arrows are on the left side */
        pos_x += abox.min[0] + size / 2 ;
    }

    input_box->draw( dx, dy );
    
    puBox box ;
    box.min[0] = pos_x - size / 2 ;
    box.max[0] = pos_x + size / 2 ;
    box.min[1] = pos_y - size / 2 ;
    box.max[1] = pos_y + size / 2 ;

    box.draw ( 0, 0, style, colour, FALSE, border_thickness ) ;
    puDrawArrowButtonTriangle ( pos_x, pos_y, size, size, colour [ PUCOL_MISC ],
                                PUARROW_DOWN, down_arrow_active ) ;
    box.min[1] += size ; 
    box.max[1] += size ;
    box.draw ( 0, 0, style, colour, FALSE, border_thickness ) ;
    puDrawArrowButtonTriangle ( pos_x, pos_y + size, size, size, colour [ PUCOL_MISC ],
                                    PUARROW_UP, up_arrow_active ) ;
  }

  draw_label ( dx, dy ) ;
}


void puSpinBox::doHit ( int button, int updown, int x, int y )
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
    int size = int( inbox_height * getArrowHeight()) ;
    if ( ( x < abox.max[0] - size ) && ( getArrowPosition() == 1 ) ||
         ( x > abox.min[0] + size ) && ( getArrowPosition() == 0 ) )  
    {/* User clicked in the input box */
        /* Now see if it's in the Y of the input box, in case the arrows are made huge */
        puBox *ibox = input_box->getABox() ;
        if ( y > ibox->min[1] && y < ibox->max[1] )
            input_box->doHit ( button, updown, x, y ) ;
    }
    else
    {
      if ( y < abox.min[1] + size )  /* User clicked on down-arrow */
        setValue ( getFloatValue () - getStepSize () ) ;
      else  /* User clicked on up-arrow */
        setValue ( getFloatValue () + getStepSize () ) ;

      if ( getFloatValue () > getMaxValue () )
        setValue ( getMaxValue () ) ;
      else if ( getFloatValue () < getMinValue () )
        setValue ( getMinValue () ) ;

      input_box->setValue ( getFloatValue () ) ;
    }
    puSetActiveWidget ( this, x, y ) ;
    invokeCallback () ;
  }
}

int puSpinBox::checkKey ( int key, int updown )
{
  if ( input_box->checkKey ( key, updown ) )
  {
    setValue ( input_box->getFloatValue () ) ;
    if ( getFloatValue () > getMaxValue () )
      setValue ( getMaxValue () ) ;
    else if ( getFloatValue () < getMinValue () )
      setValue ( getMinValue () ) ;

	  invokeCallback () ;
    return TRUE ;
  }
  else
    return FALSE ;
}

void puSpinBox_handle_input (puObject *ob) 
{
    puObject *master = (puObject *)(ob->getUserData());
    master->setValue(ob->getFloatValue());
    master->invokeCallback();
}
