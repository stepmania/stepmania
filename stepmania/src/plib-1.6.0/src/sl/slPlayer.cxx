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

void slPlayer::addEnvelope ( int i, slEnvelope *_env, slEnvelopeType _type )
{
  if ( i < 0 || i >= SL_MAX_ENVELOPES ) return ;

  if ( env [ i ] != NULL )
    env [ i ] -> unRef () ;

  env [ i ] = _env ;

  if ( _env != NULL )
    env [ i ] -> ref () ;

  env_type [ i ] = _type ;
  env_start_time [ i ] = slScheduler::getCurrent() -> getTimeNow () ;
}

int slPlayer::preempt ( int delay )
{
  switch ( preempt_mode )
  {
    case SL_SAMPLE_CONTINUE: if ( isRunning() )
			       return SL_FALSE ;
			     /* FALLTHROUGH! */
    case SL_SAMPLE_DELAY   :                   break ;
    case SL_SAMPLE_MUTE    : skip  ( delay ) ; break ;
    case SL_SAMPLE_ABORT   : stop  ()        ; break ;
    case SL_SAMPLE_RESTART : reset ()        ; break ;
    default : break ;
  }

  return SL_TRUE ;
}

slPlayer::~slPlayer ()
{
}


void slPlayer::read ( int nframes, Uchar *dst, int next_env )
{
  /*
    WARNING:

       CO-RECURSIVE!
  */

  /* Find the next envelope */

  while ( next_env < SL_MAX_ENVELOPES && env [ next_env ] == NULL )
    next_env++ ;

  /*
    If there are no fancy envelopes to process then return
    the raw data.
  */

  if ( next_env >= SL_MAX_ENVELOPES ) /* No fancy envelopes left */
  {
    low_read ( nframes, dst ) ;
    return ;
  }

  /*
    Envelope processing required...

    Process the next envelope using data read recursively through
    the remaining envelopes.
  */

  switch ( env_type [ next_env ] )
  {
    /* For Volume envelopes, SRC and DST can be the same buffer */

    case SL_INVERSE_VOLUME_ENVELOPE:
      read ( nframes, dst, next_env+1 ) ;
      env[ next_env ]->applyToInvVolume ( dst,dst,nframes,env_start_time[ next_env ] ) ;
      break ;

    case SL_VOLUME_ENVELOPE :
      read ( nframes, dst, next_env+1 ) ;
      env[ next_env ]->applyToVolume ( dst,dst,nframes,env_start_time[ next_env ] ) ;
      break ;

    case SL_INVERSE_PITCH_ENVELOPE :
      env[ next_env ]->applyToInvPitch ( dst,this,nframes,env_start_time[ next_env ], next_env+1 ) ;
      break ;

    case SL_PITCH_ENVELOPE  :
      env[ next_env ]->applyToPitch ( dst,this,nframes,env_start_time[ next_env ], next_env+1 ) ;
      break ;

    case SL_INVERSE_FILTER_ENVELOPE:
    case SL_FILTER_ENVELOPE :
      read ( nframes, dst, next_env+1 ) ;
      break ;

    case SL_INVERSE_PAN_ENVELOPE   :
    case SL_PAN_ENVELOPE    :
      read ( nframes, dst, next_env+1 ) ;
      break ;

    case SL_INVERSE_ECHO_ENVELOPE  :
    case SL_ECHO_ENVELOPE   :
      read ( nframes, dst, next_env+1 ) ;
      break ;

    default :
      break ;
  }
}

