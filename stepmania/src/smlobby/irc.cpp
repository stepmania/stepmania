// irc.cpp

#include "stdafx.h"
#include "irc.h"

using namespace irc;

////////////////////////////////////////////////////////////////////

CIrcMessage::CIrcMessage(const char* lpszCmdLine, bool bIncoming)
	: m_bIncoming(bIncoming)
{
	ParseIrcCommand(lpszCmdLine);
}

CIrcMessage::CIrcMessage(const CIrcMessage& m)
	:	sCommand(m.sCommand),
		parameters(m.parameters),
		m_bIncoming(m.m_bIncoming)
{
	prefix.sNick = m.prefix.sNick;
	prefix.sUser = m.prefix.sUser;
	prefix.sHost = m.prefix.sHost;
}

void CIrcMessage::Reset()
{
	prefix.sNick = prefix.sUser = prefix.sHost = sCommand = "";
	m_bIncoming = false;
	parameters.clear();
}

CIrcMessage& CIrcMessage::operator = (const CIrcMessage& m)
{
	if( &m != this )
	{
		sCommand = m.sCommand;
		parameters = m.parameters;
		prefix.sNick = m.prefix.sNick;
		prefix.sUser = m.prefix.sUser;
		prefix.sHost = m.prefix.sHost;
		m_bIncoming = m.m_bIncoming;
	}
	return *this;
}

CIrcMessage& CIrcMessage::operator = (const char* lpszCmdLine)
{
	Reset();
	ParseIrcCommand(lpszCmdLine);
	return *this;
}

void CIrcMessage::ParseIrcCommand(const char* lpszCmdLine)
{
	const char* p1 = lpszCmdLine;
	const char* p2 = lpszCmdLine;

	ASSERT(lpszCmdLine != NULL);
	ASSERT(*lpszCmdLine);

	// prefix exists ?
	if( *p1 == ':' )
	{ // break prefix into its components (nick!user@host)
		p2 = ++p1;
		while( *p2 && !strchr(" !", *p2) )
			++p2;
		prefix.sNick.assign(p1, p2 - p1);
		if( *p2 != '!' )
			goto end_of_prefix;
		p1 = ++p2;
		while( *p2 && !strchr(" @", *p2) )
			++p2;
		prefix.sUser.assign(p1, p2 - p1);
		if( *p2 != '@' )
			goto end_of_prefix;
		p1 = ++p2;
		while( *p2 && !isspace(*p2) )
			++p2;
		prefix.sHost.assign(p1, p2 - p1);
end_of_prefix :
		while( *p2 && isspace(*p2) )
			++p2;
		p1 = p2;
	}

	// get command
	ASSERT(*p1 != '\0');
	p2 = p1;
	while( *p2 && !isspace(*p2) )
		++p2;
	sCommand.assign(p1, p2 - p1);
	_strupr((char*)sCommand.c_str());
	while( *p2 && isspace(*p2) )
		++p2;
	p1 = p2;

	// get parameters
	while( *p1 )
	{
		if( *p1 == ':' )
		{
			++p1;
			// seek end-of-message
			while( *p2 )
				++p2;
			parameters.push_back(String(p1, p2 - p1));
			break;
		}
		else
		{
			// seek end of parameter
			while( *p2 && !isspace(*p2) )
				++p2;
			parameters.push_back(String(p1, p2 - p1));
			// see next parameter
			while( *p2 && isspace(*p2) )
				++p2;
			p1 = p2;
		}
	} // end parameters loop
}

String CIrcMessage::AsString() const
{
	String s;

	if( prefix.sNick.length() )
	{
		s += ":" + prefix.sNick;
		if( prefix.sUser.length() && prefix.sHost.length() )
			s += "!" + prefix.sUser + "@" + prefix.sHost;
		s += " ";
	}

	s += sCommand;

	for(int i=0; i < parameters.size(); i++)
	{
		s += " ";
		if( i == parameters.size() - 1 ) // is last parameter ?
			s += ":";
		s += parameters[i];
	}

	s += endl;

	return s;
}

////////////////////////////////////////////////////////////////////

CIrcSession::CIrcSession(IIrcSessionMonitor* pMonitor)
	:	m_hThread(NULL)
{
	InitializeCriticalSection(&m_cs);
}

