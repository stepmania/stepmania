/*******************************************************************\
| ezsockets.h: EzSockets Class Header                               |
|   Designed by Josh Allen, Charles Lohr and Adam Lowman.           |
|   Socket programming methods based on Charles Lohr's EZW progam.  |
|   Modified by Charles Lohr for use with Windows-Based OSes.       |
|   UDP/NON-TCP Support by Adam Lowman.                             |
\*******************************************************************/
 
#ifndef EZSOCKETS_H
#define EZSOCKETS_H

#if defined(WITHOUT_NETWORKING)
#error do not include ezsockets.h when WITHOUT_NETWORKING
#endif

#include <sstream>
#include <string>
#include <vector>
#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif
#include <ctype.h>

using namespace std;

enum EzSockets_Proto
{
	EZS_NONE,
	EZS_TCP,
	EZS_UDP
};

class EzSockets
{
public:

	EzSockets();
	~EzSockets();

	//Crate the socket
	bool create();
	bool create(EzSockets_Proto Protocol);
	bool create(EzSockets_Proto Protocol, int Type);

	//Bind Socket to local port
	bool bind(unsigned short port);

	//Listen with socket
	bool listen();

	//Accept incoming socket
	bool accept(EzSockets &socket);

	//Connect
	bool connect(const string& host, unsigned short port);

	//Kill socket
	void close();

	//see if socket has been created
	bool check();

	long uAddr();

	bool CanRead();
	bool CanRead(unsigned int msTimeout);
	bool DataAvailable() { return ( ( inBuffer.length()>0 ) || CanRead() ); }
	bool DataAvailable(unsigned int msTimeout) { return ( ( inBuffer.length()>0 ) || CanRead(msTimeout) ); }
	bool IsError();
	bool CanWrite();
	bool CanWrite(unsigned int msTimeout);

	void update();

	//Raw data system 
	void SendData(const string& outData);
	void SendData(const char *data, unsigned int bytes);
	int ReadData(char *data, unsigned int bytes);
	int PeekData(char *data, unsigned int bytes);

	//Packet system (for structures and classes)
	void SendPack(const char *data, unsigned int bytes); 
	int ReadPack(char *data, unsigned int max);
	int PeekPack(char *data, unsigned int max);

	//String (Flash) system / Null-terminated strings
	void SendStr(const string& data, char delim = '\0');
	int ReadStr(string& data, char delim = '\0');
	int PeekStr(string& data, char delim = '\0');


	//Operators
	char operator[] (int i); //Access buffer
	friend istream& operator>>(istream& is, EzSockets& obj);
	friend ostream& operator<<(ostream& os, const EzSockets& obj);

	bool blocking;
	enum SockState
	{ 
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

	uint32_t getAddress();

	// The following possibly should be private.
	string inBuffer;
	string outBuffer;

	int pUpdateWrite();
	int pUpdateRead();

	int pReadData(char * data);
	int pWriteData(const char * data, int dataSize);

	SockState state;

	int lastCode;	// Used for debugging purposes

	RString address;

	// Wrapped here so we don't have to leak winapi everywhere...
	static uint32_t sm_ntohl(uint32_t);
	static uint16_t sm_ntohs(uint16_t);
	static uint32_t sm_htonl(uint32_t);
	static uint16_t sm_htons(uint16_t);

private:

	void *opaque;
};

istream& operator>>(istream& is, EzSockets& obj);
ostream& operator<<(ostream& os, EzSockets& obj);


#endif

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
