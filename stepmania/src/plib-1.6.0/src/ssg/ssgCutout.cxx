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

void ssgCutout::copy_from ( ssgCutout *src, int clone_flags )
{
  ssgBranch::copy_from ( src, clone_flags ) ;
  point_rotate = src -> isPointRotate () ;
}

ssgBase *ssgCutout::clone ( int clone_flags )
{
  ssgCutout *b = new ssgCutout ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgCutout::ssgCutout (int pntrot)
{
  point_rotate = pntrot ;
  type = ssgTypeCutout () ;
}

ssgCutout::~ssgCutout ()
{
}


void ssgCutout::cull ( sgFrustum *f, sgMat4 m, int test_needed )
{
  if ( ! preTravTests ( &test_needed, SSGTRAV_CULL ) )
    return ;

  sgMat4 tmp ;

  if ( point_rotate )
  {
    /*
      Trash the current viewpoint transform and replace it
      with one that only contains the OpenGL axis swap
      kludge along with the current translation.

      This prevents the object from rotating relative to the
      screen so that the modelled X/Z axis are always parallel
      to the screen X/Y axes.
    */

    sgCopyMat4 ( tmp, _ssgOpenGLAxisSwapMatrix ) ;
    sgCopyVec3 ( tmp[3], m[3] ) ;
  }
  else
  {
    sgCopyMat4 ( tmp, m ) ;

    /*
      Figure out where the Z axis of the model ends up - that
      isn't changed by the billboarding process.

      Next, figure out where the X axis should go - which
      will be at right angles to the Z axis - and at right
      angles to the view vector. A cross product achieves
      that.

      Finally, figure out where the Y axis should go - which
      will be at right angles to the new X and Z axes. A second
      cross product sorts that out.

      Notice that the SSG coordinate system's Z axis is really
      GL's Y axis.
    */

    sgVec3 x_axis ;
    sgVec3 y_axis ;
    sgVec3 z_axis ;

    sgSetVec3  ( y_axis, 0.0f, 0.0f, -1.0f ) ;
    sgCopyVec3 ( z_axis, tmp[ 2 ] ) ;

    sgVectorProductVec3 ( x_axis, y_axis, z_axis ) ;
    sgVectorProductVec3 ( y_axis, z_axis, x_axis ) ;

    sgNormaliseVec3 ( x_axis ) ;
    sgNormaliseVec3 ( y_axis ) ;  /* Optional if your cutout is flat */

    /*
      Now we know where we want the three axes to end up,
      change the matrix to make it so.
    */

    sgCopyVec3 ( tmp[0], x_axis ) ;
    sgCopyVec3 ( tmp[1], y_axis ) ;
  }

  _ssgPushMatrix ( tmp ) ;

  glPushMatrix   () ;
  glLoadMatrixf  ( (float *) tmp ) ;
  ssgBranch::cull ( f, tmp, test_needed ) ;
  glPopMatrix () ;

  _ssgPopMatrix  () ;

  postTravTests ( SSGTRAV_CULL ) ; 
}


void ssgCutout::hot ( sgVec3 s, sgMat4 m, int test_needed )
{
  ssgBranch::hot ( s, m, test_needed ) ;
}

void ssgCutout::los ( sgVec3 s, sgMat4 m, int test_needed )
{
  ssgBranch::los ( s, m, test_needed ) ;
}


void ssgCutout::isect ( sgSphere *s, sgMat4 m, int test_needed )
{
  ssgBranch::isect ( s, m, test_needed ) ;
}



int ssgCutout::load ( FILE *fd )
{
  _ssgReadInt ( fd, & point_rotate ) ;

  return ssgBranch::load(fd) ;
}

int ssgCutout::save ( FILE *fd )
{
  _ssgWriteInt ( fd, point_rotate ) ;

  return ssgBranch::save(fd) ;
}



