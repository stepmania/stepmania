/*******************************************
  ezsockets.cpp -- Header for sockets.cpp
   Designed by Josh Allen and Charles
   Lohr. Socket programming methods based
   on Charles Lohr's EZW progam.

  Modified by Charles Lohr for use with 
   Windows-Based OSes.
********************************************/

// StepMania only includes
#include "global.h"

#include "ezsockets.h"

// We need the WinSock32 Library on Windows
#if defined(_XBOX)
#elif defined(_WINDOWS)
#pragma comment(lib,"wsock32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

EzSockets::EzSockets()
{
	MAXCON = 5;
	memset ( &addr, 0, sizeof(addr) ); //Clear the sockaddr_in structure

	// Windows REQUIRES WinSock Startup
#if defined(_WINDOWS) || defined(_XBOX)
	WSAStartup( MAKEWORD(1,1), &wsda );
#endif

	sock = -1;
	blocking=true;


	scks = new fd_set;
	times = new timeval;

	
	times->tv_sec = 0;
	times->tv_usec = 0;
	
	state = skDISCONNECTED;
}

EzSockets::~EzSockets()
{
	close();
	delete scks;
	delete times;
}

bool EzSockets::check()
{
	//Check to see if the socket has been created
	if ( sock < 0 )
		return false;
	return true;
}

int EzSockets::create()
{
	state = skDISCONNECTED;
	sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	return (true);
}

bool EzSockets::bind( unsigned short port )
{
	if ( !check() )
		return false;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl( INADDR_ANY );
	addr.sin_port = htons( port );

	int desc = ::bind( sock, (struct sockaddr *)&addr, sizeof(addr) );
	if ( desc < 0 )
		return false;
	return true;
}

bool EzSockets::listen()
{
	int desc = ::listen( sock, MAXCON );

	if ( desc < 0 )
		return false;

	state = skLISTENING;
	return true;
}

// glibc already has socklen_t defined
// to whoever edited this:
// please do not assume LINUX is defined if you're compiling on linux!
// it will not compile properly.
#if defined(WIN32) || defined(DARWIN)
	typedef int socklen_t;
#endif

bool EzSockets::accept( EzSockets &socket )
{
	// Windows wants it defined as a signed int
    // As does everything else -- Steve
	int length = sizeof( socket );

	socket.sock = ::accept( sock, (struct sockaddr *)&socket.addr,(socklen_t *)&length );
	socket.state = skCONNECTED;
	if ( socket.sock < 0 )
		return false;
	
	return true;
}

void EzSockets::close() {
	state = skDISCONNECTED;
	inBuffer = "";
	outBuffer = "";
	// The close socket command is different in Windows
#if defined(WIN32)
	::closesocket( sock );
#else
	::close( sock );
#endif
}

long EzSockets::uAddr()
{
	return addr.sin_addr.s_addr;
}


bool EzSockets::connect( const std::string& host, unsigned short port )
{
	if (! check() )
		return false;

#if defined(_XBOX)
	// FIXME: Xbox doesn't have gethostbyname or any way to get a hostent.  
	// Investigate the samples and figure out how this is supposed to work.
	return false;
#elif defined(_WINDOWS)
	struct hostent* phe;

	addr.sin_family = AF_INET;
	addr.sin_port = htons( port );
	phe = gethostbyname( host.c_str() );
	addr.sin_addr = *( (LPIN_ADDR)*phe->h_addr_list );

	int desc = ::connect( sock, (struct sockaddr *)&addr, sizeof(addr) );
	if (!desc)
		state = skCONNECTED;
	return !desc;
#else
	addr.sin_family = AF_INET;
	addr.sin_port = htons( port );
	inet_pton( AF_INET, host.c_str(), &addr.sin_addr );

	int desc = ::connect( sock, (struct sockaddr *)&addr, sizeof(addr) );
	if (!desc)
		state = skCONNECTED;
	return !desc;
#endif
}



bool EzSockets::CanRead()
{
	FD_ZERO(scks);
	FD_SET((unsigned)sock,scks);

	return select(sock+1, scks, NULL, NULL, times) > 0;
}
bool EzSockets::IsError()
{
	if (state == skERROR)
		return true;

	FD_ZERO(scks);
	FD_SET((unsigned)sock,scks);

	return select(sock+1, NULL, NULL, scks, times);
}
bool EzSockets::CanWrite()
{
	FD_ZERO(scks);
	FD_SET((unsigned)sock,scks);

	return select(0, NULL, scks, NULL, times) > 0;
}

void EzSockets::update()
{
	if (state==skERROR)
		return;
	//If socket is in error, don't bother.

	if (IsError())
	{
		state=skERROR;
		return;
	}

	//Check for reading
	while (CanRead() && !IsError())
		if (pUpdateRead()<1)
			break;
 
	if (CanWrite() && (outBuffer.length()>0))
		pUpdateWrite();
}



