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

*/

#include "ssg.h"
#include "ssgAux.h"

#define EMBER_SCALE  3.0f

void _ssgaFireParticleCreate ( ssgaParticleSystem *ps,
                               int idx, ssgaParticle *p )
{
  ((ssgaFire *) ps) -> createParticle ( idx, p ) ;
}



void _ssgaFireParticleUpdate ( float, ssgaParticleSystem *ps,
                               int idx, ssgaParticle *p )
{
  ((ssgaFire *) ps) -> updateParticle ( idx, p ) ;
}




void ssgaFire::reInit ()
{
  setCreationRate ( (float)(getNumParticles()) / max_ttl ) ;

  delete colourTable ;
  delete sizeTable   ;

  tableSize = (int)( 10.0f * max_ttl ) ;

  colourTable = new float [ tableSize * 4 ] ;
  sizeTable   = new float [ tableSize     ] ;

  sgCopyVec4 ( & colourTable [ 0 ], hot_colour ) ;
  sizeTable [ 0 ] = start_size ;

  for ( int i = 1 ; i < tableSize ; i++ )
  {
    sizeTable [ i ] = sizeTable [ i-1 ] * 1.06f ;

    if ( sizeTable [ i ] >= 1.5 ) sizeTable [ i ] = 1.5 ;

    sgScaleVec3 ( & colourTable [   i  * 4 ],
		  & colourTable [ (i-1)* 4 ], 0.90f ) ;

    colourTable [ i * 4 + 3 ] = 1.0f ;
  }
}


void ssgaFire::createParticle ( int idx, ssgaParticle *p )
{
  float xx = (float)(rand()%1000)/500.0f * radius - radius ;
  float yy = (float)sqrt ( radius * radius - xx * xx ) ;

  yy = (float)(rand()%1000)/500.0f * yy - yy ;

  p -> time_to_live = max_ttl ;
  p -> size = sizeTable [ 0 ] ;

  sgCopyVec4 ( p -> col, & colourTable [ 0 ] ) ;

  if ( (idx & 3) != 0 )
  {
    sgSetVec3 ( p -> pos, xx, yy, -p->size ) ;
    sgSetVec3 ( p -> vel, 0, 0, upward_speed ) ;
  }
  else
  {
    p->size *= EMBER_SCALE ;

    sgSetVec3 ( p -> pos, xx, yy, 0.0f ) ;
    sgSetVec3 ( p -> vel, 0, 0, 0 ) ;
  }

  sgSetVec3 ( p -> acc, 0, 0, 0 ) ;
}




void ssgaFire::updateParticle ( int idx, ssgaParticle *p )
{
  int tick = (int)( ( max_ttl - p->time_to_live ) * 10.0f ) ;

  if ( tick >= tableSize )
  {
    p->time_to_live = 0.0f ;
    return ;
  }

  if ( (idx & 3) != 0 )
    p -> size = sizeTable [ tick ] ;
  else
    p -> size = sizeTable [ tick ] * EMBER_SCALE ;

  sgCopyVec4 ( p -> col, & colourTable [ tick * 4 ] ) ;
}


static ssgSimpleState *fireState   = NULL ;
static ssgTexture     *fireTexture = NULL ;                                   

static int preFireDraw ( ssgEntity * )
{
  glBlendFunc ( GL_ONE, GL_ONE ) ;
  return TRUE ;
}


static int postFireDraw ( ssgEntity * )
{
  glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;
  return TRUE ;
}                                                                              

ssgaFire::ssgaFire ( int num,
                     float _radius,
                     float height,
                     float speed )
            : ssgaParticleSystem ( num/2,
                                  0, 0,             // <== Don't Change this!!
                                  TRUE, 1.0f, height * 2.0f,
                                  _ssgaFireParticleCreate,
                                  _ssgaFireParticleUpdate )
{
  radius = _radius ;
  upward_speed = speed ;
  start_size = 0.6f ;

  if ( fireState == NULL )
  {
    /*
      The texture constructor deletes the texture from
      main memory after it's finished with it!!  YIKES!!
    */
 
    unsigned char *t = new unsigned char [ 32 * 32 ] ;
    memcpy ( t, _ssgaGetFireTexture(), 32 * 32 ) ;
 
    fireTexture = new ssgTexture ( "NONE", t, 32, 32, 1 ) ;
 
    fireState = new ssgSimpleState () ;
    fireState -> setTexture        ( fireTexture ) ;
    fireState -> setTranslucent    () ;
    fireState -> enable            ( GL_TEXTURE_2D ) ;
    fireState -> enable            ( GL_BLEND ) ;
    fireState -> disable           ( GL_LIGHTING ) ;
    fireState -> ref () ;
  }


  tableSize   = 0 ;
  colourTable = NULL ;
  sizeTable   = NULL ;

  sgSetVec4 ( hot_colour, 1.0f, 0.2f, 0.1f, 1.0f ) ;

  setHeight ( height ) ;  /* Forces a reInit */

  setState    ( fireState ) ;
  setCallback ( SSG_CALLBACK_PREDRAW , preFireDraw  ) ;
  setCallback ( SSG_CALLBACK_POSTDRAW, postFireDraw ) ;
}

void ssgaFire::update ( float t )
{
  ssgaParticleSystem::update ( t ) ;
}


