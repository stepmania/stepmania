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


#include "ssgLocal.h"

void ssgSimpleList::copy_from ( ssgSimpleList *src, int clone_flags )
{
  ssgBase::copy_from ( src, clone_flags ) ;

  delete [] list ;
  size_of = src -> getSizeOf () ;
  total   = src -> getNum () ;
  limit   = total ;
  list    = new char [ limit * size_of ] ;
  memcpy ( list, src->raw_get ( 0 ), limit * size_of ) ;
}

ssgBase *ssgSimpleList::clone ( int clone_flags )
{
  ssgSimpleList *b = new ssgSimpleList () ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}

ssgBase *ssgNormalArray::clone ( int clone_flags )
{
  ssgNormalArray *b = new ssgNormalArray () ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}

ssgBase *ssgVertexArray::clone ( int clone_flags )
{
  ssgVertexArray *b = new ssgVertexArray () ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}

ssgBase *ssgTexCoordArray::clone ( int clone_flags )
{
  ssgTexCoordArray *b = new ssgTexCoordArray () ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}

ssgBase *ssgColourArray::clone ( int clone_flags )
{
  ssgColourArray *b = new ssgColourArray () ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}



 
void ssgVertexArray::print ( FILE *fd, char *indent, int how_much )
{
  ssgSimpleList::print ( fd, indent, how_much ) ;

  if ( how_much < 4 )
    return;

  for ( unsigned int i = 0 ; i < total ; i++ )
    fprintf ( fd, "%s  V%d) { %f, %f, %f }\n", indent, i,
                     get(i)[0], get(i)[1], get(i)[2] ) ;
}
 


 
void ssgNormalArray::print ( FILE *fd, char *indent, int how_much )
{
  ssgSimpleList::print ( fd, indent, how_much ) ;

  if ( how_much < 4 )
    return;

  for ( unsigned int i = 0 ; i < total ; i++ )
    fprintf ( fd, "%s  N%d) { %f, %f, %f }\n", indent, i,
                     get(i)[0], get(i)[1], get(i)[2] ) ;
}
 


void ssgIndexArray::print ( FILE *fd, char *indent, int how_much )
{
  ssgSimpleList::print ( fd, indent, how_much ) ;

  if ( how_much < 4 )
    return;

  for ( unsigned int i = 0 ; i < total ; i++ )
    fprintf ( fd, "%s  I%d) { %d }\n", indent, i,
                     (int) (*get(i)) ) ;
}
 
void ssgTexCoordArray::print ( FILE *fd, char *indent, int how_much )
{
  ssgSimpleList::print ( fd, indent, how_much ) ;

  if ( how_much < 4 )
    return;

  for ( unsigned int i = 0 ; i < total ; i++ )
    fprintf ( fd, "%s  T%d) { S=%f, T=%f }\n", indent, i,
                     get(i)[0], get(i)[1] ) ;
}
 


 
void ssgColourArray::print ( FILE *fd, char *indent, int how_much )
{
  ssgSimpleList::print ( fd, indent, how_much ) ;

  if ( how_much < 4 )
    return;

  for ( unsigned int i = 0 ; i < total ; i++ )
    fprintf ( fd, "%s  C%d) { R=%f, G=%f, B=%f, A=%f }\n", indent, i,
                     get(i)[0], get(i)[1], get(i)[2], get(i)[3] ) ;
}
 


void ssgSimpleList::print ( FILE *fd, char *indent, int how_much )
{
  ssgBase::print ( fd, indent, how_much ) ;

  fprintf ( fd, "%s  Total # items = %d\n", indent, total ) ;

  if ( how_much < 3 )
    return;

  fprintf ( fd, "%s  Size of items = %d bytes\n", indent, size_of ) ;
}
 


int ssgSimpleList::load ( FILE *fd )
{
  delete [] list ;
  _ssgReadUInt ( fd, &size_of ) ;
  _ssgReadUInt ( fd, &total   ) ;
  limit = total ;
  list = new char [ limit * size_of ] ;
  assert(list!=NULL);
  // wk: The old code:
  //_ssgReadFloat ( fd, limit * size_of / sizeof(float), (float *)list ) ;
  // doesn't work since some ssgSimpleLists consist of shorts, so
  // limit * size_of may not be divisible by sizeof(float).
  // The new code works, but I am not 100% sure what we want in
  // the event that there are machines with another sizeof(float).
  _ssgReadBytes   ( fd, limit * size_of , list) ;

  return ! _ssgReadError () ;
}



int ssgSimpleList::save ( FILE *fd )
{
  _ssgWriteUInt ( fd, size_of ) ;
  _ssgWriteUInt ( fd, total   ) ;
  // see comment in ssgSimpleList::load
  // _ssgWriteFloat( fd, total * size_of / sizeof(float), (float *)list ) ;
  _ssgWriteBytes ( fd, total * size_of, list);
  return ! _ssgWriteError () ;
}