CIrcSession::~CIrcSession()
{
	Disconnect();
	DeleteCriticalSection(&m_cs);
}

bool CIrcSession::Connect(const CIrcSessionInfo& info)
{
	ASSERT(m_hThread==NULL && !m_socket);

	try
	{
//		if( !m_socket.Create() )
//			throw "Failed to create socket!";

		InetAddr addr(info.sServer.c_str(), info.iPort);
		if( !m_socket.Connect(addr) )
		{
			m_socket.Close();
			throw "Failed to connect to host!";
		}

		m_info = info;

		// start receiving messages from host
		m_hThread = CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
		Sleep(100);

		if( info.sPassword.length() )
			m_socket.Send("PASS %s\r\n", info.sPassword.c_str());

		m_socket.Send("NICK %s\r\n", info.sNick.c_str());

		TCHAR szHostName[MAX_PATH];
		DWORD cbHostName = sizeof(szHostName);
		GetComputerName(szHostName, &cbHostName);

		m_socket.Send("USER %s %s %s :%s\r\n", 
			info.sUserID.c_str(), szHostName, "server", info.sFullName.c_str());
	}
	catch( const char* )
	{
		Disconnect();
	}
	catch( ... )
	{
		Disconnect();
	}

	return (bool)m_socket;
}

void CIrcSession::Disconnect(const char* lpszMessage)
{
	static const DWORD dwServerTimeout = 5 * 1000;

	if( !m_hThread )
		return;

	m_socket.Send("QUIT :%s\r\n", lpszMessage ? lpszMessage : "Bye!");

	if( m_hThread && WaitForSingleObject(m_hThread, dwServerTimeout) != WAIT_OBJECT_0 )
	{
		m_socket.Close();
		Sleep(100);
		if( m_hThread && WaitForSingleObject(m_hThread, dwServerTimeout) != WAIT_OBJECT_0 )
		{
			TerminateThread(m_hThread, 1);
			CloseHandle(m_hThread);
			m_hThread = NULL;
			m_info.Reset();
		}
	}
}

void CIrcSession::Notify(const CIrcMessage* pmsg)
{
	// forward message to monitor objects
	EnterCriticalSection(&m_cs);
	for(std::set<IIrcSessionMonitor*>::iterator it = m_monitors.begin();
			it != m_monitors.end();
			it++
			)
	{
		(*it)->OnIrcMessage(pmsg);
	}
	LeaveCriticalSection(&m_cs);
}

void CIrcSession::DoReceive()
{
	CIrcIdentServer m_identServer;
	char chBuf[1024*4+1];
	int cbInBuf = 0;

	if( m_info.bIdentServer )
		m_identServer.Start(m_info.sUserID.c_str());

	while( m_socket )
	{
		int cbRead;
		int nLinesProcessed = 0;

		cbRead = m_socket.Receive((unsigned char*)chBuf+cbInBuf, sizeof(chBuf)-cbInBuf-1);
		if( cbRead <= 0 )
			break;
		cbInBuf += cbRead;
		chBuf[cbInBuf] = '\0';

		char* pStart = chBuf;
		while( *pStart )
		{
			char* pEnd;

			// seek end-of-line
			for(pEnd=pStart; *pEnd && *pEnd != '\r' && *pEnd != '\n'; ++pEnd)
				;
			if( *pEnd == '\0' )
				break; // uncomplete message. stop parsing.

			++nLinesProcessed;

			// replace end-of-line with NULLs and skip
			while( *pEnd == '\r' || *pEnd == '\n' )
				*pEnd++ = '\0';

			if( *pStart )
			{
				// process single message by monitor objects
				CIrcMessage msg(pStart, true);
				Notify(&msg);
			}

			cbInBuf -= pEnd - pStart;
			ASSERT(cbInBuf >= 0);

			pStart = pEnd;
		}

		// discard processed messages
		if( nLinesProcessed != 0 )
			memmove(chBuf, pStart, cbInBuf+1);
	}

	if( m_socket )
		m_socket.Close();

	if( m_info.bIdentServer )
		m_identServer.Stop();

	// notify monitor objects that the connection has been closed
	Notify(NULL);
}

DWORD WINAPI CIrcSession::ThreadProc(LPVOID pparam)
{
	CIrcSession* pThis = (CIrcSession*)pparam;
	try { pThis->DoReceive(); } catch( ... ) {}
	pThis->m_info.Reset();
	CloseHandle(pThis->m_hThread);
	pThis->m_hThread = NULL;
	return 0;
}

