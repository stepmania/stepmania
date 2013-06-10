// socket.h

#ifndef SOCKET_H
#define SOCKET_H

#pragma comment(lib, "../SDL-1.2.5/lib/SDL.lib")
#pragma comment(lib, "../SDL_net-1.2.5/lib/SDL_net.lib")

#include "../SDL_net-1.2.5/include/SDL_net.h"

//HACK: SDL_net doesn't appear to have a way to get the host name
//		which I need to resolve the machine's ip address
//		(no. using "localhost" doesn't work. you'll get 127.0.0.1 @_@)
#ifdef WIN32
	//int FAR PASCAL gethostname(char FAR * name, int namelen);
	#undef INADDR_ANY
	#undef INADDR_NONE
	#include <winsock2.h>
#elif UNIX
	#include <unistd.h>
	#include <arpa/inet.h>
#endif

class NetworkInit
{
public :
	NetworkInit();
	~NetworkInit();
};


class InetAddr : public IPaddress
{
public :
	InetAddr(short wPort = 0);
	InetAddr(long dwIP, short wPort);
	InetAddr(const char* lpszAddress, short wPort = 0);
	InetAddr& operator = (char* lpszAddress);

protected :
	void Resolve(const char* lpszAddress, short wPort = 0);
};

class Socket
{
public :
	Socket();
	Socket(TCPsocket s);
	Socket(const Socket& s);
	virtual ~Socket();

	void Close();
	bool Bind(const InetAddr& addr);
	bool Connect(const InetAddr& addr);
	bool Listen();
	Socket Accept();
	int Send(const unsigned char* buf, int cbBuf);
	int Send(const char* fmt, ...);
	int Receive(unsigned char* buf, int cbBuf);
	operator TCPsocket& () const { return (TCPsocket&)m_sock; }
	operator bool() const { return m_sock != NULL; }

protected:
	InetAddr m_bind_addr;
	TCPsocket m_sock;

private :
	bool m_bOwnSocket;
};


#endif
