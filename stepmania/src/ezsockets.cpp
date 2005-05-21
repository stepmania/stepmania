/*******************************************************************\
| ezsockets.cpp: EzSockets Class Source                             |
|   Designed by Josh Allen, Charles Lohr and Adam Lowman.           |
|   Socket programming methods based on Charles Lohr's EZW progam.  |
|   Modified by Charles Lohr for use with Windows-Based OSes.       |
|   UDP/NON-TCP Support by Adam Lowman.                             |
\*******************************************************************/

#include "ezsockets.h"

#if defined(_XBOX)
#elif defined(_MSC_VER) // We need the WinSock32 Library on Windows
#pragma comment(lib,"wsock32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#if !defined(SOCKET_ERROR)
#define SOCKET_ERROR -1
#endif

//There are cases where 0 isn't a proper socket
#if !defined(SOCKET_NONE)
#define SOCKET_NONE 0
#endif

#if !defined(INVALID_SOCKET)
#define INVALID_SOCKET -1
#endif

EzSockets::EzSockets()
{
	MAXCON = 5;
	memset (&addr,0,sizeof(addr)); //Clear the sockaddr_in structure
	
#if defined(_WINDOWS) || defined(_XBOX) // Windows REQUIRES WinSock Startup
	WSAStartup( MAKEWORD(1,1), &wsda );
#endif
	
	sock = INVALID_SOCKET;
	blocking = true;
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

//Check to see if the socket has been created
bool EzSockets::check()
{
#if !defined(XBOX)
	return sock > SOCKET_NONE;
#else
	return sock != INVALID_SOCKET;
#endif
}

bool EzSockets::create()
{
	return create(IPPROTO_TCP, SOCK_STREAM);
}

bool EzSockets::create(int Protocol)
{
	switch(Protocol)
	{
	case IPPROTO_TCP:
		return create(IPPROTO_TCP, SOCK_STREAM);
	case IPPROTO_UDP:
		return create(IPPROTO_UDP, SOCK_DGRAM);
	default:
		//XBOX does not support the raw socket.
		//So, since there's no need, we aren't
		//going to allow it on XBOX
#if defined(_XBOX)
		return false;
#else
		return create(Protocol, SOCK_RAW);
#endif
			
	}
}

bool EzSockets::create(int Protocol, int Type)
{
	state = skDISCONNECTED;
	sock = socket(AF_INET, Type, Protocol);
	lastCode = sock;
#if !defined(XBOX)
	return sock > SOCKET_NONE;	//Socket must be Greater than 0
#else
	return sock != INVALID_SOCKET;
#endif
}


bool EzSockets::bind(unsigned short port)
{
	if(!check())
		return false;
	
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port        = htons(port);
	lastCode = ::bind(sock,(struct sockaddr*)&addr, sizeof(addr));
	return !lastCode;
}

bool EzSockets::listen()
{
	lastCode = ::listen(sock, MAXCON);
	if (lastCode == SOCKET_ERROR)
		return false;
	
	state = skLISTENING;
	return true;
}

#if defined(WIN32)
typedef int socklen_t;
#endif

bool EzSockets::accept(EzSockets& socket)
{

	if (!blocking && !CanRead())
		return false;

	#if defined(HAVE_INET_NTOP)
		char buf[INET_ADDRSTRLEN];

		inet_ntop(AF_INET, &addr.sin_addr, buf, INET_ADDRSTRLEN);
		address = buf;

	#elif defined(HAVE_INET_NTOA)
		address = inet_ntoa(addr.sin_addr);
	#endif
	
	int length = sizeof(socket);
	
	socket.sock = ::accept(sock,(struct sockaddr*) &socket.addr, 
						   (socklen_t*) &length);
	
	lastCode = socket.sock;

	if ( socket.sock == SOCKET_ERROR )
		return false;
	
	socket.state = skCONNECTED;
	return true;
}

void EzSockets::close()
{
	state = skDISCONNECTED;
	inBuffer = "";
	outBuffer = "";
	
#if defined(WIN32) // The close socket command is different in Windows
	::closesocket(sock);
#else
	::close(sock);
#endif
}

long EzSockets::uAddr()
{
	return addr.sin_addr.s_addr;
}


bool EzSockets::connect(const std::string& host, unsigned short port)
{
	if(!check())
		return false;
	
#if defined(_XBOX)
	if(!isdigit(host[0])) // don't do a DNS lookup for an IP address
	{
		XNDNS *pxndns = NULL;
		XNetDnsLookup(host.c_str(), NULL, &pxndns);
		while (pxndns->iStatus == WSAEINPROGRESS)
		{
			// Do something else while lookup is in progress
		}
		
		if (pxndns->iStatus == 0)
			memcpy(&addr.sin_addr, &pxndns->aina[0], sizeof(struct in_addr));
		else
			return false;
		
		XNetDnsRelease(pxndns);
	}
	else
		addr.sin_addr.s_addr = inet_addr(host.c_str());
#else
	struct hostent* phe;
	phe = gethostbyname(host.c_str());
	if (phe == NULL)
		return false;
	memcpy(&addr.sin_addr, phe->h_addr, sizeof(struct in_addr));
#endif 
	addr.sin_family = AF_INET;
	addr.sin_port   = htons(port);
	
	if(::connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
		return false;
	
	state = skCONNECTED;
	return true;
}

bool EzSockets::CanRead()
{
	FD_ZERO(scks);
	FD_SET((unsigned)sock, scks);
	
	return select(sock+1,scks,NULL,NULL,times) > 0;
}

bool EzSockets::IsError()
{
	if (state == skERROR)
		return true;
	
	FD_ZERO(scks);
	FD_SET((unsigned)sock, scks);
	
	if (select(sock+1, NULL, NULL, scks, times) >=0 )
		return false;
	
	state = skERROR;
	return true;
}

bool EzSockets::CanWrite()
{
	FD_ZERO(scks);
	FD_SET((unsigned)sock, scks);
	
	return select(sock+1, NULL, scks, NULL, times) > 0;
}

void EzSockets::update()
{
	if (IsError()) //If socket is in error, don't bother.
		return;
	
	while (CanRead() && !IsError()) //Check for Reading
		if (pUpdateRead() < 1)
			break;
	
	if (CanWrite() && (outBuffer.length()>0))
		pUpdateWrite();
}


/*********************\
|   Raw Data System   |
\*********************/
void EzSockets::SendData(const string& outData)
{
	outBuffer.append(outData);
	if(blocking)
		while ((outBuffer.length()>0) && !IsError())
			pUpdateWrite();
	else
		update();
}

void EzSockets::SendData(const char *data, unsigned int bytes)
{
	outBuffer.append(data, bytes);
	if(blocking)
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
		while (CanRead() && !IsError())
			if (pUpdateRead()<1)
				break;
	
	int bytesRead = bytes;
	if (inBuffer.length()<bytes)
		bytesRead = inBuffer.length();
	memcpy(data,inBuffer.c_str(), bytesRead);
	
	return bytesRead;
}


/**********************************\
|   Packet/Structure Data System   |
\**********************************/
void EzSockets::SendPack(const char *data, unsigned int bytes)
{
	unsigned int SendSize = htonl(bytes);
	outBuffer.append( (const char *) & SendSize, 4);	//Add size to buffer, but don't send yet.
	SendData(data, bytes);
}

int EzSockets::ReadPack(char *data, unsigned int max)
{
	int size = PeekPack(data, max);
	
	if (size != -1)
		inBuffer = inBuffer.substr(size+4);
	
	return size;
}

int EzSockets::PeekPack(char *data, unsigned int max)
{
	if (CanRead())
		pUpdateRead();
	
	if (blocking)
	{
		while ((inBuffer.length()<4) && !IsError())
			pUpdateRead();
		
		if (IsError())
			return -1;
	}
	
	if (inBuffer.length()<4)
		return -1;
	
	unsigned int size;
	PeekData((char*)&size, 4);
	size = ntohl(size);
	
	if (blocking)
		while (inBuffer.length()<(size+4) && !IsError())
			pUpdateRead();
	else
		if (inBuffer.length()<(size+4) || inBuffer.length()<=4)
			return -1;
	
	if (IsError())
		return -1; 
	//What if we get disconnected while waiting for data?
	
	string tBuff(inBuffer.substr(4, size));
	if (tBuff.length() > max)
		tBuff.substr(0, max);
	
	memcpy (data, tBuff.c_str(),tBuff.length());
	return size;
}


/*****************************************\
|   Null Terminating String Data System   |
\*****************************************/
void EzSockets::SendStr(const string& data, char delim)
{
	char tDr[1];
	tDr[0] = delim;
	SendData(data.c_str(), data.length());
	SendData(tDr, 1);
}

int EzSockets::ReadStr(string& data, char delim)
{
	int t = PeekStr(data, delim);
	if (t >= 0)
		inBuffer = inBuffer.substr(t+1);
	return t;
}

int EzSockets::PeekStr(string& data, char delim)
{
	int t = inBuffer.find(delim,0);
	if (blocking)
	{
		while (t == -1 && !IsError())
		{
			pUpdateRead();
			t = inBuffer.find(delim, 0);
		}
		data = inBuffer.substr(0, t);
	}
	
	if(t >= 0)
		data = inBuffer.substr(0, t);
	return t;
}


/************************\
|   Stream Data System   |
\************************/
istream& operator>>(istream &is, EzSockets& obj)
{
	string writeString;
	obj.SendStr(writeString);
	is >> writeString;
	return is;
}

ostream& operator<<(ostream &os, EzSockets &obj)
{
	string readString;
	obj.ReadStr(readString);
	os << readString;
	return os;
}


/**************************\
|   Internal Data System   |
\**************************/
int EzSockets::pUpdateRead()
{
	char tempData[1024];
	int bytes = pReadData(tempData);
	
	if (bytes > 0)
		inBuffer.append(tempData, bytes);
	else if (bytes <= 0)
		/* To get her I think CanRead was called at least once.
		So if length equals 0 and can read says there is data than 
		the socket was closed.*/
		state = skERROR;
	return bytes;
}

int EzSockets::pUpdateWrite()
{
	int bytes = pWriteData(outBuffer.c_str(), outBuffer.length());
	
	if (bytes > 0)
		outBuffer = outBuffer.substr(bytes);
	else if (bytes < 0)
		state = skERROR;
	return bytes;
}


int EzSockets::pReadData(char* data)
{
	if(state == skCONNECTED || state == skLISTENING)
		return recv(sock, data, 1024, 0);
	
	fromAddr_len = sizeof(sockaddr_in);
	return recvfrom(sock, data, 1024, 0, (sockaddr*)&fromAddr,
					(socklen_t*)&fromAddr_len);
}

int EzSockets::pWriteData(const char* data, int dataSize)
{
	return send(sock, data, dataSize, 0);
}

/* 
 * (c) 2003-2004 Josh Allen, Charles Lohr, and Adam Lowman
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
