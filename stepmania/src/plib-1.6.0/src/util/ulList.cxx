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
 
 
#include <stdio.h>
#include <stdlib.h>
#if defined(WIN32)
#include <windows.h>
#include <mmsystem.h>
#endif
 
#include "ul.h"
                                                                                
ulList::ulList ( int init )
{
  total = 0 ;
  next = 0 ;
  entity_list = new void * [ limit = (init <= 0) ? 1 : init ] ;
}                                                                               

 
ulList::~ulList (void)
{
  removeAllEntities () ;
  delete [] entity_list ;
}


void ulList::addEntity ( void *entity )
{
  sizeChk () ;
  entity_list [ total++ ] = entity ;
}


void ulList::addEntityBefore ( int i, void *entity )
{
  sizeChk () ;
  memmove ( &entity_list[i+1], &entity_list[i], sizeof(void *) * (total-i) ) ;
  entity_list [ i ] = entity ;
  total++ ;
}


void ulList::sizeChk (void)
{
  /* Room for one more Entity? */
 
  if ( total >= limit )
  {
    limit += limit ;
    void **nlist = new void * [ limit ] ;
    memmove ( nlist, entity_list, sizeof(void *) * total ) ;
    delete [] entity_list ;
    entity_list = nlist ;
  }
}
 
 
int ulList::searchForEntity ( void *entity ) const
{
  for ( unsigned int i = 0 ; i < total ; i++ )
    if ( entity_list [ i ] == entity )
      return (int) i ;
 
  return -1 ;
}
                                                                                 
void ulList::removeAllEntities ()
{
  while ( total > 0 )
    removeEntity ( (unsigned int) 0 ) ;
}
 
void ulList::removeEntity ( unsigned int n )
{
  memmove ( &(entity_list[n]), &(entity_list[n+1]),
                               sizeof(void *) * (total-n-1) ) ;
  total-- ;
 
  if ( next >= n )
    next-- ;
}
                                                                                

 
void ulList::replaceEntity ( unsigned int n, void *new_entity )
{
  entity_list [ n ] = new_entity;
}                                                                               