//Raw data system 
void EzSockets::SendData(string & outData)
{
	outBuffer.append(outData);
	if (blocking)
		while ((outBuffer.length()>0) && !IsError())
			pUpdateWrite();
	else
		update();

}

void EzSockets::SendData(const char *data, unsigned int bytes)
{
	outBuffer.append(data,bytes);
	if (blocking)
		while ((outBuffer.length()>0) && !IsError())
			pUpdateWrite();
	else
		update();
}

int EzSockets::ReadData(char *data, unsigned int bytes)
{	
	int bytesRead = PeekData(data,bytes);

	inBuffer = inBuffer.substr(bytesRead);

	return bytesRead;
}

int EzSockets::PeekData(char *data, unsigned int bytes)
{
	if (blocking)
		while ((inBuffer.length()<bytes) && !IsError())
			pUpdateRead();
	else
		while (CanRead()&&!IsError())
			if (pUpdateRead()<1)
				break;

	int bytesRead = bytes;
	if (inBuffer.length()<bytes)
		bytesRead = inBuffer.length();

	memcpy(data,inBuffer.c_str(),bytesRead);

	return bytesRead;
}

//Packet system (for structures and classes)
void EzSockets::SendPack(char * data, unsigned int bytes)
{
	unsigned int SendSize = htonl(bytes);
	SendData ((char *)&SendSize,sizeof(int));
	SendData (data,bytes);
}

int EzSockets::ReadPack(char * data, unsigned int max)
{
	int size = PeekPack(data,max);
	if (size!=-1)
		inBuffer = inBuffer.substr(size+4);
	return size;
}

int EzSockets::PeekPack(char * data, unsigned int max)
{
	if (CanRead())
		pUpdateRead();

	if (blocking)
	{
		while ((inBuffer.length()<4)&& !IsError())
			pUpdateRead();

		if (IsError())
			return (-1);

//		LOG->Info("TTY:%d",inBuffer.length()
		unsigned int size=0;
		PeekData((char*)&size,4);
		size = ntohl(size);
//		LOG->Info("TXA:%d",size);
		while ((inBuffer.length()<(size+4)) && !IsError())
			pUpdateRead();

		string tBuff(inBuffer.substr(4,size));

		if (tBuff.length()>max)
			tBuff.substr(0,max);

		memcpy (data,tBuff.c_str(),tBuff.length());

		return (size);
	}	
	else if (inBuffer.length()>3)
	{
		unsigned int size=0;
		PeekData((char*)&size,4);
		size = ntohl(size);
		if ((inBuffer.length()<(size+4)) || (inBuffer.length()<=4))
			return -1;

		string tBuff(inBuffer.substr(4,size));

		if (tBuff.length()>max)
			tBuff.substr(0,max);

		memcpy (data,tBuff.c_str(),tBuff.length());

		return size;
	}
	return -1;
}


//String (Flash) system / Null-terminated strings
void EzSockets::SendStr(string & data, char delim)
{
	char tDr[1];
	tDr[0] = delim;
	SendData(data.c_str(),data.length());
	SendData(tDr,1);
}

int EzSockets::ReadStr(string & data, char delim)
{
	int t = PeekStr (data, delim);
	if (t!=-1)
		inBuffer = inBuffer.substr(t+1);
	return t;
}
int EzSockets::PeekStr(string & data, char delim)
{
	int t;
	t = inBuffer.find(delim,0);
	if (blocking)
	{
		while ((t==-1) && !IsError())
		{
			pUpdateRead();
			t = inBuffer.find(delim,0);
		}
		data = inBuffer.substr(0,t);
	}
	else
	{
		if (t == -1)
			return -1;
		data = inBuffer.substr(0,t);
	}
	return t;
}




int EzSockets::pUpdateRead()
{
	char tempData[1024];
	int bytes = pReadData(tempData);
	if (bytes>0)
		inBuffer.append(tempData,bytes);

	//You cannot read - bytes!
	//0 bytes may happen under non-blocking circuimstances
	if (bytes<0) 
		state = skERROR;

	return bytes;
}

int EzSockets::pUpdateWrite()
{
	int bytes = pWriteData(outBuffer.c_str(),outBuffer.length());
	if (bytes>0)
		outBuffer = outBuffer.substr(bytes);
	if (bytes<0)
		state = skERROR;
	return bytes;
}


int EzSockets::pReadData(char * data)
{
	return recv( sock, data,1024, 0 );
}

int EzSockets::pWriteData(const char * data, int dataSize)
{
	return send( sock, data, dataSize, 0 );
}


istream& operator >>(istream &is,EzSockets &obj)
{
	string writeString;
	obj.SendStr(writeString);
	is>>writeString;
	return is;
}
ostream& operator <<(ostream &os, EzSockets &obj)
{
	string readString;
	obj.ReadStr(readString);
	os<<readString;
	return os;
}

/*
 * (c) 2003-2004 Charles Lohr, Josh Allen
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
