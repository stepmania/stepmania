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
*   netSocket - network sockets
*
* DESCRIPTION
*   netSocket is a thin C++ wrapper over bsd sockets to
*   facilitate porting to other platforms
*
* AUTHOR
*   Dave McClurg <dpm@efn.org>
*
* CREATION DATE
*   Dec-2000
*
****/

#ifndef NET_SOCKET_H
#define NET_SOCKET_H


#include "ul.h"
#include <errno.h>


/*
 * Define Basic types
 */

typedef float f32;
typedef double f64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef const char cchar;
typedef const void cvoid;


/*
 * Socket address, internet style.
 */
class netAddress
{
  s16     sin_family;
  u16     sin_port;
  u32     sin_addr;
  char    sin_zero[8];

public:
  netAddress () {}
  netAddress ( cchar* host, int port ) ;

  void set ( cchar* host, int port ) ;
  cchar* getHost () const ;
  int getPort() const ;

  static cchar* getLocalHost () ;

  bool getBroadcast () const ;
};


/*
 * Socket type
 */
class netSocket
{
  int handle ;

public:

  netSocket () ;
  virtual ~netSocket () ;

  int getHandle () const { return handle; }
  void setHandle (int handle) ;
  
  bool  open        ( bool stream=true ) ;
  void  close		    ( void ) ;
  int   bind        ( cchar* host, int port ) ;
  int   listen	    ( int backlog ) ;
  int   accept      ( netAddress* addr ) ;
  int   connect     ( cchar* host, int port ) ;
  int   send		    ( const void * buffer, int size, int flags = 0 ) ;
  int   sendto      ( const void * buffer, int size, int flags, const netAddress* to ) ;
  int   recv		    ( void * buffer, int size, int flags = 0 ) ;
  int   recvfrom    ( void * buffer, int size, int flags, netAddress* from ) ;

  void setBlocking ( bool blocking ) ;
  void setBroadcast ( bool broadcast ) ;

  static bool isNonBlockingError () ;
  static int select ( netSocket** reads, netSocket** writes, int timeout ) ;
} ;


int netInit ( int* argc = NULL, char** argv = NULL ) ;
cchar* netFormat ( cchar* fmt, ... ) ;


#endif // NET_SOCKET_H
