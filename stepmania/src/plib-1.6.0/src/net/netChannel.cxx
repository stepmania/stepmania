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

// TODO:
// have all socket-related functions assert that the socket has not
// been closed.  [a read event may close it, and a write event may try
// to write or something...]
// Maybe assert valid handle, too?

#include "netChannel.h"

static netChannel* channels = 0 ;

netChannel::netChannel ()
{
  closed = true ;
  connected = false ;
  accepting = false ;
  write_blocked = false ;
  should_delete = false ;

  next_channel = channels ;
  channels = this ;
}
  
netChannel::~netChannel ()
{
  close();

  netChannel* prev = NULL ;
  for ( netChannel* ch = channels; ch != NULL;
    ch = ch -> next_channel )
  {
    if (ch == this)
    {
      ch = ch -> next_channel ;
      if ( prev != NULL )
        prev -> next_channel = ch ;
      else
        channels = ch ;
      next_channel = 0 ;
      break;
    }
    prev = ch ;
  }
}
  
void
netChannel::setHandle (int handle, bool is_connected)
{
  close () ;
  netSocket::setHandle ( handle ) ;
  connected = is_connected ;
  //if ( connected ) this->handleConnect();
  closed = false ;
}

bool
netChannel::open (void)
{
  close();
  if (netSocket::open(true)) {
    closed = false ;
    setBlocking ( false ) ;
    return true ;
  }
  return false ;
}

int
netChannel::listen ( int backlog )
{
  accepting = true ;
  return netSocket::listen ( backlog ) ;
}

int
netChannel::connect ( cchar* host, int port )
{
  int result = netSocket::connect ( host, port ) ;
  if (result == 0) {
    connected = true ;
    //this->handleConnect();
    return 0;
  } else if (isNonBlockingError ()) {
    return 0;
  } else {
    // some other error condition
    this->handleError (result);
    close();
    return -1;
  }
}

int
netChannel::send (const void * buffer, int size, int flags)
{
  int result = netSocket::send (buffer, size, flags);
  
  if (result == (int)size) {
    // everything was sent
    write_blocked = false ;
    return result;
  } else if (result >= 0) {
    // not all of it was sent, but no error
    write_blocked = true ;
    return result;
  } else if (isNonBlockingError ()) {
    write_blocked = true ;
    return 0;
  } else {
    this->handleError (result);
    close();
    return -1;
  }
  
}

int
netChannel::recv (void * buffer, int size, int flags)
{
  int result = netSocket::recv (buffer, size, flags);
  
  if (result > 0) {
    return result;
  } else if (result == 0) {
    close();
    return 0;
  } else if (isNonBlockingError ()) {
    return 0;
  } else {
    this->handleError (result);
    close();
    return -1;
  }
}

void
netChannel::close (void)
{
  if ( !closed )
  {
    this->handleClose();
  
    closed = true ;
    connected = false ;
    accepting = false ;
    write_blocked = false ;
  }

  netSocket::close () ;
}

void
netChannel::handleReadEvent (void)
{
  if (accepting) {
    if (!connected) {
      connected = true ;
      //this->handleConnect();
    }
    this->handleAccept();
  } else if (!connected) {
    connected = true ;
    //this->handleConnect();
    this->handleRead();
  } else {
    this->handleRead();
  }
}

void
netChannel::handleWriteEvent (void)
{
  if (!connected) {
    connected = true ;
    //this->handleConnect();
  }
  write_blocked = false ;
  this->handleWrite();
}

bool
netChannel::poll (u32 timeout)
{
  if (!channels)
    return false ;
  
  enum { MAX_SOCKETS = 256 } ;
  netSocket* reads [ MAX_SOCKETS+1 ] ;
  netSocket* writes [ MAX_SOCKETS+1 ] ;
  netSocket* deletes [ MAX_SOCKETS+1 ] ;
  int nreads = 0 ;
  int nwrites = 0 ;
  int ndeletes = 0 ;
  int nopen = 0 ;
  netChannel* ch;
  for (  ch = channels; ch != NULL; ch = ch -> next_channel )
  {
    if ( ch -> should_delete )
    {
      assert(ndeletes<MAX_SOCKETS);
      deletes[ndeletes++] = ch ;
    }
    else if ( ! ch -> closed )
    {
      nopen++ ;
      if (ch -> readable()) {
        assert(nreads<MAX_SOCKETS);
        reads[nreads++] = ch ;
      }
      if (ch -> writable()) {
        assert(nwrites<MAX_SOCKETS);
        writes[nwrites++] = ch ;
      }
    }
  }
  reads[nreads] = NULL ;
  writes[nwrites] = NULL ;
  deletes[ndeletes] = NULL ;

  int i ;
  for ( i=0; deletes[i]; i++ )
  {
    ch = (netChannel*)deletes[i];
    delete ch ;
  }

  if (!nopen)
    return false ;
  if (!nreads && !nwrites)
    return true ; //hmmm- should we shutdown?

  netSocket::select (reads, writes, timeout) ;

  for ( i=0; reads[i]; i++ )
  {
    ch = (netChannel*)reads[i];
    if ( ! ch -> closed )
      ch -> handleReadEvent();
  }

  for ( i=0; writes[i]; i++ )
  {
    ch = (netChannel*)writes[i];
    if ( ! ch -> closed )
      ch -> handleWriteEvent();
  }

  return true ;
}

void
netChannel::loop (u32 timeout)
{
  while ( poll (timeout) ) ;
}
