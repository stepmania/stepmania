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

UL_RTTI_DEF1(puComboBox,puGroup)


void puComboBox::input_down_cb ( puObject *inp )
{
  puComboBox *cbox = (puComboBox *) inp -> getUserData () ;

  cbox -> invokeCallback () ;
}

void puComboBox::handle_arrow ( puObject *arrow )
{
  puComboBox *cbox = (puComboBox *) arrow -> getUserData () ;

  if ( ! cbox -> __getPopupMenu () -> isVisible () )
    cbox -> __getPopupMenu () -> reveal () ;
  else
    cbox -> __getPopupMenu () -> hide () ;

  /* arrow -> setStyle ( - arrow -> getStyle () ) ; */
}

void puComboBox::handle_popup ( puObject *popupm )
{
  puComboBox *cbox = (puComboBox *) popupm -> getUserData () ;

  cbox -> setCurrentItem ( popupm -> getLegend () ) ;
}


void puComboBox::update_widgets ( void )
{
  if ( curr_item >= 0 )
  /* There are some items */
  {
    setValue ( list[curr_item] ) ;

    arrow_btn -> activate () ;
  }
  else
  /* There aren't any items */
  {
    setValue ( "" ) ;

    arrow_btn -> greyOut () ;
  }
}


void puComboBox::newList ( char ** _list )
{
  list = _list ;

  popup_menu -> empty () ;

  if ( list == NULL )
    num_items = 0 ;
  else
  {
    for ( num_items = 0 ; list[num_items] != NULL ; num_items++ )
      /* Count number of items */ ;

    if ( num_items > 0 )
    {
      int dummy, h ;
      int old_height = abox.max[1] - abox.min[1] ;
      int i ;

      puPushGroup ( popup_menu ) ;

      for ( i = (num_items - 1) ; i >= 0 ; i-- )
      {
        puObject *menu_item = popup_menu -> add_item ( list[i], handle_popup ) ;

        menu_item -> setUserData ( this ) ;
        menu_item -> getSize ( &dummy, &h ) ;
        menu_item -> setSize ( abox.max[0] - abox.min[0], h ) ;
      }

      popup_menu -> close() ;

      /* Position popup menu correctly */

      popup_menu -> getSize ( &dummy, &h ) ;

      if ( (abox.min[1] - h) >= 0 )
      {
        popup_menu -> setPosition ( 0, 0 - h ) ;

        arrow_btn  -> setArrowType ( PUARROW_DOWN ) ;
      }
      else
      {
        popup_menu -> setPosition ( 0, old_height ) ;

        arrow_btn  -> setArrowType ( PUARROW_UP   ) ;
      }
    }
  }

  curr_item = ( num_items > 0 ? 0 : - 1 ) ;
  update_widgets () ;
}

int puComboBox::getCurrentItem ( void )
{
  if ( num_items > 0 )
  {
    if ( strcmp ( list[curr_item], getStringValue() ) )
    /* The user typed in an arbitrary string.
       Let's see if it is one of our entries ... */
    {
      int i ;

      for ( i = 0 ; i < num_items ; i++ )
      {
        if ( !strcmp ( list[i], getStringValue() ) )
          /* ... yes, it its ! */
          return i ;
      }

      /* ... no, it isn't. */
      return -1 ;
    }
  }

  return curr_item ;
}

void puComboBox::setCurrentItem ( const char *item_ptr )
{
  int i ;

  for ( i = 0 ; i < num_items ; i++ )
  {
    if ( list[i] == item_ptr )
    {
      setCurrentItem ( i ) ;

      break ;
    }
  }
}


void puComboBox::draw ( int dx, int dy )
{
  if ( !visible || ( window != puGetWindow () ) ) return ;

  draw_label ( dx, dy ) ;

  puGroup::draw ( dx, dy ) ;
}

int puComboBox::checkHit ( int button, int updown, int x, int y )
{
  if ( input -> checkHit ( button, updown, x-abox.min[0], y-abox.min[1] ) )
  {
    popup_menu -> hide () ;

    return TRUE ;
  }
  else
    return puGroup::checkHit ( button, updown, x, y ) ;
}

int puComboBox::checkKey ( int key, int updown )
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
      setCurrentItem ( getCurrentItem () + 1 ) ;
      break ;

    case PU_KEY_DOWN      :
      setCurrentItem ( getCurrentItem () - 1 ) ;
      break ;

    case PU_KEY_PAGE_UP   :
    case PU_KEY_PAGE_DOWN :
      if ( ! popup_menu -> isVisible () )
        popup_menu -> reveal () ;
      else
        popup_menu -> hide () ;

      break ;

    default :
      return input -> checkKey ( key, updown ) ;
      break ;
  }

  return TRUE ;
}


puComboBox::puComboBox ( int minx, int miny, int maxx, int maxy,
                         char **entries, int editable ) :
   puGroup( minx, miny )
{
  type |= PUCLASS_COMBOBOX ;

  char *stringval ;
  int arrow_width = (int) ( (maxy-miny) / 1.5 ) ;

  input = new puInput ( 0, 0, maxx-minx - arrow_width, maxy-miny ) ;
  input -> setUserData ( this ) ;
  input -> setDownCallback ( input_down_cb ) ;

  input -> setStyle ( PUSTYLE_SMALL_SHADED ) ;

  if ( ! editable )
    input -> disableInput () ;

  /* Share 'string' value with input box */
  input -> getValue ( &stringval ) ;
  setValuator ( stringval ) ;

  arrow_btn = new puArrowButton ( maxx-minx - arrow_width, 0,
                                  maxx-minx, maxy-miny,
                                  PUARROW_DOWN ) ;
  arrow_btn -> setStyle ( PUSTYLE_SMALL_SHADED ) ;
  arrow_btn -> setUserData ( this ) ;
  arrow_btn -> setCallback ( handle_arrow ) ;

  popup_menu = new puPopupMenu ( 0, 0 ) ;
  popup_menu -> close () ;

  close () ;

  newList ( entries ) ;
}

