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
*   netMessage - message buffer and channel classes
*
* DESCRIPTION
*   messages are a binary format for sending buffers over a channel.
*   message headers contain a type field and length.
*
* AUTHOR
*   Dave McClurg <dpm@efn.org>
*
* CREATION DATE
*   Dec-2000
****/

#ifndef __NET_MESSAGE__
#define __NET_MESSAGE__


#include "netBuffer.h"

#if defined(__CYGWIN__) || !defined (WIN32)
#include <netinet/in.h> // ntohs() etc prototypes
#endif


class netGuid //Globally Unique IDentifier
{
public:
  u8 data [ 16 ] ;

  netGuid () {}

  netGuid ( u32 l, u16 w1, u16 w2,
    u8 b1, u8 b2, u8 b3, u8 b4, u8 b5, u8 b6, u8 b7, u8 b8 )
  {
    //store in network format (big-endian)
    data[0] = u8(l>>24);
    data[1] = u8(l>>16);
    data[2] = u8(l>>8);
    data[3] = u8(l);
    data[4] = u8(w1>>8);
    data[5] = u8(w1);
    data[6] = u8(w2>>8);
    data[7] = u8(w2);
    data[8] = b1;
    data[9] = b2;
    data[10] = b3;
    data[11] = b4;
    data[12] = b5;
    data[13] = b6;
    data[14] = b7;
    data[15] = b8;
  }

  bool operator ==( const netGuid& guid ) const
  { 
    return memcmp ( data, guid.data, sizeof(data) ) == 0 ;
  }
  bool operator !=( const netGuid& guid ) const
  { 
    return memcmp ( data, guid.data, sizeof(data) ) != 0 ;
  }
} ;


class netMessage : public netBuffer
{
  int pos ;

  void seek ( int new_pos ) const
  {
    if ( new_pos < 0 )
      new_pos = 0 ;
    else if ( new_pos > length )
      new_pos = length ;

    //logical const-ness
    ((netMessage*)this) -> pos = new_pos ;
  }
  void skip ( int off ) const
  {
    seek(pos+off);
  }

public:

  // incoming message; header is already there
  netMessage ( const char* s, int n ) : netBuffer(n)
  {
    assert ( n >= 5 ) ;
    append(s,n);
    pos = 5 ; // seek past header
  }

  // outgoing message
  netMessage ( int type, int to_id, int from_id=0, int n=256 ) : netBuffer(n)
  {
    // output header
    putw ( 0 ) ;  //msg_len
    putbyte ( type ) ;
    putbyte ( to_id ) ;
    putbyte ( from_id ) ;
  }

  int getType () const { return ( (u8*)data )[ 2 ] ; }
  int getToID () const { return ( (u8*)data )[ 3 ] ; }
  int getFromID () const { return ( (u8*)data )[ 4 ] ; }
  void setFromID ( int from_id ) { ( (u8*)data )[ 4 ] = (u8)from_id; }

  void geta ( void* a, int n ) const
  {
    assert (pos>=0 && pos<length && (pos+n)<=length) ;
    //if (pos>=0 && pos<length && (pos+n)<=length)
    {
      memcpy(a,&data[pos],n) ;
      seek(pos+n);
    }
  }
  void puta ( const void* a, int n )
  {
    append((const char*)a,n);
    pos = length;
    *((u16*)data) = u16(length); //update msg_len
  }

  int getbyte () const
  {
    u8 temp ;
    geta(&temp,sizeof(temp)) ;
    return temp ;
  }
  void putbyte ( int c )
  {
    u8 temp = c ;
    puta(&temp,sizeof(temp)) ;
  }

  bool getb () const
  {
    u8 temp ;
    geta(&temp,sizeof(temp)) ;
    return temp != 0 ;
  }
  void putb ( bool b )
  {
    u8 temp = b? 1: 0 ;
    puta(&temp,sizeof(temp)) ;
  }

  int getw () const
  {
    u16 temp ;
    geta ( &temp, sizeof(temp) ) ;
    return int ( ntohs ( temp ) ) ;
  }
  void putw ( int i )
  {
    u16 temp = htons ( u16(i) ) ;
    puta ( &temp, sizeof(temp) ) ;
  }

  int geti () const
  {
    u32 temp ;
    geta ( &temp, sizeof(temp) ) ;
    return int ( ntohl ( temp ) ) ;
  }
  void puti ( int i )
  {
    u32 temp = htonl ( u32(i) ) ;
    puta ( &temp, sizeof(temp) ) ;
  }

  void getfv ( f32* fv, int n ) const
  {
    u32* v = (u32*)fv;
    geta ( v, (n<<2) ) ;
    for ( int i=0; i<n; i++ )
      v[i] = ntohl ( v[i] ) ;
  }
  void putfv ( const f32* fv, int n )
  {
    const u32* v = (const u32*)fv;
    for ( int i=0; i<n; i++ )
    {
      u32 temp = htonl ( v[i] ) ;
      puta ( &temp, sizeof(temp) ) ;
    }
  }
  
  f32 getf () const
  {
    u32 temp ;
    geta ( &temp, sizeof(temp) ) ;
    temp = ntohl ( temp ) ;
    return *((f32*)&temp) ;
  }
  void putf ( f32 f )
  {
    u32 temp = *((u32*)&f) ;
    temp = htonl ( temp ) ;
    puta ( &temp, sizeof(temp) ) ;
  }

  void gets ( char* s, int n ) const
  {
    char* src = &data[pos];
    char* dst = s;
    while (pos<length)
    {
      char ch = *src++;
      if ((dst-s)<(n-1))
        *dst++ = ch ;
      ((netMessage*)this)->pos++;
      if (ch==0)
        break;
    }
    *dst = 0 ;
  }
  void puts ( const char* s )
  {
    puta(s,strlen(s)+1);
  }

  void print ( FILE *fd = stderr ) const
  {
    fprintf ( fd, "netMessage: %p, length=%d\n", this, length ) ;
    fprintf ( fd, "  header (type,to,from) = (%d,%d,%d)\n",
      getType(), getToID(), getFromID() ) ;
    fprintf ( fd, "  data = " ) ;
    for ( int i=0; i<length; i++ )
      fprintf ( fd, "%02x ", data[i] ) ;
    fprintf ( fd, "\n" ) ;
  }
};


class netMessageChannel : public netBufferChannel
{
  virtual void handleBufferRead (netBuffer& buffer) ;

public:

  bool sendMessage ( const netMessage& msg )
  {
    return bufferSend ( msg.getData(), msg.getLength() ) ;
  }

  virtual void handleMessage ( const netMessage& msg ) {}
};


#endif //__NET_MESSAGE__
