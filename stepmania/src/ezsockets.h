/*******************************************
  ezsockets.h -- Header for sockets.cpp
   Designed by Josh Allen and Charles
   Lohr. Socket programming methods based
   on Charles Lohr's EZW progam.
   You may freely destribute this code
   if this message is retained.
   This code is distributed on an as-is
   basis with no gaurentees whatsoever.

  Modified by Charles Lohr for use on
   windows and Unix.
********************************************/

#ifndef __EZSOCKETS_H
#define __EZSOCKETS_H

#include <string>
#include <vector>

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

class EzSockets {
 private:

  #if defined(WIN32)
	 WSADATA wsda;
  #endif


  int MAXCON;
  int sock;
  struct sockaddr_in addr;

  bool sendData(char data[1024], int size = 1024);
  bool sendLength(int length);
  bool receiveLength(int &length);
 public:

  EzSockets();
  ~EzSockets();
  //Check to see if Socket is active
  bool check();	
  //Crate the socket
  int create();
  //Bind Socket to local port
  bool bind(unsigned short port);
  //Listen with socket
  bool listen();
  //Accept incomming socket
  bool accept(EzSockets &socket);
  //Receive raw data
  bool receive(std::vector<char> &data);
  //Receive structure
  bool receive(int &x);
  //Send string
  bool send(const std::string& data);
  //Send Structure (or other data)
  bool send(char *data, int length);
  //Send Integer
  bool send(int x);
  //Connect to remote host
  //NOTE: YOU MUST PUT IN IP, NOT NAME
  bool connect(const std::string& host, unsigned short port);
  
  long uAddr(); 
  //Kill socket
  void close();
};

#endif
