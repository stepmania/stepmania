/* EzSockets - BSD sockets class */
 
#ifndef EZSOCKETS_H
#define EZSOCKETS_H

#include <sstream>
#include <string>
#include <vector>
#include <fcntl.h>
#include <ctype.h>

#if defined(WIN32)
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

using namespace std;

class EzSockets {
public:

	EzSockets();
	~EzSockets();

	//Crate the socket
	int create();

	//Bind Socket to local port
	bool bind(unsigned short port);

	//Listen with socket
	bool listen();

	//Accept incomming socket
	bool accept(EzSockets &socket);

	//Connect
	bool connect(const std::string& host, unsigned short port);

	//Kill socket
	void close();

	//see if socket has been created
	bool check();

	long uAddr();

	bool CanRead();
	bool IsError();
	bool CanWrite();
	
	void update();

	//Raw data system 
	void SendData(string & outData);
	void SendData(const char *data, unsigned int bytes);
	int ReadData(char *data, unsigned int bytes);
	int PeekData(char *data, unsigned int bytes);

	//Packet system (for structures and classes)
	void SendPack(char * data, unsigned int bytes); 
	int ReadPack(char * data, unsigned int max);
	int PeekPack(char * data, unsigned int max);

	//String (Flash) system / Null-terminated strings
	void SendStr(string & data, char delim = '\0');
	int ReadStr(string & data, char delim = '\0');
	int PeekStr(string & data, char delim = '\0');


	//Operators
	char operator[] (int i); //Access buffer
	friend istream& operator >>(istream &is,EzSockets &obj);
	friend ostream& operator <<(ostream &os,const EzSockets &obj);


	bool blocking;


	//The following possibly should be private.
	string inBuffer;
	string outBuffer;

	int pUpdateWrite();
	int pUpdateRead();

	int pReadData(char * data);
	int pWriteData(const char * data, int dataSize);


	enum SockState { 
		skDISCONNECTED = 0, 
		skUNDEF1, //Not implemented
		skLISTENING, 
		skUNDEF3, //Not implemented
		skUNDEF4, //Not implemented
		skUNDEF5, //Not implemented
		skUNDEF6, //Not implemented
		skCONNECTED, 
		skERROR 
	};

	SockState state;

private:

	//Only necessiary in windows
#if defined(WIN32)
	WSADATA wsda;
#endif

	int MAXCON;
	int sock;
	struct sockaddr_in addr;


	//Used for Select() command
	fd_set  * scks;
	timeval * times;

	//Buffers
};

	istream& operator >>(istream &is,EzSockets &obj);
	ostream& operator <<(ostream &os,EzSockets &obj);


#endif

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
