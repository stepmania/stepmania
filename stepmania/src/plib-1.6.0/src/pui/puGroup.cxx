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

UL_RTTI_DEF1(puGroup,puObject)


#define PUSTACK_MAX 100

static int currGroup = -1 ;
static puGroup *groupStack [ PUSTACK_MAX ] ;

void puPushGroup ( puGroup *in )
{
  if ( currGroup < PUSTACK_MAX )
    groupStack [ ++currGroup ] = in ;
  else
    ulSetError ( UL_WARNING, "PUI: Too many puGroups open at once!" ) ;
}


void  puPopGroup ( void )
{
  if ( currGroup > 0 )
    --currGroup ;
  else 
    ulSetError ( UL_WARNING, "PUI: puGroup stack is empty!" ) ;
}

int  puNoGroup ( void )
{
  return currGroup < 0 ;
}


puGroup *puGetCurrGroup ( void )
{
  if ( currGroup < 0 )
  {
    ulSetError ( UL_WARNING, "PUI: No Group!" ) ;
    return NULL ;
  }

  return groupStack [ currGroup ] ;
}

void puGroup::remove ( puObject *obj )
{
  if ( obj -> getParent () != this )
    return ;

  /* Are we the first object in the list */

  if ( obj -> getPrevObject() == NULL )
    dlist = obj -> getNextObject() ;
  else
    obj -> getPrevObject() -> setNextObject( obj -> getNextObject() ) ;

  /* Are we the last object in the list */

  if ( obj -> getNextObject() != NULL )
    obj -> getNextObject() -> setPrevObject( obj -> getPrevObject() ) ;

  obj -> setNextObject ( NULL ) ;
  obj -> setPrevObject ( NULL ) ;
  obj -> setParent ( NULL ) ;

  num_children-- ;
  recalc_bbox () ;
}

void puGroup::empty ( void )
{
  puObject *obj = getLastChild () ;
  while ( obj != NULL )
  {
    if ( obj->getType () & PUCLASS_GROUP )
    {
      puGroup *group = (puGroup *)obj ;
      group->empty () ;
    }

    puObject *temp = obj->getPrevObject () ;
    delete obj ;
    obj = temp ;
  }

  dlist = NULL ;
}

void puGroup::add ( puObject *new_obj )
{
  if ( new_obj -> getParent () != NULL )
    new_obj -> getParent () -> remove ( new_obj ) ;

  new_obj -> setParent ( this ) ;

  if ( dlist == NULL )
  {
    dlist = new_obj ;
    new_obj -> setNextObject ( NULL ) ;
    new_obj -> setPrevObject ( NULL ) ;
  }
  else
  {
    puObject *last ;

    for ( last = dlist ;
          last -> getNextObject() != NULL ;
          last = last -> getNextObject() )
      /* Search for end of list. */ ;

    last -> setNextObject ( new_obj ) ;
    new_obj -> setPrevObject ( last ) ;
    new_obj -> setNextObject ( NULL ) ;
  }

  num_children++ ;
  recalc_bbox () ;
}

int puGroup::checkKey ( int key, int updown )
{
  if ( dlist == NULL || ! isVisible () || ! isActive () )
    return FALSE ;

  puObject *bo ;

  /*
    We have to walk the list backwards to ensure that
    the click order is the same as the DRAW order.
  */

  bo = getLastChild () ;

  for ( ; bo != NULL ; bo = bo -> getPrevObject() )
    if ( bo -> checkKey ( key, updown ) )
      return TRUE ;

  return FALSE ;
}

