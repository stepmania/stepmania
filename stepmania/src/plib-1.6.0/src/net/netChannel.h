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

/****
* NAME
*   netChannel - network channel class
*
* DESCRIPTION
*   netChannel is adds event-handling to the low-level
*   netSocket class.  Otherwise, it can be treated as
*   a normal non-blocking socket object.
*
*   The direct interface between the netPoll() loop and
*   the channel object are the handleReadEvent and
*   handleWriteEvent methods. These are called
*   whenever a channel object 'fires' that event.
*
*   The firing of these low-level events can tell us whether
*   certain higher-level events have taken place, depending on
*   the timing and state of the connection.
*
* AUTHORS
*   Sam Rushing <rushing@nightmare.com> - original version for Medusa
*   Dave McClurg <dpm@efn.org> - modified for use in PLIB
*
* CREATION DATE
*   Dec-2000
*
****/

#ifndef NET_CHANNEL_H
#define NET_CHANNEL_H

#include "netSocket.h"

class netChannel : public netSocket
{
  bool closed, connected, accepting, write_blocked, should_delete ;
  netChannel* next_channel ;
  
  friend bool netPoll (u32 timeout);

public:

  netChannel () ;
  virtual ~netChannel () ;

  void setHandle (int s, bool is_connected = true);
  bool isConnected () const { return connected; }
  bool isClosed () const { return closed; }
  void shouldDelete () { should_delete = true ; }

  // --------------------------------------------------
  // socket methods
  // --------------------------------------------------
  
  bool  open    ( void ) ;
  void  close   ( void ) ;
  int   listen  ( int backlog ) ;
  int   connect ( cchar* host, int port ) ;
  int   send    ( const void * buf, int size, int flags = 0 ) ;
  int   recv    ( void * buf, int size, int flags = 0 ) ;

  // poll() eligibility predicates
  virtual bool readable (void) { return (connected || accepting); }
  virtual bool writable (void) { return (!connected || write_blocked); }
  
  // --------------------------------------------------
  // event handlers
  // --------------------------------------------------
  
  void handleReadEvent (void);
  void handleWriteEvent (void);
  
  // These are meant to be overridden.
  virtual void handleClose (void) {
    //ulSetError(UL_WARNING,"Network: %d: unhandled close",getHandle());
  }
  virtual void handleRead (void) {
    ulSetError(UL_WARNING,"Network: %d: unhandled read",getHandle());
  }
  virtual void handleWrite (void) {
    ulSetError(UL_WARNING,"Network: %d: unhandled write",getHandle());
  }
  virtual void handleAccept (void) {
    ulSetError(UL_WARNING,"Network: %d: unhandled accept",getHandle());
  }
  virtual void handleError (int error) {
    ulSetError(UL_WARNING,"Network: %d: errno: %s(%d)",getHandle(),strerror(errno),errno);
  }

  static bool poll (u32 timeout = 0 ) ;
  static void loop (u32 timeout = 0 ) ;
};

#endif // NET_CHANNEL_H
