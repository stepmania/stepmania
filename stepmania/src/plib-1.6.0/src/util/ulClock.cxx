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


#include <stdio.h>
#include <stdlib.h>
#if defined(WIN32)
#include <windows.h>
#include <mmsystem.h>
 #ifdef __CYGWIN__
  typedef long long _int64;
  #define LARGEINTEGER _int64
//  #include <largeint.h>
 #endif
#else
#include <sys/time.h>
#endif
#include <time.h>

#include "ul.h"

#if defined(WIN32)
double ulClock::res ;
int ulClock::perf_timer = -1;

void ulClock::initPerformanceTimer ()
{
	if( perf_timer == -1 ) {
		/* Use Performance Timer if it's available, mmtimer if not.  */

		__int64 frequency ;

		perf_timer = QueryPerformanceFrequency ( (LARGE_INTEGER *) & frequency ) ;

		if ( perf_timer )
		{
			res = 1.0 / (double) frequency ;
			perf_timer = 1 ;
		}
	}
}
#endif

double ulClock::getRawTime () const
{
#if defined(WIN32)

  /* Use Performance Timer if it's available, mmtimer if not.  */

  if ( perf_timer )
  {
    __int64 t ;
 
    QueryPerformanceCounter ( (LARGE_INTEGER *) &t ) ;
 
    return res * (double) t ;
  }
 
  return (double) timeGetTime() * 0.001 ;

#else
  timeval tv ;

  gettimeofday ( & tv, NULL ) ;

  return (double) tv.tv_sec + (double) tv.tv_usec / 1000000.0 ;
#endif
}


void ulClock::update ()
{
  now = getRawTime() - start ;

  delta = now - last_time ;

  /*
    KLUDGE: If the frame rate drops below ~5Hz, then
            control will be very difficult.  It's
            actually easier to give up and slow
            down the action. max_delta defaults to
            200ms for that reason.

    KLUDGE: If update is called very rapidly, then
            delta can be zero which causes some
            programs to div0. So we'll clamp to
            a millionth of a second.
  */

  if ( delta >  max_delta ) delta = max_delta ;
  if ( delta <= 0.0 ) delta = 0.0000001 ;

  last_time = now ;
}



