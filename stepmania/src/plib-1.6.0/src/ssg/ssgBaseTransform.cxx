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

void ssgBaseTransform::copy_from ( ssgBaseTransform *src, int clone_flags )
{
  ssgBranch::copy_from ( src, clone_flags ) ;

  src -> getTransform     ( transform ) ;
  src -> getLastTransform ( last_transform ) ;
  last_updated = src -> last_updated ;
  first_time   = src -> first_time   ;
}



ssgBaseTransform::ssgBaseTransform (void)
{
  type = ssgTypeBaseTransform () ;
  last_updated = -9999999 ;
  first_time = TRUE ;
  sgMakeIdentMat4 ( transform ) ;
  sgMakeIdentMat4 ( last_transform ) ;
}

ssgBaseTransform::~ssgBaseTransform (void)
{
}


int ssgBaseTransform::load ( FILE *fd )
{
  _ssgReadMat4 ( fd, transform ) ;
  updateTransform () ;
  first_time = TRUE ;
  return ssgBranch::load(fd) ;
}

int ssgBaseTransform::save ( FILE *fd )
{
  _ssgWriteMat4 ( fd, transform ) ;
  return ssgBranch::save(fd) ;
}


void ssgBaseTransform::print ( FILE *fd, char *indent, int how_much )
{
  ssgBranch::print ( fd, indent, how_much ) ;

  if ( how_much >= 2 )
    for ( int row = 0 ; row < 4 ; row++ )
      fprintf ( fd, "%s  Transform[%d]= %f,%f,%f,%f\n", indent, row,
        transform[row][0], transform[row][1],
        transform[row][2], transform[row][3] ) ;
}


