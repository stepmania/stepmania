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

UL_RTTI_DEF1(puButtonBox,puObject)


puButtonBox::puButtonBox ( int minx, int miny, int maxx, int maxy,
                char **labels, int one_button ) :
                    puObject ( minx, miny, maxx, maxy )
{
  type |= PUCLASS_BUTTONBOX ;
  one_only = one_button ;

  button_labels = labels ;

  newList ( labels ) ;
}


void puButtonBox::newList ( char ** _list )
{
  button_labels = _list ;

  if ( button_labels == NULL )
    num_kids = 0 ;
  else
    for ( num_kids = 0 ; button_labels [ num_kids ] != NULL ; num_kids++ )
      /* Count number of items */ ;

  puPostRefresh() ;
}


int puButtonBox::checkKey ( int key, int updown )
{
  if ( updown == PU_UP ||
       ! isReturnDefault() ||
       ( key != '\r' && key != '\n' ) || ( window != puGetWindow () ) )
    return FALSE ;

  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    puActiveWidget() -> invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  int v = getIntegerValue () ;

  if ( ! one_only )
    v = ~v ;
  else
  if ( ++v > num_kids )
    v = 0 ;

  setValue ( v ) ;
//  puSetActiveWidget ( this, x, y ) ;
  invokeCallback() ;
  return TRUE ;
}


int puButtonBox::checkHit ( int /*button*/, int updown, int x, int y )
{
  if ( ! isHit ( x, y ) ||
       ( updown != active_mouse_edge &&
         active_mouse_edge != PU_UP_AND_DOWN ) )
    return FALSE ;

  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    puActiveWidget() -> invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( updown != PU_DRAG )
    puMoveToLast ( this );

  int i = num_kids - 1 - (( y - abox.min[1] - PUSTR_BGAP ) * num_kids ) /
                          ( abox.max[1] - abox.min[1] - PUSTR_BGAP - PUSTR_TGAP ) ;

  if ( i < 0 ) i = 0 ;
  if ( i >= num_kids ) i = num_kids - 1 ;

  if ( one_only )
    setValue ( i ) ;
  else
    setValue ( getIntegerValue () ^ ( 1 << i ) ) ;

  puSetActiveWidget ( this, x, y ) ;
  invokeCallback () ;
  return TRUE ;
}


void puButtonBox::draw ( int dx, int dy )
{
  if ( !visible || ( window != puGetWindow () ) ) return ;

  abox.draw ( dx, dy, style, colour, isReturnDefault(), border_thickness ) ;

  if ( r_cb )
    r_cb ( this, dx, dy, render_data ) ;
  else
  {
    for ( int i = 0 ; i < num_kids ; i++ )
    {
      puBox tbox ;

      tbox.min [ 0 ] = abox.min [ 0 ] + PUSTR_LGAP + PUSTR_LGAP ;
      tbox.min [ 1 ] = abox.min [ 1 ] + ((abox.max[1]-abox.min[1]-PUSTR_TGAP-PUSTR_BGAP)/num_kids) * (num_kids-1-i) ;
      tbox.max [ 0 ] = tbox.min [ 0 ] ;
      tbox.max [ 1 ] = tbox.min [ 1 ] ;

      if (( one_only && i == getIntegerValue() ) ||
          ( !one_only && ((1<<i) & getIntegerValue() ) != 0 ) ) 
        tbox.draw ( dx, dy + PUSTR_BGAP + PUSTR_BGAP, -PUSTYLE_RADIO, colour, FALSE, border_thickness ) ;
      else
        tbox.draw ( dx, dy + PUSTR_BGAP + PUSTR_BGAP, PUSTYLE_RADIO, colour, FALSE, border_thickness ) ;

      /* If greyed out then halve the opacity when drawing the label and legend */

      if ( active )
        glColor4fv ( colour [ PUCOL_LEGEND ] ) ;
      else
        glColor4f ( colour [ PUCOL_LEGEND ][0],
                    colour [ PUCOL_LEGEND ][1],
                    colour [ PUCOL_LEGEND ][2],
                    colour [ PUCOL_LEGEND ][3] / 2.0f ) ; /* 50% more transparent */

      legendFont.drawString ( button_labels[i],
                     dx + tbox.min[0] + PU_RADIO_BUTTON_SIZE + PUSTR_LGAP,
                     dy + tbox.min[1] + legendFont.getStringDescender() + PUSTR_BGAP  + PUSTR_BGAP) ;
    }
  }

  draw_label ( dx, dy ) ;
}


