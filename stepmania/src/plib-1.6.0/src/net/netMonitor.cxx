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

#include "netMonitor.h"


class netMonitorChannel : public netChat
{
  netMonitorServer* server ;
  bool authorized ;
  netBuffer buffer;

  void prompt () ;

  virtual void collectIncomingData	(const char* s, int n) ;
  virtual void foundTerminator (void) ;

public:

  netMonitorChannel ( netMonitorServer* server ) ;

  virtual void handleClose (void)
  {
    ulSetError(UL_DEBUG, "%d: Client disconnected.",getHandle());
    shouldDelete () ;
    netChat::handleClose () ;
    server -> active = 0 ;
  }
} ;

		
// for now, we ignore any telnet option stuff sent to
// us, and we process the backspace key ourselves.
// gee, it would be fun to write a full-blown line-editing
// environment, etc...

static void clean_line (char* line)
{
  char* dst = line ;
  for ( char* src = line ; *src ; src ++ )
  {
    char ch = *src ;
    if (ch==8 || ch==127)
    {
      // backspace
      if (dst != line)
        dst -- ;
    }
    else if (ch<127)
    {
      *dst++ = *src ;
    }
  }
  *dst = 0 ;
}


netMonitorChannel::netMonitorChannel ( netMonitorServer* _server ) : buffer(512)
{
  server = _server ;
  setTerminator("\r\n");
  
  if ( server -> password && server -> password [0] != 0 )
  {
    authorized = false ;
    push ("Enter password: ") ;
  }
  else
  {
    authorized = true ;
    push ( netFormat("Connected to \"%s\"... Welcome!\r\n", server -> name ) ) ;
    prompt();
  }
}


void netMonitorChannel::prompt ()
{
	push ( server -> prompt ) ;
}


void netMonitorChannel::collectIncomingData	(const char* s, int n)
{
  if ( !buffer.append(s,n) )
  {
    // denial of service.
    push ("BCNU\r\n");
    closeWhenDone();
  }
}

void netMonitorChannel::foundTerminator (void)
{
  char* line = buffer.getData();
  clean_line ( line ) ;
  
  if (!authorized)
  {
    if (strcmp(line,server -> password) == 0)
    {
      authorized = true ;
      push ( netFormat("Connected to \"%s\"... Welcome!\r\n",server -> name) ) ;
      prompt () ;
    }
    else
    {
      close();
    }
  }
  else if (*line == 0)
  {
    prompt();
  }
  else if (*line == 4 || strcmp(line,"exit") == 0)
  {
    push ("BCNU\r\n");  //Be seein' you
    closeWhenDone();
  }
  else
  {
    if ( server -> cmdfunc )
    {
      server -> cmdfunc ( line ) ;
    }
    else
    {
      ulSetError(UL_DEBUG,"echo: %s",line);

      push(line);
      push(getTerminator());
    }
    
    prompt();
  }
  buffer.remove();
}


void netMonitorServer::handleAccept (void)
{
  if ( !active )
  {
    netAddress addr ;
    int s = accept ( &addr ) ;

    ulSetError(UL_DEBUG, "%d: Client %s:%d connected",s,addr.getHost(),addr.getPort());

    active = new netMonitorChannel ( this ) ;
    active -> setHandle (s);
  }
}


bool netMonitorServer::push (const char* s)
{
  if ( active )
    return active -> push ( s ) ;
  return false ;
}
