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

UL_RTTI_DEF1(puMenuBar,puInterface)


void puMenuBar_drop_down_the_menu ( puObject *b )
{
  // Inputs:  b = pointer to the button on the menu which has been pushed
  // p = pointer to the popup menu which is b's submenu

  puPopupMenu *p = (puPopupMenu *) b -> getUserData () ;

  if ( b -> getIntegerValue () )
  {
/*
  SJBL Test hack.
*/
    puDeactivateWidget () ;

    p->reveal () ;   // Reveal the submenu

    // If the parent floats in its own window, and the submenu drops off the window,
    // expand the window to fit.

#ifndef PU_NOT_USING_GLUT
    puGroup *parent = p -> getParent () ;

    if ( ( parent != parent -> getParent () ) && parent -> getFloating () )
    {
      int temp_window = puGetWindow () ;
      glutSetWindow ( parent -> getWindow () ) ;

      puBox *par_box = parent -> getBBox () ;
      puBox *cur_box = p -> getBBox () ;
      int x_min = (cur_box->min[0] < 0) ? par_box->min[0] + cur_box->min[0] : par_box->min[0] ;
      int x_max = (par_box->max[0] > par_box->min[0] + cur_box->max[0]) ?
                                    par_box->max[0] : par_box->min[0] + cur_box->max[0] ;
      int y_min = (cur_box->min[1] < 0) ? par_box->min[1] + cur_box->min[1] : par_box->min[1] ;
      int y_max = (par_box->max[1] > par_box->min[1] + cur_box->max[1]) ?
                                    par_box->max[1] : par_box->min[1] + cur_box->max[1] ;
      int x_siz = glutGet ( (GLenum)GLUT_WINDOW_WIDTH ) ;
      int y_siz = glutGet ( (GLenum)GLUT_WINDOW_HEIGHT ) ;
      if ( x_siz < (x_max - x_min) ) x_siz = x_max - x_min ;    // Adjust the present size
      if ( y_siz < (y_max - y_min) ) y_siz = y_max - y_min ;

      glutReshapeWindow ( x_siz, y_siz ) ;

      x_min = par_box->min[0] - x_min ;
      y_min = y_siz - ( par_box->max[1] - par_box->min[1] ) ;

      /* If the parent window is SUPPOSED to be stuck at the top of the screen, move it. 
         - JCJ 6 June 2002 */
      if (parent -> getVStatus () == 1)
        parent -> setPosition ( x_min, y_min ) ;

      glutSetWindow ( temp_window ) ;
    }
#endif
  }
  else
    p->hide () ;

  for ( puObject *child = b -> getParent () -> getFirstChild () ;
        child != NULL ; child = child -> getNextObject() )
  {
    if (( child -> getType() & PUCLASS_BUTTON    ) != 0 && child != b ) child -> clrValue () ;
    if (( child -> getType() & PUCLASS_POPUPMENU ) != 0 && child != p ) child -> hide     () ;
  }

  // Move the popup menu to the last item in the "dlist" so it is drawn last
  // (in front of everything else).

  puMoveToLast ( p );
}

void puMenuBar::add_submenu ( const char *str, char *items[], puCallback _cb[],
                              void *_user_data[] )
{
  int w, h ;
  getSize ( &w, &h ) ;

  puOneShot *b ;

  if ( bar_height > 0 )
  {
    b = new puOneShot ( w+10,
                        0,
                        w+10 + PUSTR_LGAP + puGetDefaultLegendFont().getStringWidth ( str ) + PUSTR_RGAP,
                        bar_height ) ;
    b -> setLegend ( str ) ;
  }
  else
    b = new puOneShot ( w+10, 0, str ) ;

  b -> setStyle ( PUSTYLE_SPECIAL_UNDERLINED ) ;
  b -> setColourScheme ( colour[PUCOL_FOREGROUND][0],
                         colour[PUCOL_FOREGROUND][1],
                         colour[PUCOL_FOREGROUND][2],
                         colour[PUCOL_FOREGROUND][3] ) ;
  b -> setCallback ( puMenuBar_drop_down_the_menu ) ;
  b -> setActiveDirn ( PU_UP_AND_DOWN ) ;

  puPopupMenu *p = new puPopupMenu ( w+10, 0 ) ;

  b -> setUserData ( p ) ;

  if ( _user_data != NULL )
    for ( int i = 0 ; items[i] != NULL ; i++ )
      p -> add_item ( items[i], _cb[i], _user_data[i] ) ;
  else
    for ( int i = 0 ; items[i] != NULL ; i++ )
      p -> add_item ( items[i], _cb[i] ) ;

  p->close () ;
  recalc_bbox () ;
}

void puMenuBar::close (void)
{
  puInterface::close () ;

  if ( dlist == NULL )
    return ;

  int width = 0 ;
  puObject *ob ;

  /*
    Use alternate objects - which gets the puOneShot/puPopupMenu pairs
  */

  for ( ob = dlist ; ob != NULL ; ob = ob -> getNextObject() )
  {
    int w, h ;

    /* Reposition the button so it looks nice */

    ob -> getSize ( &w, &h ) ;
    ob -> setPosition ( width, 0 ) ;
    ob = ob -> getNextObject() ;

    /* Reposition the submenu so it sits under the button */

    int w2, h2 ;
    ob -> getSize ( &w2, &h2 ) ;
    ob -> setPosition ( width, -h2 ) ;

    /* Next please! */
    width += w ;
  }

  recalc_bbox () ;
}


