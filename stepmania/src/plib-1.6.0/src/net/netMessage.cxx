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

#include "netMessage.h"


void
netMessageChannel::handleBufferRead (netBuffer& in_buffer)
{
  int n = in_buffer.getLength () ;
  while ( n >= 2 )
  {
    u16 msg_len = *( (u16*)in_buffer.getData() ) ;
    if ( n >= msg_len )
    {
      //we have a complete message; handle it
      netMessage msg(in_buffer.getData(),msg_len);
      in_buffer.remove(0,msg_len);
      handleMessage ( msg );

      //ulSetError ( UL_DEBUG, "netMessageChannel: %d read", msg_len ) ;
      n -= msg_len ;
    }
    else
    {
      //ulSetError ( UL_DEBUG, "netMessageChannel: %d waiting", n ) ;
      break ;
    }
  }
}
