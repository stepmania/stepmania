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

void ssgTexTrans::copy_from ( ssgTexTrans *src, int clone_flags )
{
  ssgBaseTransform::copy_from ( src, clone_flags ) ;
}

ssgBase *ssgTexTrans::clone ( int clone_flags )
{
  ssgTexTrans *b = new ssgTexTrans ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgTexTrans::ssgTexTrans ( sgCoord *c )
{
  type = ssgTypeTexTrans () ;
  setTransform ( c ) ;
}

ssgTexTrans::ssgTexTrans (void)
{
  type = ssgTypeTexTrans () ;
}

ssgTexTrans::~ssgTexTrans (void)
{
}

void ssgTexTrans::setTransform ( sgVec3 xyz )
{
  sgMakeTransMat4 ( transform, xyz ) ;
}

void ssgTexTrans::setTransform ( sgCoord *xform, float sx, float sy, float sz  )
{
  sgMakeCoordMat4 ( transform, xform ) ;
  sgScaleVec3 ( transform[0], sx ) ;
  sgScaleVec3 ( transform[1], sy ) ;
  sgScaleVec3 ( transform[2], sz ) ;
}

void ssgTexTrans::setTransform ( sgCoord *xform )
{
  sgMakeCoordMat4 ( transform, xform ) ;
}

void ssgTexTrans::setTransform ( sgMat4 xform )
{
  sgCopyMat4 ( transform, xform ) ;
}

void ssgTexTrans::cull ( sgFrustum *f, sgMat4 m, int test_needed )
{
  if ( ! preTravTests ( &test_needed, SSGTRAV_CULL ) )
    return ;

  int cull_result = cull_test ( f, m, test_needed ) ;

  if ( cull_result == SSG_OUTSIDE )
    return ;

  _ssgLoadTexMatrix ( transform ) ;
  glMatrixMode ( GL_TEXTURE ) ;
  glLoadMatrixf ( (float *) transform ) ;
  glMatrixMode ( GL_MODELVIEW ) ;

  for ( ssgEntity *e = getKid ( 0 ) ; e != NULL ; e = getNextKid() )
    e -> cull ( f, m, cull_result != SSG_INSIDE ) ;

  glMatrixMode ( GL_TEXTURE ) ;
  glLoadIdentity () ;
  glMatrixMode ( GL_MODELVIEW ) ;
  _ssgUnloadTexMatrix () ;

  postTravTests ( SSGTRAV_CULL ) ; 
}



int ssgTexTrans::load ( FILE *fd )
{
  return ssgBaseTransform::load(fd) ;
}

int ssgTexTrans::save ( FILE *fd )
{
  return ssgBaseTransform::save(fd) ;
}