int puGroup::checkHit ( int button, int updown, int x, int y )
{
  if ( dlist == NULL || ! isVisible () || ! isActive () )
    return FALSE ;

  /*
    This might be a bit redundant - but it's too hard to keep
    track of changing abox sizes when daughter objects are
    changing sizes.
  */

  recalc_bbox () ;

  puObject *bo ;

  x -= abox.min[0] ;
  y -= abox.min[1] ;

  /*
    We have to walk the list backwards to ensure that
    the click order is the same as the DRAW order.
  */

  if ( !mouse_active )
  {
    bo = getLastChild () ;

    for ( ; bo != NULL ; bo = bo -> getPrevObject() )
    {
      /* If this is a menu bar and the mouse is over the button, highlight the button, if it's in the right window - JCJ 6 Jun 2002 */
      if ( ( ( getType () & PUCLASS_MENUBAR ) || ( getType () & PUCLASS_VERTMENU ) ) && ( bo->getType () & PUCLASS_ONESHOT ) && ( window == puGetWindow () ) )
      /*
      Changing this statement to something like this:
      if ( bo->getType () & PUCLASS_ONESHOT )
      Breaks a lot of stuff... like pressing buttons actually working in filepicker. Need 
      to just define everything that can be pressed passively above, and things should
      be fröhlich.
      */
      {
        puBox *box = bo->getABox () ;
        if ( ( x >= box->min[0] ) && ( x <= box->max[0] ) &&
             ( y >= box->min[1] ) && ( y <= box->max[1] ) )
          bo->highlight () ;
        else
          bo->lowlight () ;
      }

      if ( bo -> checkHit ( button, updown, x, y ) )
        return TRUE ;
    }
  }

  /*
    If right mouse button is pressed, save mouse coordinates for
    dragging and dropping.  Do this only if the "floating" flag is set.
  */

  if ( mouse_active || ( isHit ( x+abox.min[0], y+abox.min[1]) &&
       floating && ( button == PU_RIGHT_BUTTON ) ) )
  {
    puMoveToLast ( this );

    /*
      Return (x, y) to coordinates of parent interface to avoid "jumping" of
      present interface as mouse drags
    */

    x += abox.min[0] ;
    y += abox.min[1] ;

    if ( updown == PU_DOWN )
    {
      mouse_x = x;  /* Save mouse coordinates for dragging */
      mouse_y = y;

      mouse_active = TRUE ;

      return TRUE ;
    }
    else if ( updown == PU_DRAG )
    {
      int curr_x, curr_y;

      getPosition ( &curr_x, &curr_y );
      setPosition ( curr_x+x-mouse_x, curr_y+y-mouse_y );  /* Move to new position */

      mouse_x = x;  /* Save new coordinates */
      mouse_y = y;

      return TRUE ;
    }
    else if ( updown == PU_UP )
    {
      mouse_active = FALSE ;

      return TRUE ;
    }
  }

  return FALSE ;
}


void puGroup::draw ( int dx, int dy )
{
  if ( ! isVisible () )
    return ;

  for ( puObject *bo = dlist ; bo != NULL ; bo = bo -> getNextObject() )
  {
    /* June 16th, 98, Shammi :
     * The next if statement checks if the object is
     * a menu bar and makes sure it is repositioned
     * correctly.
     */

      int x, y ;
      int xdraw = dx + abox.min[0] ;
      int ydraw = dy + abox.min[1] ;
      bo -> getPosition (&x, &y) ;

    /* Introduced PUCLASS_VERTMENU into the club of widgets being automatically moved */
    /* to the top-left corner of the screen. This eliminates all Vertmenu-resize and  */
    /* window-jumping problems.                                 - JCJ 31 May 2002     */

    /* If the object is a menubar or a vertmenu and supposed to be locked to the top, */
    /* then move it there. - JCJ 6 June 2002                                          */
    if ( ( ( bo -> getType () & PUCLASS_MENUBAR ) || 
           ( ( bo -> getType () & PUCLASS_VERTMENU ) && 
           ( bo -> getVStatus () == 1 ) ) ) &&
         ( bo -> getWindow () == puGetWindow () ) )
    {
      int obWidth, obHeight ;
      bo -> getSize ( &obWidth, &obHeight ) ;
      bo -> setPosition ( 0, puGetWindowHeight() - obHeight ) ;
    } 
    else 
    {
      if ( ( y < 0 ) && ( bo->getType () & PUCLASS_POPUPMENU ) )
      {
    /* IF the object's bottom left corner lies outside the window, THEN */
    /* move the object to the top left         - JCJ and Fay 5 Jun 2002 */
        int absx, absy ;
        bo -> getAbsolutePosition (&absx, &absy) ;
        if ( absy < 0 )
          ydraw -= absy ;
      }
    }

    bo -> draw ( xdraw, ydraw ) ;
  }
}


