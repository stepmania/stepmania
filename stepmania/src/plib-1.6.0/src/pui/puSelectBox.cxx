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

UL_RTTI_DEF1(puSelectBox,puGroup)


void puSelectBox::handle_arrow ( puObject *arrow )
{
  puSelectBox *selbox = (puSelectBox *) arrow -> getUserData () ;
  int arrow_type = ((puArrowButton*) arrow) -> getArrowType () ;

  if ( arrow_type == PUARROW_DOWN )
    selbox -> setCurrentItem ( selbox -> getCurrentItem () - 1 ) ;
  else if ( arrow_type == PUARROW_UP )
    selbox -> setCurrentItem ( selbox -> getCurrentItem () + 1 ) ;
}


void puSelectBox::update_widgets ( void )
{
  if ( curr_item >= 0 )
  /* There are some items */
  {
    setValue ( list[curr_item] ) ;

    if ( curr_item == 0 )
    /* Current item is first item, deactivate 'down' button */
      down_arrow -> greyOut () ;
    else
      down_arrow -> activate () ;

    if ( curr_item == (num_items - 1) )
    /* Current item is last item, deactivate 'up' button */
      up_arrow -> greyOut () ;
    else
      up_arrow -> activate () ;
  }
  else
  /* There aren't any items */
  {
    setValue ( "" ) ;

    down_arrow -> greyOut () ;
    up_arrow   -> greyOut () ;
  }
}


void puSelectBox::newList ( char ** _list )
{
  list = _list ;

  if ( list == NULL )
    num_items = 0 ;
  else
  {
    for ( num_items = 0 ; list[num_items] != NULL ; num_items++ )
      /* Count number of items */ ;
  }

  curr_item = ( num_items > 0 ? 0 : - 1 ) ;
  update_widgets () ;
}


void puSelectBox::setSize ( int w, int h )
{
  int arrow_size = h / 2 ;

  /* Resize the puInput widget */

  input -> setSize ( w - arrow_size, h ) ;

  /* Resize and reposition the arrow buttons */

  down_arrow -> setPosition ( w - arrow_size, 0 ) ;
  down_arrow -> setSize ( arrow_size, arrow_size ) ;

  up_arrow   -> setPosition ( w - arrow_size, arrow_size ) ;
  up_arrow   -> setSize ( arrow_size, arrow_size ) ;
}

void puSelectBox::draw ( int dx, int dy )
{
  if ( !visible || ( window != puGetWindow () ) ) return ;

  draw_label ( dx, dy ) ;

  puGroup::draw ( dx, dy ) ;
}

int puSelectBox::checkKey ( int key, int updown )
{
  if ( ! input -> isAcceptingInput () || ! isVisible () || ! isActive () || ( window != puGetWindow () ) )
    return FALSE ;

  switch ( key )
  {
    case PU_KEY_HOME      :
      setCurrentItem ( 0 ) ;
      break ;

    case PU_KEY_END       :
      setCurrentItem ( num_items - 1 ) ;
      break ;

    case PU_KEY_UP        :
    case PU_KEY_PAGE_UP   :
      setCurrentItem ( curr_item + 1 ) ;
      break ;

    case PU_KEY_DOWN      :
    case PU_KEY_PAGE_DOWN :
      setCurrentItem ( curr_item - 1 ) ;
      break ;

    default :
      /* We might decide to make the input box editable on request
         in future - and it does not hurt ... */
      return input -> checkKey ( key, updown ) ;
      break ;
  }

  return TRUE ;
}


puSelectBox::puSelectBox ( int minx, int miny, int maxx, int maxy,
                         char **entries ) :
   puGroup( minx, miny )
{
  type |= PUCLASS_SELECTBOX ;

  char *stringval ;
  int arrow_size = (maxy - miny) / 2 ;

  input = new puInput ( 0, 0, maxx-minx - arrow_size, maxy-miny ) ;
  input -> setStyle ( PUSTYLE_SMALL_SHADED ) ;
  input -> disableInput () ;

  /* Share 'string' value with input box */
  input -> getValue ( &stringval ) ;
  setValuator ( stringval ) ;

  down_arrow = new puArrowButton ( maxx-minx - arrow_size, 0,
                                   maxx-minx, arrow_size,
                                   PUARROW_DOWN ) ;
  down_arrow -> setStyle ( PUSTYLE_SMALL_SHADED ) ;
  down_arrow -> setUserData ( this ) ;
  down_arrow -> setCallback ( handle_arrow ) ;

  up_arrow   = new puArrowButton ( maxx-minx - arrow_size, arrow_size,
                                   maxx-minx, maxy-miny,
                                   PUARROW_UP ) ;
  up_arrow   -> setStyle ( PUSTYLE_SMALL_SHADED ) ;
  up_arrow   -> setUserData ( this ) ;
  up_arrow   -> setCallback ( handle_arrow ) ;

  newList ( entries ) ;

  close () ;
}

