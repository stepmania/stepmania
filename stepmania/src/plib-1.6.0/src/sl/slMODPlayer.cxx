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
#include "slMODPrivate.h"
#include "slMODfile.h"

int slMODPlayer::preempt ( int delay )
{
  return slPlayer::preempt ( delay ) ;
}

void slMODPlayer::init ( const char *fname )
{
  mf = new MODfile ( fname, slScheduler::getCurrent()->getRate (), SL_FALSE ) ;
}

slMODPlayer::~slMODPlayer ()
{
  delete mf ;
}

void slMODPlayer::skip ( int /* nframes */ )
{
}


void slMODPlayer::low_read ( int nframes, Uchar *dst )
{
  if ( isWaiting() ) start () ;

  int need_bytes = nframes ;
  int all_done = 0 ;

  while ( need_bytes > 0 && !all_done )
  {
    int new_bytes = dacioGetLen () ;

    /* Compute some more audio */

    while ( new_bytes == 0 && !all_done )
    {
      all_done = ! mf -> update () ;
      new_bytes = dacioGetLen () ;
    }

    /* How much did we get? */

    if ( new_bytes > need_bytes )   /* oops! Too much */
    {
      memcpy ( dst, dacioGetOutBuffer (), need_bytes ) ;
      dacioSubtract ( need_bytes ) ;
      dst += need_bytes ;
      need_bytes = 0 ;
    }
    else
    {
      memcpy ( dst, dacioGetOutBuffer (), new_bytes ) ;
      dacioEmpty () ;
      dst += new_bytes ;
      need_bytes -= new_bytes ;
    }
  }
  
  /* Pad with silence if not enough data */

  if ( need_bytes > 0 )
    memset ( dst, 128, need_bytes ) ;

  if ( all_done )
  {
    if ( replay_mode == SL_SAMPLE_ONE_SHOT )
      stop () ;
    else
      start () ;
  }
}