void CIrcSession::AddMonitor(IIrcSessionMonitor* pMonitor)
{
	ASSERT(pMonitor != NULL);
	EnterCriticalSection(&m_cs);
	m_monitors.insert(pMonitor);
	LeaveCriticalSection(&m_cs);
}

void CIrcSession::RemoveMonitor(IIrcSessionMonitor* pMonitor)
{
	ASSERT(pMonitor != NULL);
	EnterCriticalSection(&m_cs);
	m_monitors.erase(pMonitor);
	LeaveCriticalSection(&m_cs);
}

////////////////////////////////////////////////////////////////////

CIrcSessionInfo::CIrcSessionInfo()
	:	iPort(0), bIdentServer(false), iIdentServerPort(0)
{
}

CIrcSessionInfo::CIrcSessionInfo(const CIrcSessionInfo& si)
	:	sServer(si.sServer),
		sServerName(si.sServerName),
		iPort(si.iPort),
		sNick(si.sNick),
		sUserID(si.sUserID),
		sFullName(si.sFullName),
		sPassword(si.sPassword),
		bIdentServer(si.bIdentServer),
		sIdentServerType(si.sIdentServerType),
		iIdentServerPort(si.iIdentServerPort)
{
}

void CIrcSessionInfo::Reset()
{
	sServer = "";
	sServerName = "";
	iPort = 0;
	sNick = "";
	sUserID = "";
	sFullName = "";
	sPassword = "";
	bIdentServer = false;
	sIdentServerType = "";
	iIdentServerPort = 0;
}

////////////////////////////////////////////////////////////////////

CIrcIdentServer::CIrcIdentServer()
	: m_uiPort(0), m_hThread(NULL)
{
}

CIrcIdentServer::~CIrcIdentServer()
{
	Stop();
}

bool CIrcIdentServer::Start(
				const char* lpszUserID,
				unsigned int uiPort,
				const char* lpszResponseType
				)
{
	if( m_socket )
		return false;

	if( !m_socket.Bind(InetAddr(uiPort)) )
	{
		m_socket.Close();
		return false;
	}

	m_sResponseType = lpszResponseType;
	m_sUserID = lpszUserID;
	m_uiPort = uiPort;

	m_hThread = CreateThread(NULL, 0, ListenProc, this, 0, NULL);
	Sleep(100);

	return true;
}

void CIrcIdentServer::Stop()
{
	if( m_hThread )
	{
		m_socket.Close();
		if( WaitForSingleObject(m_hThread, 5000) != WAIT_OBJECT_0 && m_hThread )
		{
			TerminateThread(m_hThread, 1);
			CloseHandle(m_hThread);
			m_hThread = NULL;
		}
	}
}

void CIrcIdentServer::DoThread()
{
	m_socket.Listen();

	while( (bool)m_socket )
	{
		Socket s = m_socket.Accept();
		if( !s )
			break;

		char szBuf[1024];
		int cbRead = s.Receive((unsigned char*)szBuf, sizeof(szBuf)-1);
		if( cbRead <= 0 )
			continue;
		szBuf[cbRead] = '\0';

		// strip CRLF from query
		for(char* p = szBuf; *p && *p != '\r' && *p != '\n'; ++p)
			;
		*p = '\0';

		s.Send("%s : USERID : %s : %s\r\n", 
			szBuf, m_sResponseType.c_str(), m_sUserID.c_str());
		Sleep(500);
		s.Close();
	}

	m_socket.Close();
}

DWORD WINAPI CIrcIdentServer::ListenProc(LPVOID pparam)
{
	CIrcIdentServer* pThis = (CIrcIdentServer*)pparam;

	try { pThis->DoThread(); } catch( ... ) {}

	pThis->m_sResponseType = "";
	pThis->m_sUserID = "";
	pThis->m_uiPort = 0;

	CloseHandle(pThis->m_hThread);
	pThis->m_hThread = NULL;

	return 0;
}

////////////////////////////////////////////////////////////////////

CIrcDCCServer::CIrcDCCServer()
	: m_uiPort(0), m_hThread(NULL), m_bIsSender(false), 
		m_ulPartnerIP(0L), m_ulFileSize(0L),
		m_pSendFileDialog(NULL), m_ulStartTime(0L),
		m_ulBytesSent(0L), m_uiXferRate(0)
{
}

