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

void ssgTimedSelector::copy_from ( ssgTimedSelector *src, int clone_flags )
{
  ssgSelector::copy_from ( src, clone_flags ) ;

  running = src -> running ;
  mode    = src -> mode    ;

  start_time    = src -> start_time ;
  pause_time    = src -> pause_time ;
  loop_time     = src ->  loop_time ;

  delete [] times ;
  times = new float [ max_kids ] ;
  for ( int i = 0 ; i < max_kids ; i++ )
    times [ i ]  = src -> times [ i ] ;

  curr  = src -> curr  ;
  start = src -> start ;
  end   = src -> end   ;
}


ssgBase *ssgTimedSelector::clone ( int clone_flags )
{
  ssgTimedSelector *b = new ssgTimedSelector ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}


ssgTimedSelector::ssgTimedSelector ( int max_kids ) : ssgSelector ( max_kids )
{
  type = ssgTypeTimedSelector () ;
  select ( 1 ) ;
  running = SSG_ANIM_STOP ;
  mode = SSG_ANIM_SHUTTLE ;
  start_time = pause_time = 0.0f ;
  loop_time = 1.0f ;

  times = new float [ max_kids ] ;
  for ( int i = 0 ; i < max_kids ; i++ )
    times [ i ] = 1.0f ;

  curr = start = end = 0 ;
}


void ssgTimedSelector::cull ( sgFrustum *f, sgMat4 m, int test_needed )
{
  compute_loop_time () ;
  selectStep ( getStep () ) ;
  ssgSelector::cull ( f, m, test_needed ) ;
}


void ssgTimedSelector::hot ( sgVec3 sp, sgMat4 m, int test_needed )
{
  selectStep ( getStep () ) ;
  ssgSelector::hot ( sp, m, test_needed ) ;
}

void ssgTimedSelector::los ( sgVec3 sp, sgMat4 m, int test_needed )
{
  selectStep ( getStep () ) ;
  ssgSelector::los ( sp, m, test_needed ) ;
}

void ssgTimedSelector::isect ( sgSphere *sp, sgMat4 m, int test_needed )
{
  selectStep ( getStep () ) ;
  ssgSelector::isect ( sp, m, test_needed ) ;
}


int ssgTimedSelector::getStep ()
{
  float t = (float) ssgGetFrameCounter () ;

  if ( running == SSG_ANIM_STOP || running == SSG_ANIM_PAUSE )
    return curr ;

  /* SSG_ANIM_START */

  t -= start_time ;  /* t is time since start of run */

  if ( mode == SSG_ANIM_ONESHOT )
  {
    if ( t >= loop_time )
    {
      running = SSG_ANIM_STOP ;
      return end ;
    }
  }
  else
  if ( mode == SSG_ANIM_SHUTTLE )      
  {
    /* Compute time since start of this loop */

    t = t - (float) floor ( t / loop_time ) * loop_time ;
  }
  else
  if ( mode == SSG_ANIM_SWING )      
  {
    /* Compute time since start of this swing loop */

    t = t - (float) floor ( t / (2.0f * loop_time) ) * (2.0f * loop_time) ;

    /* Are we on the reverse part of the loop? */
    if ( t >= loop_time )
      t = 2.0f * loop_time - t ;
  }

  int k ;

  for ( k = start ; t > 0.0f && k <= end ; k++ )
    t -= times [ k ] ;

//DaveM: i removed this line because, in shuttle mode, start plays twice
//  k-- ;

  if ( k < start ) k = start ;
  if ( k > end   ) k =   end ;

   curr = k ;
   return curr ;
}



ssgTimedSelector::~ssgTimedSelector (void)
{
  delete [] times ;
}



int ssgTimedSelector::load ( FILE *fd )
{
  _ssgReadInt   ( fd, (int *) & running ) ;
  _ssgReadInt   ( fd, (int *) & mode    ) ;
  _ssgReadFloat ( fd, & start_time ) ;
  _ssgReadFloat ( fd, & pause_time ) ;
  _ssgReadFloat ( fd, & loop_time  ) ;
  _ssgReadInt   ( fd, & max_kids  ) ;
  delete [] times ;
  times = new float [ max_kids ] ;
  _ssgReadFloat ( fd, max_kids, times    ) ;
  _ssgReadInt   ( fd, & curr  ) ;
  _ssgReadInt   ( fd, & start ) ;
  _ssgReadInt   ( fd, & end   ) ;

  return ssgSelector::load(fd) ;
}

int ssgTimedSelector::save ( FILE *fd )
{
  _ssgWriteInt   ( fd, (int) running ) ;
  _ssgWriteInt   ( fd, (int) mode    ) ;
  _ssgWriteFloat ( fd, start_time ) ;
  _ssgWriteFloat ( fd, pause_time ) ;
  _ssgWriteFloat ( fd, loop_time  ) ;
  _ssgWriteInt   ( fd, max_kids  ) ;
  _ssgWriteFloat ( fd, max_kids, times  ) ;
  _ssgWriteInt   ( fd, curr  ) ;
  _ssgWriteInt   ( fd, start ) ;
  _ssgWriteInt   ( fd, end   ) ;

  return ssgSelector::save(fd) ;
}


