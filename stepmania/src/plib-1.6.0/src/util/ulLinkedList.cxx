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


#include "ulLocal.h"

void ulLinkedList::unlinkNode ( ulListNode *prev, ulListNode *node )
{
  /* Is this the first node ? */
  if ( prev == NULL )
    head = node -> getNext () ;
  else
    prev -> setNext ( node -> getNext () ) ;

  /* Is this the last node ? */
  if ( node -> getNext () == NULL )
    tail = prev ;
}


int ulLinkedList::getNodePosition ( void *data ) const
{
  ulListNode *curr = head ;
  int pos = 0 ;

  while ( curr != NULL )
  {
    if ( curr -> getData () == data )
      return pos ;

    pos++ ;
    curr = curr -> getNext () ;
  }

  return -1 ;
}


void ulLinkedList::appendNode ( void *data )
{
  ulListNode *new_node = new ulListNode ( data, NULL ) ;

  if ( head == NULL )
    head = new_node ;
  else
    tail -> setNext ( new_node ) ;

  tail = new_node ;

  if ( ++nnodes > 1 )
    sorted = false ;
}

void ulLinkedList::insertNode ( void *data, int pos )
{
  if ( pos == 0 )
  {
    head = new ulListNode ( data, head ) ;

    if ( tail == NULL )
      tail = head ;
  }
  else
  {
    if ( ! isValidPosition ( pos ) )
      return ;
    else
    {
      ulListNode *prev = head ;

      while ( --pos > 0 )
        prev = prev -> getNext () ;

      prev -> setNext ( new ulListNode ( data, prev -> getNext () ) ) ;
    }
  }

  if ( ++nnodes > 1 )
    sorted = false ;
}


int ulLinkedList::insertSorted ( void *data, ulCompareFunc comparefn )
{
  if ( comparefn != NULL )
  {
    if ( sorted )
    {
      int pos = 0 ;

      if ( head == NULL )
        head = tail = new ulListNode ( data, NULL ) ;
      else
      {
        ulListNode *curr = head, *prev = NULL ;

        while ( (*comparefn)( curr -> getData (), data ) < 0 )
        {
          prev = curr ;
          curr = curr -> getNext () ;

          pos++ ;

          if ( curr == NULL )
          {
            tail = new ulListNode ( data, curr ) ;
            prev -> setNext ( tail ) ;

            nnodes++ ;
            return pos ;
          }
        }

        if ( prev == NULL )
          head = new ulListNode ( data, head ) ;
        else
          prev -> setNext ( new ulListNode ( data, curr ) ) ;
      }

      nnodes++ ;
      return pos ;
    }
    else
      ulSetError ( UL_WARNING,
                   "ulLinkedList::insertSorted: This is not a sorted list !" ) ;
  }

  return -1 ;
}


void ulLinkedList::removeNode ( void *data )
{
  ulListNode *curr = head, *prev = NULL ;

  while ( curr != NULL )
  {
    if ( curr -> getData () == data )
    {
      unlinkNode ( prev, curr ) ;

      delete curr ;

      if ( --nnodes <= 0 )
        sorted = true ;

      return ;
    }

    prev = curr ;
    curr = curr -> getNext () ;
  }

  ulSetError ( UL_WARNING, "ulLinkedList::removeNode: No such node" ) ;
}

void * ulLinkedList::removeNode ( int pos )
{
  if ( ! isValidPosition ( pos ) )
    return NULL ;

  ulListNode *curr = head, *prev = NULL ;

  while ( pos-- > 0 )
  {
    prev = curr ;
    curr = curr -> getNext () ;
  }

  unlinkNode ( prev, curr ) ;

  void *datap = curr -> getData () ;

  delete curr ;

  if ( --nnodes <= 1 )
    sorted = true ;

  return datap ;
}


void * ulLinkedList::getNodeData ( int pos ) const
{
  if ( ! isValidPosition ( pos ) )
    return NULL ;

  ulListNode *node ;

  if ( pos == nnodes - 1 )
    node = tail ;
  else
  {
    node = head ;

    while ( pos-- > 0 )
      node = node -> getNext () ;
  }

  return node -> getData () ;
}


void * ulLinkedList::forEach ( ulIterateFunc fn, void *user_data ) const
{
  if ( fn != NULL )
  {
    ulListNode *curr ;

    for ( curr = head ; curr != NULL ; curr = curr -> getNext () )
    {
      if ( (*fn)( curr -> getData (), user_data ) == false )
        return curr -> getData () ;
    }
  }

  return NULL ;
}


void ulLinkedList::empty ( ulIterateFunc destroyfn, void *user_data )
{
  ulListNode *curr = head ;

  if ( destroyfn != NULL )
  {
    while ( curr != NULL )
    {
      ulListNode *next = curr -> getNext () ;

      (*destroyfn) ( curr -> getData (), user_data ) ;

      delete curr ;
      curr = next ;
    }
  }
  else
  {
    while ( curr != NULL )
    {
      ulListNode *next = curr -> getNext () ;

      delete curr ;
      curr = next ;
    }
  }

  head = tail = NULL ;
  nnodes = 0 ;
  sorted = true ;
}

