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
 
     For further information visit http://plib.sourceforge.net                  */


#include "ulRTTI.h"

/* Originally written by: Alexandru C. Telea <alext@win.tue.nl> */


static const ulRTTITypeinfo *RTTI_base_null_type [] = { 0 } ;

const ulRTTITypeinfo* ulRTTIdyntypeid::a[] = { 0 } ;

const ulRTTITypeinfo ulRTTITypeinfo::null_type ( "NULL", RTTI_base_null_type,
                                                 0, 0 ) ;


ulRTTITypeinfo::ulRTTITypeinfo ( const char *name, const ulRTTITypeinfo *bb[],
                                 void* (*f1)(int,void*),void* (*f2)() )
{
  /* Create default ulRTTITypeinfo */
  int name_sz = strlen ( name ) + 1 ;
  n = new char [ name_sz ] ;
  memcpy ( n, name, name_sz ) ;

  b       = bb ; /* ns = 0 ; subtypes = 0 ; */

  cast    = f1 ; /* Attach casting func  */
  new_obj = f2 ; /* Attach creation func */

  for ( int i = 0 ; b[i] ; i++ )
  /* Add this as subtype to all its basetypes */
    /* REMARK: Harmless const castaway */
    ((ulRTTITypeinfo**)b)[i]->add_subtype ( this ) ;
}

ulRTTITypeinfo::~ulRTTITypeinfo ()
{
  delete [] n ;
  for ( int i = 0 ; b[i] ; i++ )
  /* Del this subtype from all its basetypes */
    /* REMARK: Harmless const castaway */
    ((ulRTTITypeinfo**)b)[i]->del_subtype ( this ) ;
}

void ulRTTITypeinfo::add_subtype ( const ulRTTITypeinfo *t )
/*
  Adds t as last ulRTTITypeinfo in the 'subtypes' list. For this, the list is
  realloc'd with one extra entry.
*/
{
  const ulRTTITypeinfo **ptr = new const ulRTTITypeinfo*[ns+1] ;
  int i ; for ( i = 0 ; i < ns ; i++ ) ptr[i] = subtypes[i] ;
  ptr[i] = t ;
  ns++ ;
  delete[] subtypes ;
  subtypes = ptr ;
}

void ulRTTITypeinfo::del_subtype ( const ulRTTITypeinfo* t )
/* Searches for t in the subtypes list of this and removes it, if found. */
{
  int i ; for ( i = 0 ; i < ns && subtypes[i] != t ; i++ ) ;
  if ( i < ns )
    for(; i < ns-1 ; i++ ) subtypes[i] = subtypes[i+1] ;
}

void * ulRTTITypeinfo::create ( const ulRTTITypeinfo* bt, const char *c ) const
/*
  Tries to create an obj of type-name given by char*. Searches for this type
  in the type-DAG rooted by this, creates it and returns it as cast to 'bt',
  where bt is either this or a direct base of this.
*/
{
  void *p = NULL ; int i ;

  if ( !strcmp ( c, n ) ) /* Want to create an obj of this type ? */
    /* Yes, do it if this type is instantiable. */
    p = (new_obj) ? new_obj () : 0 ;
  else  /* No, try with subclasses... */
    for ( i = 0 ; i < ns &&
          !( ( p = subtypes[i]->create ( this, c ) ) ) ; i++ ) ;
  /* Succeeded creating on ith subclass branch ? */
  if ( !p ) return 0 ; /* Couldn't create it in any way, abort. */
  if ( bt == this )
    i = -1 ; /* Must cast to this's own type (i==-1) */
  else
    /* Search to which base of this we should cast */
    for ( i = 0 ; b[i] && b[i] != bt ; i++ ) ;

   /* Found: cast to ith base of this */
   return cast(i,p) ; /* Cast to ith base of to this, return as void* */
}

