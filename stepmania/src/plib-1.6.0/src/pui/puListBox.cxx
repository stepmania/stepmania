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

/****
* NAME
*   puListBox
*
* DESCRIPTION
*   list of strings
*
* AUTHOR
*   Dave McClurg <dpm@efn.org>
*
* CREATION DATE
*   August 2000
*
* MODIFICATION HISTORY
*   John Fay - many improvements
****/

#include "puLocal.h"

UL_RTTI_DEF1(puListBox,puButton)


puListBox::puListBox  ( int minx, int miny, int maxx, int maxy, char** _list ) :
               puButton ( minx, miny, maxx, maxy )
{
  type |= PUCLASS_LISTBOX ;

  newList ( _list ) ;
}


void puListBox::newList ( char ** _list )
{
  list = _list ;

  if ( list == NULL )
    num = 0 ;
  else
    for ( num = 0 ; list [ num ] != NULL ; num++ )
      /* Count number of items */ ;

  top = 0 ;

  /* Set index of selected item */
  setValue ( -1 ) ;

  puPostRefresh () ;
}


void puListBox::setTopItem( int item_index )
{
  top = item_index ;
  if ( top < 0 )
    top = 0 ;
  else if ( top > num-1 )
    top = num-1;

  puPostRefresh () ;
}


void puListBox::draw ( int dx, int dy )
{
  if ( !visible || ( window != puGetWindow () ) || list == NULL ) return ;

  abox.draw ( dx, dy, style, colour, isReturnDefault(), border_thickness ) ;

  /* If greyed out then halve the opacity when drawing the text */

  if ( active )
    glColor4fv ( colour [ PUCOL_LEGEND ] ) ;
  else
    glColor4f ( colour [ PUCOL_LEGEND ][0],
                colour [ PUCOL_LEGEND ][1],
                colour [ PUCOL_LEGEND ][2],
                colour [ PUCOL_LEGEND ][3] / 2.0f ) ; /* 50% more transparent */

  int xsize = abox.max[0] - abox.min[0] + 1 ;

  if ( r_cb )
    r_cb ( this, dx, dy, render_data ) ;
  else
  {
    int yinc = legendFont.getStringHeight () + PUSTR_BGAP ;
    int num_vis = getNumVisible () ;

    int selected ;
    getValue ( &selected ) ;

    for ( int i = top ; i < num && i < top + num_vis ; i++ )
    {
      if ( i == selected )
        glColor4f ( 1.0f, 1.0f, 1.0f, 1.0f ) ;
      else
        glColor4f ( 0.0f, 0.0f, 0.0f, 1.0f ) ;

      int x = PUSTR_LGAP ;
      int y = yinc * ((i-top)+1) ;

      int xx = dx + abox.min[0] + x ;
      int yy = dy + abox.max[1] - y ;

      int width ;
      char str [ PUSTRING_MAX ] ;
      strcpy ( str, list [ i ] ) ;

      /*
        Does the string fit into the box?

        If not, chop it down one character at a time until
        it does fit.
      */

      while ( 1 )
      {
        width = legendFont.getStringWidth ( (char *)str ) + PUSTR_LGAP ;

        if ( width < xsize )
          break ;

        /*
          Nibble off one character and try again
          (Do that sneakily by replacing the last 4 characters with 3 dots)
        */

        strcpy ( & str [ strlen(str) - 4 ], "..." ) ;
      }

      legendFont.drawString ( (char*)str, xx, yy ) ;
    }
  }

  draw_label ( dx, dy ) ;
}


void puListBox::doHit ( int button, int updown, int x, int y )
{
  if ( puActiveWidget() && ( this != puActiveWidget() ) )
  {
    puActiveWidget() -> invokeDownCallback () ;
    puDeactivateWidget () ;
  }

  if ( button == PU_LEFT_BUTTON )
  {
    if ( updown == active_mouse_edge || active_mouse_edge == PU_UP_AND_DOWN )
    {
      lowlight () ;
    
      int yinc = legendFont.getStringHeight () + PUSTR_BGAP ;
      int idx = top + ( abox.max[1] - PUSTR_BGAP - y ) / yinc;
      if ( idx < 0 )
        idx = 0;
      else if ( idx >= num )
        idx = num-1;
    
      setValue ( idx ) ;
    
      puSetActiveWidget ( this, x, y ) ;
      invokeCallback () ;
    }
    else
      highlight () ;                                                            
  }
  else
    lowlight () ;
}