CIrcDCCServer::~CIrcDCCServer()
{
	Stop();
}

bool CIrcDCCServer::Start(
			const CString fileName,
			const CString directory,
			const CString partner,
			unsigned long ulPartnerIP,
			unsigned short uiPort,
			unsigned long ulFileSize,
			unsigned short uiXferRate,
			const bool bIsSender
			)
{
	//Clean up old send file dialog
	if (m_pSendFileDialog)
	{
		delete m_pSendFileDialog;
		m_pSendFileDialog = NULL;
	}

	if( m_socket )
		return false;

	//Setup the connection info vars
	m_fileName = fileName;
	m_directory = directory;
	m_partnerName = partner;
	m_ulPartnerIP = ulPartnerIP;
	m_uiPort = uiPort;
	m_ulFileSize = ulFileSize;
	m_bIsSender = bIsSender;
	m_ulBytesSent = 0L;
	m_uiXferRate = uiXferRate;

	//Remember when we started
	m_ulStartTime = GetTickCount();

	//Create a file transfer dialog
	m_pSendFileDialog = new CSendFileDialog(AfxGetMainWnd());
	if ( !m_pSendFileDialog->Create(IDD_FILEXFER, AfxGetMainWnd()) )
		return false;

	m_pSendFileDialog->m_BytesSent = "0 bytes";
	m_pSendFileDialog->m_ProgressFile.SetRange(0, 100);
	m_pSendFileDialog->m_ProgressFile.SetPos(0);
	sprintf(m_pSendFileDialog->m_Filesize.GetBuffer(32), "%lu bytes", m_ulFileSize);
	m_pSendFileDialog->m_FolderName = m_directory;
	m_pSendFileDialog->m_FileName = m_fileName;
	m_pSendFileDialog->m_RecvrName = m_partnerName;
	m_pSendFileDialog->m_TimeLeft = "Infinite";
	m_pSendFileDialog->m_XferRate = "0 bytes/sec";
	m_pSendFileDialog->m_XferStatus = "Waiting to Connect...";
	
	//Some fields in the 
	if (m_bIsSender)
	{
		m_pSendFileDialog->m_SentRecvd = "Sent:";
		m_pSendFileDialog->m_ToFrom = "To:";
	}
	else
	{
		m_pSendFileDialog->m_SentRecvd = "Recv'd:";
		m_pSendFileDialog->m_ToFrom = "From:";
	}

	//Show the dialog and tell it about this server
	m_pSendFileDialog->AssignDCCServer(this);
	m_pSendFileDialog->ShowWindow(SW_SHOW);

	//Fire off the thread 
	m_hThread = CreateThread(NULL, 0, ListenProc, this, 0, NULL);
	Sleep(100);

	return true;
}

void CIrcDCCServer::Stop(DWORD timeout)
{
	if( m_hThread )
	{
		m_socket.Close();
		if( WaitForSingleObject(m_hThread, timeout) != WAIT_OBJECT_0 && m_hThread )
		{
			TerminateThread(m_hThread, 1);
			CloseHandle(m_hThread);
			m_hThread = NULL;
		}
	}
}

