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


#ifndef __SLPORTABILITY_H__
#define __SLPORTABILITY_H__ 1

/* ------------------------------------------------------------- */
/* OS specific includes and defines ...                          */
/* ------------------------------------------------------------- */

#include "ul.h"
#include <stdio.h>
#include <stdlib.h>

/* the next lines are to define BSD */
/* see http://www.freebsd.org/handbook/porting.html for why we do this */
#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

#ifdef macintosh
  #include <Sound.h>
  #include <Timer.h>
  #ifdef __MWERKS__
  #include <unix.h>
  #endif
#endif

#ifdef __APPLE__
  #include <Carbon/Carbon.h>
#endif

#ifndef  WIN32
#include <unistd.h>
#include <sys/ioctl.h>
#else
#include <windows.h>
#ifdef __CYGWIN32__
#  define NEAR /* */
#  define FAR  /* */
#  define WHERE_EVER_YOU_ARE /* Curt: optional, but it reminds me of a song */
#endif
#include <mmsystem.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#if (defined(__linux__) || defined(BSD)) && !defined(__NetBSD__)
#define SL_USING_OSS_AUDIO 1
#endif

#ifdef SL_USING_OSS_AUDIO
#  if defined(__linux__)
#    include <linux/soundcard.h>
#  elif defined(__FreeBSD__)
#    include <machine/soundcard.h>
#  else
    /*
      Tom thinks this file may be <sys/soundcard.h> under some
      unixen - but that isn't where the OSS manuals say it
      should be.

      If you ever find out the truth, please email me:
       Steve Baker <sjbaker1@airmail.net>
    */
#    include <soundcard.h>
#  endif
#endif

#if defined (__NetBSD__) || defined(__OpenBSD__)
#  include <sys/audioio.h>
#endif

/* Tom */

#ifdef	sgi
#  include <audio.h>
#endif

#if defined(__svr4__) || defined(__SVR4)
#  include <sys/audioio.h>
#  include <sys/stropts.h>
#  define SOLARIS
#endif

#endif

