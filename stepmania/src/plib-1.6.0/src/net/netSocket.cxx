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

#include "netSocket.h"

#if defined(__CYGWIN__) || !defined (WIN32)

#if defined(__APPLE__)
#  include <netinet/in.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>    /* Need both for Mandrake 8.0!! */
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>

#else

#include <winsock.h>
#include <stdarg.h>

#endif

#if defined(_MSC_VER) && !defined(socklen_t)
#define socklen_t int
#endif

/* Paul Wiltsey says we need this for Solaris 2.8 */
 
#ifndef INADDR_NONE
#define INADDR_NONE ((unsigned long)-1)
#endif
                                                                                               
netAddress::netAddress ( cchar* host, int port )
{
  set ( host, port ) ;
}


void netAddress::set ( cchar* host, int port )
{
	memset(this, 0, sizeof(netAddress));

  sin_family = AF_INET ;
  sin_port = htons (port);

  /* Convert a string specifying a host name or one of a few symbolic
  ** names to a numeric IP address.  This usually calls gethostbyname()
  ** to do the work; the names "" and "<broadcast>" are special.
  */

	if (host[0] == '\0') {
		sin_addr = INADDR_ANY;
	}
	if (host[0] == '<' && strcmp(host, "<broadcast>") == 0) {
		sin_addr = INADDR_BROADCAST;
	}
  else
  {
#if 0
  	int d1, d2, d3, d4;
  	char ch;
  	if (sscanf(host, "%d.%d.%d.%d%c", &d1, &d2, &d3, &d4, &ch) == 4 &&
  	    0 <= d1 && d1 <= 255 && 0 <= d2 && d2 <= 255 &&
  	    0 <= d3 && d3 <= 255 && 0 <= d4 && d4 <= 255) {
  		sin_addr = htonl(
  			((long) d1 << 24) | ((long) d2 << 16) |
  			((long) d3 << 8) | ((long) d4 << 0));
  	}
    else  //let's try gethostbyname()
#else
    sin_addr = inet_addr(host);
    if ( sin_addr == INADDR_NONE )
#endif
    {
    	struct hostent *hp = gethostbyname(host);
    	if (hp != NULL)
      {
      	memcpy((char *) &sin_addr, hp->h_addr, hp->h_length);
      }
      else  //failure
      {
        sin_addr = INADDR_ANY;
      }
    }
  }
}


/* Create a string object representing an IP address.
   This is always a string of the form 'dd.dd.dd.dd' (with variable
   size numbers). */

cchar* netAddress::getHost () const
{
#if 0
  cchar* buf = inet_ntoa ( sin_addr ) ;
#else
  static char buf [32];
	long x = ntohl(sin_addr);
	sprintf(buf, "%d.%d.%d.%d",
		(int) (x>>24) & 0xff, (int) (x>>16) & 0xff,
		(int) (x>> 8) & 0xff, (int) (x>> 0) & 0xff );
#endif
  return buf;
}


int netAddress::getPort() const
{
  return ntohs(sin_port);
}

cchar* netAddress::getLocalHost ()
{
  //gethostbyname(gethostname())

  char buf[256];
  memset(buf, 0, sizeof(buf));
  gethostname(buf, sizeof(buf)-1);
  const hostent *hp = gethostbyname(buf);
	if (hp && *hp->h_addr_list)
  {
    in_addr addr = *((in_addr*)*hp->h_addr_list);
    cchar* host = inet_ntoa(addr);
    if ( host )
      return host ;
	}
  return "127.0.0.1" ;
}

bool netAddress::getBroadcast () const {
    return sin_addr == INADDR_BROADCAST;
}

netSocket::netSocket ()
{
  handle = -1 ;
}

netSocket::~netSocket ()
{
  close () ;
}

void netSocket::setHandle (int _handle)
{
  close () ;
  handle = _handle ;
}

bool
netSocket::open ( bool stream )
{
  close () ;
  handle = ::socket ( AF_INET, (stream? SOCK_STREAM: SOCK_DGRAM), 0 ) ;
  return (handle != -1);
}

void
netSocket::setBlocking ( bool blocking )
{
  assert ( handle != -1 ) ;

#if defined(__CYGWIN__) || !defined (WIN32)

	int delay_flag = ::fcntl (handle, F_GETFL, 0);
	if (blocking)
		delay_flag &= (~O_NDELAY);
	else
		delay_flag |= O_NDELAY;
  ::fcntl (handle, F_SETFL, delay_flag);

#else

  u_long nblocking = blocking? 0: 1;
  ::ioctlsocket(handle, FIONBIO, &nblocking);

#endif
}

void
netSocket::setBroadcast ( bool broadcast )
{
  assert ( handle != -1 ) ;
  int result;
  if ( broadcast ) {
      int one = 1;
#ifdef WIN32
      result = ::setsockopt( handle, SOL_SOCKET, SO_BROADCAST, (char*)&one, sizeof(one) );
#else
      result = ::setsockopt( handle, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one) );
#endif
  } else {
      result = ::setsockopt( handle, SOL_SOCKET, SO_BROADCAST, NULL, 0 );
  }
  if ( result < 0 ) {
      perror("set broadcast:");
  }
  assert ( result != -1 );
}

int
netSocket::bind ( cchar* host, int port )
{
  assert ( handle != -1 ) ;
  netAddress addr ( host, port ) ;
  return ::bind(handle,(const sockaddr*)&addr,sizeof(netAddress));
}