unsigned char *_ssgaGetFireTexture () ;


ssgaFire::~ssgaFire ()
{
  tableSize   = 0 ;
  delete [] colourTable ;
  delete [] sizeTable   ;
}




static unsigned char fuzzyBlob [] =
{
  0,0,0,0,0,0,0,0,0,0,0,0,1,3,4,5,5,4,3,1,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,1,5,9,14,17,19,20,20,20,17,14,10,5,1,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,1,5,12,19,25,29,33,35,36,37,36,33,30,25,20,13,6,1,0,0,0,0,0,0,0,
  0,0,0,0,0,0,2,10,19,27,34,40,45,48,51,52,53,51,49,45,40,35,28,20,11,3,0,0,0,0,0,0,
  0,0,0,0,0,4,13,23,33,41,48,55,60,64,67,69,69,67,65,61,56,49,42,34,25,15,5,0,0,0,0,0,
  0,0,0,0,4,14,26,36,46,55,63,70,75,80,83,85,85,83,80,76,70,64,56,47,37,27,16,5,0,0,0,0,
  0,0,0,2,13,26,37,48,59,68,77,84,91,95,99,101,101,99,96,91,85,78,69,60,50,39,27,15,3,0,0,0,
  0,0,1,10,23,36,48,60,71,81,90,99,105,111,115,117,117,115,111,106,100,92,82,72,61,50,38,25,12,1,0,0,
  0,0,5,19,33,46,59,71,83,94,104,113,120,126,130,133,133,131,127,121,114,105,95,84,72,60,47,34,21,7,0,0,
  0,1,12,27,41,55,68,81,94,105,116,126,134,141,146,149,149,147,142,135,127,118,107,95,83,70,56,43,29,14,2,0,
  0,5,19,34,48,63,77,90,104,116,128,139,148,156,162,165,165,162,157,149,140,129,118,105,92,78,64,50,36,21,6,0,
  0,9,25,40,55,70,84,99,113,126,139,151,161,170,177,181,181,178,171,163,152,140,128,114,100,86,72,57,42,27,11,0,
  1,14,29,45,60,75,91,105,120,134,148,161,173,184,192,196,197,193,185,175,163,150,136,122,107,92,77,62,47,31,16,2,
  3,17,33,48,64,80,95,111,126,141,156,170,184,196,206,212,212,207,197,185,172,158,143,128,113,97,82,66,50,35,19,4,
  4,19,35,51,67,83,99,115,130,146,162,177,192,206,219,227,228,220,208,194,179,163,148,132,117,101,85,69,53,37,21,5,
  5,20,36,52,69,85,101,117,133,149,165,181,196,212,227,241,242,229,214,198,182,167,151,135,119,103,87,71,54,38,22,6,
  5,20,37,53,69,85,101,117,133,149,165,181,197,212,228,242,243,230,214,199,183,167,151,135,119,103,87,71,55,39,22,6,
  4,20,36,51,67,83,99,115,131,147,162,178,193,207,220,229,230,221,209,194,179,164,148,133,117,101,85,69,53,38,22,6,
  3,17,33,49,65,80,96,111,127,142,157,171,185,197,208,214,214,209,199,187,173,159,144,129,113,98,82,67,51,35,19,4,
  1,14,30,45,61,76,91,106,121,135,149,163,175,185,194,198,199,194,187,176,164,151,137,123,108,93,78,63,47,32,16,2,
  0,10,25,40,56,70,85,100,114,127,140,152,163,172,179,182,183,179,173,164,154,142,129,115,101,87,72,57,42,27,12,1,
  0,5,20,35,49,64,78,92,105,118,129,140,150,158,163,167,167,164,159,151,142,131,119,106,93,79,65,51,36,22,7,0,
  0,1,13,28,42,56,69,82,95,107,118,128,136,143,148,151,151,148,144,137,129,119,108,96,84,71,57,44,30,15,2,0,
  0,0,6,20,34,47,60,72,84,95,105,114,122,128,132,135,135,133,129,123,115,106,96,85,74,61,49,35,22,8,0,0,
  0,0,1,11,25,37,50,61,72,83,92,100,107,113,117,119,119,117,113,108,101,93,84,74,63,51,39,26,13,2,0,0,
  0,0,0,3,15,27,39,50,60,70,78,86,92,97,101,103,103,101,98,93,87,79,71,61,51,40,28,16,4,0,0,0,
  0,0,0,0,5,16,27,38,47,56,64,72,77,82,85,87,87,85,82,78,72,65,57,49,39,28,17,6,0,0,0,0,
  0,0,0,0,0,5,15,25,34,43,50,57,62,66,69,71,71,69,67,63,57,51,44,35,26,16,6,0,0,0,0,0,
  0,0,0,0,0,0,3,12,21,29,36,42,47,50,53,54,55,53,51,47,42,36,30,22,13,4,0,0,0,0,0,0,
  0,0,0,0,0,0,0,1,7,14,21,27,31,35,37,38,39,38,35,32,27,22,15,8,2,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,2,6,11,16,19,21,22,22,22,19,16,12,7,2,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,2,4,5,6,6,6,4,2,1,0,0,0,0,0,0,0,0,0,0,0
} ;


unsigned char *_ssgaGetFireTexture () { return fuzzyBlob ; }