void CIrcDCCServer::DoThreadRecv()
{
	char szBuf[8193];
	unsigned long cbRead, cbSent;
	float fracSent;
	unsigned long seconds, minutes, hours, rate;

	//Create an active socket to retrieve data from (close listening socket)
	InetAddr addr;
	addr.host = m_ulPartnerIP;
	addr.port = m_uiPort;
	m_socket.Connect(addr);
	if ( !m_socket ) return;

	//Tell user file transfer has started
	m_pSendFileDialog->m_XferStatus = "Recieving File...";

	//Create a file to write incoming data to
	CString filename = m_directory+m_fileName;
	FILE* fp = fopen(filename, "wb");
	if ( !fp ) return;

	//Start grabbing data blocks
	while(m_ulBytesSent < m_ulFileSize)
	{
		//Grab the next buffer
		cbRead = m_socket.Receive((unsigned char*)szBuf, sizeof(szBuf)-1);
		
		if( cbRead <= 0 ) continue;
		else szBuf[cbRead] = '\0';

		//Update the file transfer statistics
		m_ulBytesSent += cbRead;
		fracSent = float(m_ulBytesSent)/float(m_ulFileSize);
		seconds = (GetTickCount() - m_ulStartTime)/1000;
		rate = m_ulBytesSent / seconds;
		seconds = (unsigned long)(float(seconds)/fracSent);
		minutes = (seconds/60) % 60;
		hours = (seconds/3600);
		seconds = seconds % 60;

		//Update the file transfer window
		sprintf(m_pSendFileDialog->m_BytesSent.GetBuffer(64), "%lu bytes", m_ulBytesSent);
		sprintf(m_pSendFileDialog->m_TimeLeft.GetBuffer(32), "%luh%lum%lus", 
				hours, minutes, seconds);
		sprintf(m_pSendFileDialog->m_XferRate.GetBuffer(64), "%lubytes/sec", rate);
		m_pSendFileDialog->m_ProgressFile.SetPos(int(fracSent*100.F));

		//Write it to file
		fwrite(szBuf, cbRead, 1, fp);

		//Tell the sender how many bytes we received
		cbRead = htonl(cbRead);
		cbSent = 0;
		while ( cbSent <= 0 )
			cbSent = m_socket.Send((unsigned char *)&cbRead, 4);
	}

	//Close our data transfer socket
	m_socket.Close();

	//Close our file
	fclose(fp);
}

void CIrcDCCServer::DoThreadSend()
{
	char szBuf[8193];
	unsigned long cbSend, cbRead, numAck;
	float fracSent;
	unsigned long seconds, minutes, hours, rate;

	//Wait for someone to connect to us
	//on a pre-arranged port
	m_socket.Bind(InetAddr(m_uiPort));
	m_socket.Listen();

	//Create an active socket to write data from (close listening socket)
	Socket sock = m_socket.Accept();
	if ( !sock ) return;
	m_socket.Close();

	//Tell user file transfer has started
	m_pSendFileDialog->m_XferStatus = "Sending File...";

	//Create a file to read incoming data from
	CString filename = m_directory+"\\"+m_fileName;
	FILE* fp = fopen(filename, "rb");
	if ( !fp ) return;


	//Start grapping data blocks
	while( m_ulBytesSent < m_ulFileSize )
	{
		//Pull a data block from the file
		fread(szBuf, m_uiXferRate, 1, fp);

		//Send the next chunk out
		cbSend = 0;
		while (cbSend <= 0)
			cbSend = sock.Send((unsigned char *)szBuf, m_uiXferRate);	

		//Make sure chunk gets acknowledged
		cbRead = 0;
		while( cbRead <= 0)
			cbRead = m_socket.Receive((unsigned char*)&numAck, 4);

		//Update that statistics
		m_ulBytesSent += ntohl(numAck);
		fracSent = float(m_ulBytesSent)/float(m_ulFileSize);
		seconds = (GetTickCount() - m_ulStartTime)/1000;
		rate = m_ulBytesSent / seconds;
		seconds = (unsigned long)(float(seconds)/fracSent);
		minutes = (seconds/60) % 60;
		hours = (seconds/3600);
		seconds = seconds % 60;

		//Update the file transfer window
		sprintf(m_pSendFileDialog->m_BytesSent.GetBuffer(64), "%lu bytes", m_ulBytesSent);
		sprintf(m_pSendFileDialog->m_TimeLeft.GetBuffer(32), "%luh%lum%lus", 
				hours, minutes, seconds);
		sprintf(m_pSendFileDialog->m_XferRate.GetBuffer(64), "%lubytes/sec", rate);
		m_pSendFileDialog->m_ProgressFile.SetPos(int(fracSent*100.F));
	}

	//Close our data transfer socket
	sock.Close();

	//Close our file
	fclose(fp);
}

DWORD WINAPI CIrcDCCServer::ListenProc(LPVOID pparam)
{
	CIrcDCCServer* pThis = (CIrcDCCServer*)pparam;

	if (pThis->IsSender())
		try { pThis->DoThreadSend(); } catch( ... ) {}
	else
		try { pThis->DoThreadRecv(); } catch( ... ) {}

	CloseHandle(pThis->m_hThread);
	pThis->m_hThread = NULL;

	return 0;
}

////////////////////////////////////////////////////////////////////

