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


#include "sl.h"

float slEnvelope::getValue ( float _time ) const
{
  float delta ;
  int   step = getStepDelta ( &_time, &delta ) ;

  return delta * (_time - time[step]) + value[step] ;
}



int slEnvelope::getStepDelta ( float *_time, float *delta ) const
{
  float tt ;

  if ( replay_mode == SL_SAMPLE_LOOP )
  {
    tt = (float) floor ( *_time / time [ nsteps-1 ] ) ; 
    *_time -= tt * time [ nsteps-1 ] ;
  }

  tt = *_time ;

  if ( tt <= time[    0   ] ) { *delta = 0.0f ; return 0 ; }
  if ( tt >= time[nsteps-1] ) { *delta = 0.0f ; return nsteps-1 ; }

  for ( int i = 1 ; i <= nsteps-1 ; i++ )
    if ( tt <= time[i] )
    {
      float t1 = time[i-1] ; float v1 = value[i-1] ;
      float t2 = time[ i ] ; float v2 = value[ i ] ;

      if ( t1 == t2 )
      {
        *delta = 0.0f ;
	return i ;
      }

      *delta = (v2-v1) / (t2-t1) ;
      return i-1 ;
    }

  *delta = 0.0f ;
  return nsteps - 1 ;
}


void slEnvelope::applyToPitch  ( Uchar *dst, slPlayer *src,
                                 int nframes, int start, int next_env )
{
  float  delta ;
  float  _time = slScheduler::getCurrent() -> getElapsedTime ( start ) ;
  int     step = getStepDelta ( &_time, &delta ) ;
  float _value = delta * (_time - time[step]) + value[step] ;

  delta /= (float) slScheduler::getCurrent() -> getRate () ;

  unsigned char tmp [ 512 ] ;
  float  pos = 0 ;
  float npos = 0 ;
  unsigned char last = 0x80 ;

  while ( nframes-- )
  {
    npos += _value ;
    _value += delta ;

    int offset = (int) ( npos - pos ) ;

    if ( offset > 512 )
      offset = 512 ;

    if ( offset < 1 )
      *(dst++) = last ;
    else
    {
      pos += offset ;

      src -> read ( offset, tmp, next_env ) ;

      *(dst++) = last = tmp [ offset-1 ] ; 
    }
  }
}


void slEnvelope::applyToInvPitch  ( Uchar *dst, slPlayer *src,
                                    int nframes, int start, int next_env )
{
  float  delta ;
  float  _time = slScheduler::getCurrent() -> getElapsedTime ( start ) ;
  int     step = getStepDelta ( &_time, &delta ) ;
  float _value = delta * (_time - time[step]) + value[step] ;

  delta /= (float) slScheduler::getCurrent() -> getRate () ;

  unsigned char tmp [ 512 ] ;
  float  pos = 0 ;
  float npos = 0 ;
  unsigned char last = 0x80 ;

  while ( nframes-- )
  {
    npos += 1.0f / _value ;
    _value += delta ;

    int offset = (int) ( npos - pos ) ;

    if ( offset > 512 )
      offset = 512 ;

    if ( offset < 1 )
      *(dst++) = last ;
    else
    {
      pos += offset ;

      src -> read ( offset, tmp, next_env ) ;

      *(dst++) = last = tmp [ offset-1 ] ; 
    }
  }
}

void slEnvelope::applyToVolume ( Uchar *dst, Uchar *src,
                                 int nframes, int start )
{
  float  delta ;
  float  _time = slScheduler::getCurrent() -> getElapsedTime ( start ) ;
  int     step = getStepDelta ( &_time, &delta ) ;
  float _value = delta * (_time - time[step]) + value[step] ;

  delta /= (float) slScheduler::getCurrent() -> getRate () ;

  while ( nframes-- )
  {
    register int res = (int)( (float)((int)*(src++)-0x80) * _value ) + 0x80 ;

    _value += delta ;

    *(dst++) = ( res > 255 ) ? 255 : ( res < 0 ) ? 0 : res ;
  }
}

void slEnvelope::applyToInvVolume ( Uchar *dst, Uchar *src,
                                    int nframes, int start )
{
  float  delta ;
  float  _time = slScheduler::getCurrent() -> getElapsedTime ( start ) ;
  int     step = getStepDelta ( &_time, &delta ) ;
  float _value = delta * (_time - time[step]) + value[step] ;

  delta /= (float) slScheduler::getCurrent() -> getRate () ;

  delta = - delta ;
  _value = 1.0f - _value ;

  while ( nframes-- )
  {
    register int res = (int)( (float)((int)*(src++)-0x80) * _value ) + 0x80 ;

    _value += delta ;

    *(dst++) = ( res > 255 ) ? 255 : ( res < 0 ) ? 0 : res ;
  }
}


