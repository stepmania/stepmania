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



#include "ssgaShapes.h"

class ssgaLensFlare : public ssgaShape
{
  ssgVtxTable      *vt ;
  ssgVertexArray   *v0 ;
  ssgNormalArray   *n0 ;
  ssgColourArray   *c0 ;
  ssgTexCoordArray *t0 ;

  void update ( sgMat4 m ) ;

protected:
  virtual void copy_from ( ssgaLensFlare *src, int clone_flags ) ;
public:

  ssgaLensFlare () ;
  ssgaLensFlare ( int nt ) ;

  virtual ~ssgaLensFlare () ;

  virtual ssgBase    *clone       ( int clone_flags = 0 ) ;
  virtual void        regenerate  () ;
  virtual const char *getTypeName ( void ) ;
  virtual void        cull  ( sgFrustum *f, sgMat4 m, int test_needed ) ;
} ;

unsigned char *_ssgaGetLensFlareTexture () ;


