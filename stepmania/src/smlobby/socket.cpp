// socket.cpp

#include "StdAfx.h"
#include <stdio.h>
#include "socket.h"

//////////////////////////////////////////////////////////////////////////

NetworkInit::NetworkInit()
{
	if (-1==SDL_Init(0))
	{
		printf("SDL_Init: %s\n", SDL_GetError());
		exit(1);
	}

	if (-1==SDLNet_Init())
	{
		printf("SDLNet_Init: %s\n", SDLNet_GetError());
		exit(2);
	}
}

NetworkInit::~NetworkInit()
{
	SDLNet_Quit();
	SDL_Quit();
}

//////////////////////////////////////////////////////////////////////////

InetAddr::InetAddr(short wPort)
{
	host = 0xFFFFFFFFL;
	port = wPort;
}

InetAddr::InetAddr(long dwIP, short wPort)
{
	host = dwIP;
	port = wPort;
}

InetAddr::InetAddr(const char* lpszAddress, short wPort)
{
	Resolve(lpszAddress, wPort);
}

InetAddr& InetAddr::operator = (char* lpszAddress)
{
	Resolve(lpszAddress);
	return *this;
}

void InetAddr::Resolve(const char* lpszAddress, short wPort)
{
	SDLNet_ResolveHost((IPaddress*)this, (char *)lpszAddress, wPort); 
}

//////////////////////////////////////////////////////////////////////////

Socket::Socket()
	: m_sock(NULL), m_bOwnSocket(false)
{
}

Socket::Socket(const Socket& s)
	: m_sock(s.m_sock), m_bOwnSocket(false)
{
}

Socket::Socket(TCPsocket s)
	: m_sock(s), m_bOwnSocket(false)
{
}

Socket::~Socket()
{
	if( m_bOwnSocket && m_sock != NULL )
		Close();
}

void Socket::Close()
{
	if (m_sock) SDLNet_TCP_Close(m_sock);
	m_sock = NULL;
}

bool Socket::Bind(const InetAddr& addr)
{
	m_bind_addr = addr;
	return true;
}

bool Socket::Connect(const InetAddr& ip)
{
	m_sock = SDLNet_TCP_Open((IPaddress *)&ip);
	return m_sock != NULL;
}

bool Socket::Listen()
{
	m_sock = SDLNet_TCP_Open((IPaddress *)&m_bind_addr);
	return m_sock != NULL;
}

Socket Socket::Accept()
{
	const int kMaxRetry = 10;
	TCPsocket new_sock;

	for (int i = 0; i < kMaxRetry; i++)
	{
		new_sock = SDLNet_TCP_Accept(m_sock); 
		if ( new_sock ) break;
	}

	return Socket(new_sock);
}

int Socket::Send(const unsigned char* buf, int cbBuf)
{
	return SDLNet_TCP_Send(m_sock, (void *)buf, cbBuf);
}

int Socket::Send(const char* fmt, ...)
{
	va_list marker;
	va_start(marker, fmt);

	char szBuf[1024*4];
	vsprintf(szBuf, fmt, marker);

	va_end(marker);

	return Send((unsigned char*)szBuf, strlen(szBuf));
}

int Socket::Receive(unsigned char* buf, int cbBuf)
{
	return SDLNet_TCP_Recv(m_sock, (char*)buf, cbBuf);
}