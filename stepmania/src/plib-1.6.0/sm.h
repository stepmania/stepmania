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


#ifndef __SM_H__
#define __SM_H__ 1

#include "slPortability.h"

#ifdef SL_USING_OSS_AUDIO
#define SMMIXER_DEFAULT_DEVICE "/dev/mixer"
#elif defined(WIN32)
#define SMMIXER_DEFAULT_DEVICE "mixer"
#else
#endif


# define SM_TRUE  1
# define SM_FALSE 0

typedef unsigned char  Uchar  ;
typedef unsigned short Ushort ;


class smMixer
{
private:

  int devices ;
  int error ;
  int fd ;

#ifdef SL_USING_OSS_AUDIO

  int ioctl ( int cmd, int param = 0 )
  {
    if ( error ) return param ;

    if ( ::ioctl ( fd, cmd, & param ) == -1 )
    {
      perror ( "smMixer: ioctl" ) ;
      error = SM_TRUE ;
    }

    return param ;
  }
#endif
  void open ( const char *device ) ;
  void close () ;

public:

  /* Tom */

  smMixer ();
  smMixer ( const char *device );
 ~smMixer ();
  
  int notWorking () const ;
  int not_working () const { return notWorking () ; }  /* Originally a typo! */

  /* Volume controls are in integer percentages */

  int  getVolume        ( int channel             );
  void setVolume        ( int channel, int volume ); 

  void getVolume        ( int channel, int *left, int *right );
  void setVolume        ( int channel, int  left, int  right );

  void setTreble       ( int treble );
  void setBass         ( int bass   );

  void setMasterVolume ( int volume );
  void setSynthVolume  ( int volume );
  void setPCMVolume    ( int volume );
  void setSpeakerVolume( int volume );
  void setLineVolume   ( int volume );
  void setMicVolume    ( int volume );
  void setCDVolume     ( int volume );

  void setMasterVolume ( int left, int right );
  void setSynthVolume  ( int left, int right );
  void setPCMVolume    ( int left, int right );
  void setSpeakerVolume( int left, int right );
  void setLineVolume   ( int left, int right );
  void setMicVolume    ( int left, int right );
  void setCDVolume     ( int left, int right );
} ;

#endif

