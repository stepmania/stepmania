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



class ssgaParticle
{
public:

  sgVec4 col ;
  sgVec3 pos ;
  sgVec3 vel ;
  sgVec3 acc ;

  float size ;

  float time_to_live ;
  void *userData ;

  void update ( float dt )
  {
    sgAddScaledVec3 ( vel, acc, dt ) ;
    sgAddScaledVec3 ( pos, vel, dt ) ;
    time_to_live -= dt ;
  }

  ssgaParticle ()
  {
    sgSetVec4 ( col, 1, 1, 1, 1 ) ;
    sgZeroVec3 ( pos ) ;
    sgZeroVec3 ( vel ) ;
    sgZeroVec3 ( acc ) ;
    time_to_live = 0 ;
    userData = NULL ;
    size = 1.0f ;
  } 

} ;


class ssgaParticleSystem ;


typedef void (* ssgaParticleCreateFunc) ( ssgaParticleSystem *ps,
                                          int index,
                                          ssgaParticle *p ) ;

typedef void (* ssgaParticleUpdateFunc) ( float deltaTime,
                                          ssgaParticleSystem *ps,
                                          int index,
                                          ssgaParticle *p ) ;

typedef void (* ssgaParticleDeleteFunc) ( ssgaParticleSystem *ps,
                                          int index,
                                          ssgaParticle *p ) ;

class ssgaParticleSystem : public ssgVtxTable
{
  int num_particles  ;
  int num_verts      ;
  int turn_to_face   ;
  int num_active     ;
  ssgaParticle *particle ;

  float create_error ;
  float create_rate ;

  float size ;

  ssgaParticleCreateFunc particle_create ;
  ssgaParticleUpdateFunc particle_update ;
  ssgaParticleDeleteFunc particle_delete ;

public:

  ssgaParticleSystem ( int num, int initial_num,
                       float _create_rate, int _turn_to_face,
                       float sz, float bsphere_size,
                       ssgaParticleCreateFunc _particle_create,
                       ssgaParticleUpdateFunc _particle_update = NULL,
                       ssgaParticleDeleteFunc _particle_delete = NULL ) ;

  virtual ~ssgaParticleSystem () ;
  virtual void update ( float t ) ;

  void setSize ( float sz ) { size = sz ; }
  float getSize () { return size ; }

  void draw_geometry () ;

  void  setCreationRate ( float cr ) { create_rate = cr ; }
  float getCreationRate () { return create_rate ; }

  int getNumParticles       () { return num_particles ; }
  int getNumActiveParticles () { return num_active    ; }
} ;