CIrcMonitor::HandlersMap CIrcMonitor::m_handlers;
CIrcMonitor::IrcCommandsMapsListEntry CIrcMonitor::m_handlersMapsListEntry
	= { &CIrcMonitor::m_handlers, NULL };


CIrcMonitor::CIrcMonitor(CIrcSession& session)
	: m_session(session)
{
	m_xPost.SetMonitor(this);
}

CIrcMonitor::~CIrcMonitor()
{
}

void CIrcMonitor::OnIrcMessage(const CIrcMessage* pmsg)
{
	CIrcMessage* pMsgCopy = NULL;
	if( pmsg )
		pMsgCopy = new CIrcMessage(*pmsg);
	m_xPost.Post(0, (LPARAM)pMsgCopy);
}

void CIrcMonitor::OnCrossThreadsMessage(WPARAM wParam, LPARAM lParam)
{
	CIrcMessage* pmsg = (CIrcMessage*)lParam;

	OnIrcAll(pmsg);

	if( pmsg )
	{
		PfnIrcMessageHandler pfn = FindMethod(pmsg->sCommand.c_str());
		if( pfn )
		{
			// call member function. if it returns 'false',
			// call the default handling
			if( !(this->*pfn)(pmsg) )
				OnIrcDefault(pmsg);
		}
		else // handler not found. call default handler
			OnIrcDefault(pmsg);
		delete pmsg;
	}
	else
		OnIrcDisconnected();
}

CIrcMonitor::PfnIrcMessageHandler CIrcMonitor::FindMethod(const char* lpszName)
{
	// call the recursive version with the most derived map
	return FindMethod(GetIrcCommandsMap(), lpszName);
}

CIrcMonitor::PfnIrcMessageHandler CIrcMonitor::FindMethod(IrcCommandsMapsListEntry* pMapsList, const char* lpszName)
{
	HandlersMap::iterator it = pMapsList->pHandlersMap->find(lpszName);
	if( it != pMapsList->pHandlersMap->end() )
		return it->second; // found !
	else if( pMapsList->pBaseHandlersMap )
		return FindMethod(pMapsList->pBaseHandlersMap, lpszName); // try at base class
	return NULL; // not found in any map
}

////////////////////////////////////////////////////////////////////

DECLARE_IRC_MAP(CIrcDefaultMonitor, CIrcMonitor)

CIrcDefaultMonitor::CIrcDefaultMonitor(CIrcSession& session)
	: CIrcMonitor(session)
{
	IRC_MAP_ENTRY(CIrcDefaultMonitor, "NICK", OnIrc_NICK)
	IRC_MAP_ENTRY(CIrcDefaultMonitor, "PING", OnIrc_PING)
	IRC_MAP_ENTRY(CIrcDefaultMonitor, "002", OnIrc_YOURHOST)
	IRC_MAP_ENTRY(CIrcDefaultMonitor, "005", OnIrc_BOUNCE)
}

bool CIrcDefaultMonitor::OnIrc_NICK(const CIrcMessage* pmsg)
{
	if( (m_session.GetInfo().sNick == pmsg->prefix.sNick) && (pmsg->parameters.size() > 0) )
		m_session.m_info.sNick = pmsg->parameters[0];
	return false;
}

bool CIrcDefaultMonitor::OnIrc_PING(const CIrcMessage* pmsg)
{
	char szResponse[100];
	sprintf(szResponse, "PONG %s", pmsg->parameters[0].c_str());
	m_session << CIrcMessage(szResponse);
	return false;
}

bool CIrcDefaultMonitor::OnIrc_YOURHOST(const CIrcMessage* pmsg)
{
	static const char* lpszFmt = "Your host is %[^ \x5b,], running version %s";
	char szHostName[100], szVersion[100];
	if( sscanf(pmsg->parameters[1].c_str(), lpszFmt, &szHostName, &szVersion) > 0 )
		m_session.m_info.sServerName = szHostName;

	return false;
}

bool CIrcDefaultMonitor::OnIrc_BOUNCE(const CIrcMessage* pmsg)
{
	static const char* lpszFmt = "Try server %[^ ,], port %d";
	char szAltServer[100];
	int iAltPort = 0;
	if( sscanf(pmsg->parameters[1].c_str(), lpszFmt, &szAltServer, &iAltPort) == 2 )
	{
	}
	return false;
}
