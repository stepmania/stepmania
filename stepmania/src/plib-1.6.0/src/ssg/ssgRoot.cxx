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

void ssgRoot::copy_from ( ssgRoot *src, int clone_flags )
{
  ssgBranch::copy_from ( src, clone_flags ) ;
}

ssgBase *ssgRoot::clone ( int clone_flags )
{
  ssgRoot *b = new ssgRoot ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgRoot::ssgRoot (void)
{
  type = ssgTypeRoot () ;
}

ssgRoot::~ssgRoot (void)
{
}


int ssgRoot::load ( FILE *fd )
{
  return ssgBranch::load(fd) ;
}

int ssgRoot::save ( FILE *fd )
{
  return ssgBranch::save(fd) ;
}



