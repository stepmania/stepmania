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



#include "ssgAux.h"


class ssgaFire : public ssgaParticleSystem
{
  int    tableSize   ;
  float *colourTable ;
  float *sizeTable   ;

  sgVec4 hot_colour  ;
  float max_ttl      ;
  float start_size   ;
  float upward_speed ;
  float radius       ;

  void reInit () ;

public:

  void createParticle ( int idx, ssgaParticle *p ) ;
  void updateParticle ( int idx, ssgaParticle *p ) ;

  ssgaFire ( int num_tris, float _radius = 1.0f, 
                           float height  = 5.0f,
                           float speed   = 2.0f ) ;

  virtual ~ssgaFire () ;

  virtual void update ( float t ) ;

  void setUpwardSpeed ( float spd )
  {
    upward_speed = spd ;
  }

  void setHeight    ( float hgt )
  {
    max_ttl = hgt / upward_speed ;
    getBSphere () -> setRadius ( hgt * 2.0f ) ;
    getBSphere () -> setCenter ( 0, 0, 0 ) ;
    reInit () ;
  }

  void setHotColour ( sgVec4 col )
  {
    sgCopyVec4 ( hot_colour, col ) ;
    reInit () ;
  }

  void getHotColour ( sgVec4 col ) { sgCopyVec4 ( col, hot_colour ) ; }

} ;

unsigned char *_ssgaGetFireTexture () ;