int
netSocket::listen ( int backlog )
{
  assert ( handle != -1 ) ;
  return ::listen(handle,backlog);
}

int
netSocket::accept ( netAddress* addr )
{
  assert ( handle != -1 ) ;
  socklen_t addr_len = (socklen_t) sizeof(netAddress) ;
  return ::accept(handle,(sockaddr*)addr,&addr_len);
}

int
netSocket::connect ( cchar* host, int port )
{
  assert ( handle != -1 ) ;
  netAddress addr ( host, port ) ;
  if ( addr.getBroadcast() ) {
      setBroadcast( true );
  }
  return ::connect(handle,(const sockaddr*)&addr,sizeof(netAddress));
}

int
netSocket::send (const void * buffer, int size, int flags)
{
  assert ( handle != -1 ) ;
  return ::send (handle, (cchar*)buffer, size, flags);
}

int
netSocket::sendto ( const void * buffer, int size, int flags, const netAddress* to )
{
  assert ( handle != -1 ) ;
  return ::sendto(handle,(cchar*)buffer,size,flags,(const sockaddr*)to,sizeof(netAddress));
}

int
netSocket::recv (void * buffer, int size, int flags)
{
  assert ( handle != -1 ) ;
  return ::recv (handle, (char*)buffer, size, flags);
}

int
netSocket::recvfrom ( void * buffer, int size, int flags, netAddress* from )
{
  assert ( handle != -1 ) ;
  socklen_t fromlen = (socklen_t) sizeof(netAddress) ;
  return ::recvfrom(handle,(char*)buffer,size,flags,(sockaddr*)from,&fromlen);
}

void
netSocket::close (void)
{
  if ( handle != -1 )
  {
#if defined(__CYGWIN__) || !defined (WIN32)
    ::close( handle );
#else
    ::closesocket( handle );
#endif
    handle = -1 ;
  }
}

bool
netSocket::isNonBlockingError ()
{
#if defined(__CYGWIN__) || !defined (WIN32)
  switch (errno) {
  case EWOULDBLOCK: // always == NET_EAGAIN?
  case EALREADY:
  case EINPROGRESS:
    return true;
  }
  return false;
#else
  int wsa_errno = WSAGetLastError();
  if ( wsa_errno != 0 )
  {
    WSASetLastError(0);
    ulSetError(UL_WARNING,"WSAGetLastError() => %d",wsa_errno);
    switch (wsa_errno) {
    case WSAEWOULDBLOCK: // always == NET_EAGAIN?
    case WSAEALREADY:
    case WSAEINPROGRESS:
      return true;
    }
  }
  return false;
#endif
}

int
netSocket::select ( netSocket** reads, netSocket** writes, int timeout )
{
  fd_set r,w;
  
  FD_ZERO (&r);
  FD_ZERO (&w);

  int i, k ;
  int num = 0 ;

  for ( i=0; reads[i]; i++ )
  {
    int fd = reads[i]->getHandle();
    FD_SET (fd, &r);
    num++;
  }

  for ( i=0; writes[i]; i++ )
  {
    int fd = writes[i]->getHandle();
    FD_SET (fd, &w);
    num++;
  }

  if (!num)
    return num ;

  /* Set up the timeout */
  struct timeval tv ;
  tv.tv_sec = timeout/1000;
  tv.tv_usec = (timeout%1000)*1000;

  // It bothers me that select()'s first argument does not appear to
  // work as advertised... [it hangs like this if called with
  // anything less than FD_SETSIZE, which seems wasteful?]
  
  // Note: we ignore the 'exception' fd_set - I have never had a
  // need to use it.  The name is somewhat misleading - the only
  // thing I have ever seen it used for is to detect urgent data -
  // which is an unportable feature anyway.

  ::select (FD_SETSIZE, &r, &w, 0, &tv);

  //remove sockets that had no activity

  num = 0 ;

  for ( k=i=0; reads[i]; i++ )
  {
    int fd = reads[i]->getHandle();
    if (FD_ISSET (fd, &r)) {
      reads[k++] = reads[i];
      num++;
    }
  }
  reads[k] = NULL ;

  for ( k=i=0; writes[i]; i++ )
  {
    int fd = writes[i]->getHandle();
    if (FD_ISSET (fd, &w)) {
      writes[k++] = writes[i];
      num++;
    }
  }
  writes[k] = NULL ;

  return num ;
}

/* Init/Exit functions */
static void netExit ( void )
{
#if defined(__CYGWIN__) || !defined (WIN32)
#else
	/* Clean up windows networking */
	if ( WSACleanup() == SOCKET_ERROR ) {
		if ( WSAGetLastError() == WSAEINPROGRESS ) {
			WSACancelBlockingCall();
			WSACleanup();
		}
	}
#endif
}

int netInit ( int* argc, char** argv )
{
  assert ( sizeof(sockaddr_in) == sizeof(netAddress) ) ;

#if defined(__CYGWIN__) || !defined (WIN32)
#else
	/* Start up the windows networking */
	WORD version_wanted = MAKEWORD(1,1);
	WSADATA wsaData;

	if ( WSAStartup(version_wanted, &wsaData) != 0 ) {
		ulSetError(UL_WARNING,"Couldn't initialize Winsock 1.1");
		return(-1);
	}
#endif

  atexit( netExit ) ;
	return(0);
}

cchar* netFormat ( cchar* format, ... )
{
  static char buffer[ 256 ];
  va_list argptr;
  va_start(argptr, format);
  vsprintf( buffer, format, argptr );
  va_end(argptr);
  return( buffer );
}
