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

typedef float (* ssgaWSDepthCallback ) ( float x, float y ) ;

#define SSGA_MAX_WAVETRAIN  16

class ssgaWaveTrain
{
  float height ;
  float length ;
  float lambda ;
  float speed  ;
  float heading;

public:

  ssgaWaveTrain ()
  {
    height  = 0.5f ;
    length  = 0.8f ;
    lambda  = 1.0f ;
    speed   = (float) sqrt ( 2.0f/3.0f ) ;
    heading = 0.0f ;
  }

  float getSpeed () { return speed  ; }
  void  setSpeed ( float h ) { speed  = h ; }

  float getLength () { return length  ; }
  void  setLength ( float h ) { length  = h ; }

  float getLambda () { return lambda  ; }
  void  setLambda ( float h ) { lambda  = h ; }

  float getHeading () { return heading  ; }
  void  setHeading ( float h ) { heading  = h ; }

  float getWaveHeight () { return height  ; }
  void  setWaveHeight ( float h ) { height  = h ; }
} ;

class ssgaWaveSystem : public ssgaShape
{
  ssgaWSDepthCallback gridGetter ;

  sgVec3 *normals   ;
  sgVec4 *colours   ;
  sgVec2 *texcoords ;
  sgVec3 *vertices  ;
  sgVec3 *orig_vertices  ;

  ssgaWaveTrain *train [ SSGA_MAX_WAVETRAIN ] ;

  float windSpeed   ;
  float windHeading ;
  float edgeFlatten ;

  float tu, tv ;

  int nstrips ;
  int nstacks ;

protected:
  virtual void copy_from ( ssgaWaveSystem *src, int clone_flags ) ;
public:

  ssgaWaveSystem ( int ntri ) ;

  virtual ~ssgaWaveSystem () ;

  virtual ssgBase    *clone       ( int clone_flags = 0 ) ;
  virtual void        regenerate  () ;
  virtual const char *getTypeName ( void ) ;
 
  virtual int load ( FILE * ) ;
  virtual int save ( FILE * ) ;

  ssgaWSDepthCallback getDepthCallback () { return gridGetter ; } 

  ssgaWaveTrain *getWaveTrain ( int i )
  {
    return ( i < 0 || i >= SSGA_MAX_WAVETRAIN ) ? NULL : train [ i ] ;
  }

  void setWaveTrain ( int i, ssgaWaveTrain *t )
  {
    assert ( i >= 0 && i < SSGA_MAX_WAVETRAIN ) ;
    train [ i ] = t ;
  }

  float getWindSpeed     () { return windSpeed   ; }
  float getWindDirn      () { return windHeading ; }
  float getEdgeFlatten   () { return edgeFlatten ; }
  float getTexScaleU () { return tu ; }
  float getTexScaleV () { return tv ; }

  void  setDepthCallback ( ssgaWSDepthCallback cb ) { gridGetter  = cb ; } 
  void  setWindSpeed     ( float speed            ) { windSpeed   = speed ; }
  void  setWindDirn      ( float dirn             ) { windHeading = dirn  ; }
  void  setEdgeFlatten   ( float dist             ) { edgeFlatten = dist ; }
  void  setTexScale      ( float u, float v       ) { tu = u ; tv = v ; }

  void updateAnimation ( float t ) ;
} ;


