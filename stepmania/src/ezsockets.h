/*******************************************
 EzSockets - a multiplatform generic 
 sockets class (Version 2)
 Original code by Josh Allen
 Modified by Charles Lohr

  All contents of this file Copyright 
  2003-2004 Charles Lohr 
			Joshua Allen

  This file may be used under what
  license the StepMania project is using.
	
********************************************/
 
#ifndef __EZSOCKETS_H
#define __EZSOCKETS_H

#include <sstream>
#include <string>
#include <vector>
#include <fcntl.h>
#include <ctype.h>

#if defined(WIN32)
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
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