void puGroup::recalc_bbox ( void ) 
{
  puBox contents ;
  contents . empty () ;

  for ( puObject *bo = dlist ;
        bo != NULL ;
        bo = bo -> getNextObject() )
    contents . extend ( bo -> getBBox() ) ;

  if ( contents . isEmpty () )
  {
    abox . max[0] = abox . min[0] ;
    abox . max[1] = abox . min[1] ;
  }
  else
  {
    abox . max[0] = abox . min[0] + contents . max[0] ;
    abox . max[1] = abox . min[1] + contents . max[1] ;
  }

  puObject::recalc_bbox () ;
}


void puGroup::doHit ( int, int, int, int )
{
}


puGroup::~puGroup ()
{
  puObject *bo = getLastChild () ;

  while ( bo != NULL )
  {
    dlist = bo    ;
    bo = bo -> getPrevObject() ;
    puDeleteObject ( dlist )  ;
  }
}


void puGroup::setChildStyle ( int childs, int which, int recursive )
{
  puObject *curr_obj ;

  for ( curr_obj = getFirstChild () ;
        curr_obj != NULL ;
        curr_obj = curr_obj->getNextObject () )
  {
    if ( ( recursive == TRUE ) && ( curr_obj->getType () & PUCLASS_GROUP ) )
      ((puGroup*)curr_obj)->setChildStyle ( childs, style, TRUE ) ;
    else
    {
      if ( curr_obj->getType () & childs )
        curr_obj->setStyle ( style ) ;
    }
  }
}

void puGroup::setChildBorderThickness ( int childs, int t, int recursive )
{
  puObject *curr_obj ;

  for ( curr_obj = getFirstChild () ;
        curr_obj != NULL ;
        curr_obj = curr_obj->getNextObject () )
  {
    if ( ( recursive == TRUE ) && ( curr_obj->getType () & PUCLASS_GROUP ) )
      ((puGroup*)curr_obj)->setChildBorderThickness ( childs, t, TRUE ) ;
    else
    {
      if ( curr_obj->getType () & childs )
        curr_obj->setBorderThickness ( t ) ;
    }
  }
}

void puGroup::setChildColour ( int childs, int which, float r, float g, float b, float a, int recursive )
{
  puObject *curr_obj ;

  for ( curr_obj = getFirstChild () ;
        curr_obj != NULL ;
        curr_obj = curr_obj->getNextObject () )
  {
    if ( ( recursive == TRUE ) && ( curr_obj->getType () & PUCLASS_GROUP ) )
      ((puGroup*)curr_obj)->setChildColour ( childs, which, r, g, b, a, TRUE ) ;
    else
    {
      if ( curr_obj->getType () & childs )
        curr_obj->setColour ( which, r, g, b, a ) ;
    }
  }
}

void puGroup::setChildColourScheme ( int childs, float r, float g, float b, float a, int recursive )
{
  puObject *curr_obj ;

  for ( curr_obj = getFirstChild () ;
        curr_obj != NULL ;
        curr_obj = curr_obj->getNextObject () )
  {
    if ( ( recursive == TRUE ) && ( curr_obj->getType () & PUCLASS_GROUP ) )
      ((puGroup*)curr_obj)->setChildColourScheme ( childs, r, g, b, a, TRUE ) ;
    else
    {
      if ( curr_obj->getType () & childs )
        curr_obj->setColourScheme ( r, g, b, a ) ;
    }
  }
}

void puGroup::setChildLegendFont ( int childs, puFont f, int recursive )
{
  puObject *curr_obj ;

  for ( curr_obj = getFirstChild () ;
        curr_obj != NULL ;
        curr_obj = curr_obj->getNextObject () )
  {
    if ( ( recursive == TRUE ) && ( curr_obj->getType () & PUCLASS_GROUP ) )
      ((puGroup*)curr_obj)->setChildLegendFont ( childs, f, TRUE ) ;
    else
    {
      if ( curr_obj->getType () & childs )
        curr_obj->setLegendFont ( f ) ;
    }
  }
}

void puGroup::setChildLabelFont ( int childs, puFont f, int recursive )
{
  puObject *curr_obj ;

  for ( curr_obj = getFirstChild () ;
        curr_obj != NULL ;
        curr_obj = curr_obj->getNextObject () )
  {
    if ( ( recursive == TRUE ) && ( curr_obj->getType () & PUCLASS_GROUP ) )
      ((puGroup*)curr_obj)->setChildLabelFont ( childs, f, TRUE ) ;
    else
    {
      if ( curr_obj->getType () & childs )
        curr_obj->setLabelFont ( f ) ;
    }
  }
}

