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
*   netChat - network chat class
*
* DESCRIPTION
*   This class adds support for 'chat' style protocols -
*   where one side sends a 'command', and the other sends
*   a response (examples would be the common internet
*   protocols - smtp, nntp, ftp, etc..).
*
*   The handle_buffer_read() method looks at the input
*   stream for the current 'terminator' (usually '\r\n'
*   for single-line responses, '\r\n.\r\n' for multi-line
*   output), calling found_terminator() on its receipt.
*
* EXAMPLE
*   Say you build an nntp client using this class.
*   At the start of the connection, you'll have
*   terminator set to '\r\n', in order to process
*   the single-line greeting.  Just before issuing a
*   'LIST' command you'll set it to '\r\n.\r\n'.
*   The output of the LIST command will be accumulated
*   (using your own 'collect_incoming_data' method)
*   up to the terminator, and then control will be
*   returned to you - by calling your found_terminator()
*
* AUTHORS
*   Sam Rushing <rushing@nightmare.com> - original version for Medusa
*   Dave McClurg <dpm@efn.org> - modified for use in PLIB
*
* CREATION DATE
*   Dec-2000
*
****/

#ifndef NET_CHAT_H
#define NET_CHAT_H

#include "netBuffer.h"

class netChat : public netBufferChannel
{
  char* terminator;
  
  virtual void handleBufferRead (netBuffer& buffer) ;

public:

  netChat () : terminator (0) {}

  void setTerminator (const char* t);
  const char* getTerminator (void);

  bool push (const char* s)
  {
    return bufferSend ( s, strlen(s) ) ;
  }

  virtual void collectIncomingData	(const char* s, int n) {}
  virtual void foundTerminator (void) {}
};

#endif // NET_CHAT_H
