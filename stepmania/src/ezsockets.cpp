#include "global.h"
/*******************************************
  ezsockets.cpp -- Header for sockets.cpp
   Designed by Josh Allen and Charles
   Lohr. Socket programming methods based
   on Charles Lohr's EZW progam.
   You may freely destribute this code
   if this message is retained.
   This code is distributed on an as-is
   basis with no gaurentees whatsoever.

  Modified by Charles Lohr for use with 
   Windows-Based OSes.
********************************************/

#ifdef _WINDOWS
//We need the Winsock32 Libary
#pragma comment(lib,"wsock32.lib")
#endif

#include "ezsockets.h"

EzSockets::EzSockets() {
  MAXCON = 5;
  memset (&addr, 0, sizeof(addr)); //Clear the sockaddr_in structure

//Windows REQUIRES WinSock Startup
#if defined(WIN32)
  WSAStartup(MAKEWORD(1,1), &wsda);
#endif

  
  sock = -1;
}

EzSockets::~EzSockets() {
  close();
}

bool EzSockets::check() {
  //Check to see if the socket ahs been created
  if (sock < 0) {
    return false;
  }
  return true;
}

int EzSockets::create() {
  sock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
  return (true);
}

bool EzSockets::bind(unsigned short port) {

  if (!check()) {
    return false;
  }

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl (INADDR_ANY);
  addr.sin_port = htons (port);
  
  int desc = ::bind(sock, (struct sockaddr *) & addr, sizeof(addr));
  if (desc < 0) {
    return false;
  }
  return true;
}

bool EzSockets::listen() {
  int desc = ::listen(sock, MAXCON);
  if (desc < 0) {
    return false;
  }
  return true;
}
typedef int socklen_t;

bool EzSockets::accept(EzSockets &socket) {

	//Windows wants it defined as a signed int
    //As does everything else -- Steve
	int length = sizeof(socket);

  socket.sock = ::accept(sock, (struct sockaddr *)&socket.addr,(socklen_t *)&length);
  if (socket.sock < 0) {
    return false;
  }
  return true;
}

void EzSockets::close() {
	//The close socket command is different in Windows
#if defined(WIN32)
  ::closesocket(sock);
#else
  ::close(sock);
#endif
}

bool EzSockets::receiveLength(int &length) {
  length = 0;

//cannot send in void* in Windows.
#if defined(WIN32)
  int desc = recv(sock, (char*)&length, sizeof(int),0);
#else
  int desc = recv(sock, (void*)&length, sizeof(int),0);
#endif

  if (desc <= 0) {
    return false;
  }
  return true;
}

bool EzSockets::receive(int &x) {
  if (!receiveLength(x)) {
    return false;
  }
  return true;
}

bool EzSockets::receive(std::vector<char> &data) {
  char buf[1024];
  /* If the length is less than 1024 then receive
     all the data in one recv() command. If the
     length is greater than 1024 receive data
     with multiple recv() commands.
  */
  int desc=0;
  int length;
  length = 0;
  data.resize(0);
  if (!receiveLength(length)) {
    return false;
  }
  if (length <= 1024) {
    memset (&buf, 0, sizeof(buf));
    desc = recv(sock, buf, 1024, 0);
    for (int x = 0;x < desc;x++) {
      data.push_back(buf[x]);
    }
  } else {
    while (int(data.size()) < length) {
      memset (&buf, 0, sizeof(buf)); //Flush the buffer
      desc = recv(sock, buf, 1024,0);
      for (int x = 0;x < desc;x++) {
	data.push_back(buf[x]);
      }
    }
  }
  if (desc <= 0) {
    return false;
  }
  /* Make sure data is the correct size.
     This maybe uneeded but seems like
     a good idea.
  */
  data.resize(length);
  return true;
}

bool EzSockets::send(const std::string& data) {
  /* First send send the length of the data to the
     server. Then if the length is less than or equal
     to the 1024 then send data in one sendData()
     commans. If data is larger then 1024 than
     send the data in multiple sendData() commands.
     Each sendData() sends as much data as 1024.
     HAVE NOT CHECK TO SEE IF TRANSFER OF LARGE DATA
     IS CORRECT.
  */

  int desc = sendLength(data.length());
  if (desc == 0) {
    return false;
  }
  if (data.length() <= (1024)) {
    desc = sendData((char*)data.c_str(), data.length());
    if (desc == 0) {
      return false;
    }
  } else {
    for (int x = 0;x < int(data.length());x += 1024) {
      desc = sendData((char*)std::string(data.substr(x, x+1024)).c_str());
    }
    if (desc == 0) {
      return false;
    }
  }
  return true;
}

bool EzSockets::send(char *data, int length) {
  /* Sends the data in chunks of 1024. Appears to
     work fine with large and small data ammounts.
  */
  int desc;
  char buf[1024];
  desc = sendLength(length);
  if (desc == 0) {
    return false;
  }
  for (int x = 0;x < length;x += 1024) {
    memset (&buf, 0, sizeof(buf));
    for (int bytes = 0;bytes < 1024;bytes++) {
      buf[bytes] = (char)data[x+bytes];
    }
    if (length < 1024) {
      desc = sendData(buf, length);
    } else {
      desc = sendData(buf);
    }
    if (desc == 0) {
      return false;
    }
  }
  return true;
}

bool EzSockets::sendLength(int length) {
#if defined(WIN32)
  int desc = ::send(sock, (char*)&length, sizeof(int), 0);
#else
  int desc = ::send(sock, (void*)&length, sizeof(int), 0);
#endif
  if (desc <= 0) {
    return false;
  }
  return true;
}

bool EzSockets::send(int x) {
  if (!sendLength(x)) {
    return false;
  }
  return true;
}

bool EzSockets::sendData(char data[1024], int size) {
  /*This function sends data just fine.
    Problem is with the reassemble while
    receiving data.*/
  int desc;
  int lpos = 0;

  if (size==0)
	  return true;

  while (size-lpos > 0) {
	desc = ::send(sock, *((&data)+lpos), size, 0);
	if (desc<=0)
		return false;
	lpos=lpos+desc;
  }
  return true;
}

bool EzSockets::connect(const std::string& host, unsigned short port) {
  if (!check())
    return false;

#if defined(WIN32)
  struct hostent* phe;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  phe = gethostbyname(host.c_str());
  addr.sin_addr = *((LPIN_ADDR)*phe->h_addr_list);

  int desc = ::connect(sock,(struct sockaddr *)&addr,sizeof(addr));
#else
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  inet_pton ( AF_INET, host.c_str(), &addr.sin_addr );

  int desc = ::connect(sock,(struct sockaddr *)&addr,sizeof(addr));
#endif

  return desc >= 0;
}

long EzSockets::uAddr() {
  return addr.sin_addr.s_addr;
}

