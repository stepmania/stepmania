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

int slSamplePlayer::preempt ( int delay )
{
  slScheduler::getCurrent() -> addCallBack ( callback, sample, SL_EVENT_PREEMPTED, magic ) ;

  return slPlayer::preempt ( delay ) ;
}

slSamplePlayer::~slSamplePlayer () 
{ 
  if ( sample ) 
    sample -> unRef () ; 

  /* unRef() attached envelopes */ 
  for ( int i = 0 ; i < SL_MAX_ENVELOPES ; i++ ) 
  { 
    if ( env [ i ] != NULL ) 
      env [ i ] -> unRef(); 
  } 

  slScheduler::getCurrent() -> addCallBack ( callback, sample, SL_EVENT_COMPLETE, magic ) ; 
} 

void slSamplePlayer::skip ( int nframes )
{
  if ( nframes < lengthRemaining )
  {
    lengthRemaining -= nframes ;
    bufferPos       += nframes ;
  }
  else 
  if ( replay_mode == SL_SAMPLE_LOOP )
  {
    slScheduler::getCurrent() -> addCallBack ( callback, sample, SL_EVENT_LOOPED, magic ) ;

    nframes -= lengthRemaining ;

    while ( nframes >= sample->getLength () )
      nframes -= sample->getLength () ;

    lengthRemaining = sample->getLength() - nframes ;
    bufferPos = & ( sample->getBuffer() [ nframes ] ) ;
  }
  else
    stop () ;
}


void slSamplePlayer::low_read ( int nframes, Uchar *dst )
{
  if ( isWaiting() ) start () ;

  if ( bufferPos == NULL )  /* Run out of sample & no repeats */
  {
    memset ( dst, 0x80, nframes ) ;
    return ;
  }

  while ( SL_TRUE )
  {
    /*
      If we can satisfy this request in one read (with data left in
      the sample buffer ready for next time around) - then we are done...
    */

    if ( nframes < lengthRemaining )
    {
      memcpy ( dst, bufferPos, nframes ) ;
      bufferPos       += nframes ;
      lengthRemaining -= nframes ;
      return ;
    }

    memcpy ( dst, bufferPos, lengthRemaining ) ;
    bufferPos       += lengthRemaining ;
    dst             += lengthRemaining ;
    nframes         -= lengthRemaining ;
    lengthRemaining  = 0 ;

    if ( replay_mode == SL_SAMPLE_ONE_SHOT )
    {
      stop () ;
      memset ( dst, 0x80, nframes ) ;
      return ;
    }
    else
    {
      slScheduler::getCurrent() -> addCallBack ( callback, sample, SL_EVENT_LOOPED, magic ) ;
      start () ;
    }
  }
}



