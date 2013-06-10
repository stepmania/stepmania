// SendFileDialog.cpp : implementation file
//

#include "stdafx.h"
#include "smlobby.h"
#include "SendFileDialog.h"
#include "irc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSendFileDialog port management data
std::vector<unsigned short> CSendFileDialog::m_usedPorts;
std::vector<HANDLE> CSendFileDialog::m_hThread;
std::vector<CSendFileDialog *> CSendFileDialog::m_transferDialogs;

const unsigned short CSendFileDialog::kFirstPort = 1024;
const unsigned short CSendFileDialog::kLastPort = 5000;

/////////////////////////////////////////////////////////////////////////////
// CSendFileDialog dialog


CSendFileDialog::CSendFileDialog(DCCTransferInfo dccinfo, CWnd* pParent)
	: CDialog(CSendFileDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSendFileDialog)
	//}}AFX_DATA_INIT

	m_dccInfo = dccinfo;
}


void CSendFileDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSendFileDialog)
	DDX_Control(pDX, IDC_XFERSTATUS, m_XferStatus);
	DDX_Control(pDX, IDC_XFERRATE, m_XferRate);
	DDX_Control(pDX, IDC_TOFROM, m_ToFrom);
	DDX_Control(pDX, IDC_TIMELEFT, m_TimeLeft);
	DDX_Control(pDX, IDC_SENTRECVD, m_SentRecvd);
	DDX_Control(pDX, IDC_RECVRNAME, m_RecvrName);
	DDX_Control(pDX, IDC_FOLDERNAME, m_FolderName);
	DDX_Control(pDX, IDC_FILESIZE, m_Filesize);
	DDX_Control(pDX, IDC_FILENAME, m_FileName);
	DDX_Control(pDX, IDC_BYTESSENT, m_BytesSent);
	DDX_Control(pDX, IDC_PROGRESS1, m_ProgressFile);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSendFileDialog, CDialog)
	//{{AFX_MSG_MAP(CSendFileDialog)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSendFileDialog shared data management functions
unsigned short CSendFileDialog::MakePortReservation()
{
	//Assume the first port number is open
	unsigned short uiPort = kFirstPort;
	
	//if there are no used ports in the list use the first one
	if (m_usedPorts.size() <= 0)
		m_usedPorts.push_back(kFirstPort);

	//next see if the port range is saturated (pigeon hole principal)
	else if ((*m_usedPorts.end() - kFirstPort + 1) >= m_usedPorts.size())
		m_usedPorts.push_back(*m_usedPorts.end() + 1);

	//there must be a find a gap in the used port list
	//since the list is always sorted
	else
	{
		std::vector<unsigned short>::iterator iter;
		for (iter = m_usedPorts.begin(); iter < m_usedPorts.end(); iter++)
		{
			//oops, this port is being used, let's try the next one over
			if (*iter == uiPort) uiPort++;
			//looks like we found a gap, stop here
			else break;
		}

		//Insert our port number into the gap we just found
		m_usedPorts.insert(iter, uiPort);
	}

	return uiPort;
}

void CSendFileDialog::FreePort(unsigned short port)
{
	//Tell used port list that this port is now open
	std::vector<unsigned short>::iterator iter;
	for (iter = m_usedPorts.begin(); iter < m_usedPorts.end(); iter++)
	{
		//Find the port number, remove it
		if (*iter == port)
		{
			m_usedPorts.erase(iter);
			return;
		}
	}
}

void CSendFileDialog::AddTransferDialog(CSendFileDialog *ptr)
{
	if (ptr == NULL) return;

	m_transferDialogs.push_back(ptr);
}

void CSendFileDialog::FreeAnyCompletedTransfers()
{
	//remove all file transfer dialogs that are completed
	std::vector<CSendFileDialog *>::iterator iter;
	CSendFileDialog *dlg;
	DCCTransferInfo info;
	while (iter < m_transferDialogs.end())
	{
		dlg = *iter;
		info = dlg->m_dccInfo;
		if (info.m_ulBytesSent >= info.m_ulFileSize && 
			info.m_bIsConnected == false)
		{
			delete dlg;
			iter = m_transferDialogs.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSendFileDialog message handlers

void CSendFileDialog::OnTimer(UINT nIDEvent) 
{
	if (m_dccInfo.m_bIsSender) SendSomeData();
	else RecvSomeData();
	
	CDialog::OnTimer(nIDEvent);
}

void CSendFileDialog::OnCancel() 
{
	const unsigned int kTimerID = 1000;

	//See if a file pointer got left open
	if (m_dccInfo.m_fp != NULL)
		fclose(m_dccInfo.m_fp);

	//See if a socket got left open
	if (m_dccInfo.m_sock == true)
		m_dccInfo.m_sock.Close();

	//See if we need to free a port reservation
	if (m_dccInfo.m_bIsSender == true && m_dccInfo.m_uiPort >= kFirstPort)
		FreePort(m_dccInfo.m_uiPort);

	//Now we are no longer connected
	m_dccInfo.m_bIsConnected = false;

	//kill the timer
	KillTimer(kTimerID);

	CDialog::OnCancel();
}

int CSendFileDialog::Setup() 
{
	//Wait 50 ms between checking for data
	const unsigned int kPortCheckDelay = 50;
	const unsigned int kTimerID = 1000;

	//Depending on whether we are sending or recieving
	// we have to set up differently
	int ret;
	if (m_dccInfo.m_bIsSender) ret = SetupSend();
	else ret = SetupRecv();

	//See if we were able to set up a connection
	if (-1 == ret)
	{
		if (m_dccInfo.m_sock == true) m_dccInfo.m_sock.Close();
		if (m_dccInfo.m_fp != NULL) fclose(m_dccInfo.m_fp);
		return -1;
	}

	//Setup a timer which routinely checks up on the data
	if ( SetTimer(kTimerID, kPortCheckDelay, NULL) < 0 )  
		return -1;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CSendFileDialog dcc file transfer functions
int CSendFileDialog::SetupSend()
{
	const unsigned long k_ulTimeout = 30000;		//30 second timeout

	//Make sure we are sending...
	ASSERT (m_dccInfo.m_bIsSender);

	//Remember when we started
	m_dccInfo.m_ulStartTime = GetTickCount();

	//Setup the status info on the dialog
	m_BytesSent.SetWindowText("0 bytes");
	m_ProgressFile.SetRange(0, 100);
	m_ProgressFile.SetPos(0);
	m_FolderName.SetWindowText(LPCTSTR(m_dccInfo.m_directory));
	m_FileName.SetWindowText(LPCTSTR(m_dccInfo.m_fileName));
	m_RecvrName.SetWindowText(LPCTSTR(m_dccInfo.m_partnerName));
	m_TimeLeft.SetWindowText("Infinite");
	m_XferRate.SetWindowText("0 bytes/sec");
	m_XferStatus.SetWindowText("Waiting to Connect...");
	m_SentRecvd.SetWindowText("Sent:");
	m_ToFrom.SetWindowText("To:");

	char filesize[32];
	sprintf(filesize, "%lu bytes", m_dccInfo.m_ulFileSize);
	m_Filesize.SetWindowText(filesize);

	//Show the dialog and tell it about this server
	ShowWindow(SW_SHOW);

	//Tell user file transfer has started
	m_XferStatus.SetWindowText("Sending File...");

	//Create a file to write incoming data to
	CString filename = m_dccInfo.m_directory+m_dccInfo.m_fileName;
	m_dccInfo.m_fp = fopen(filename, "rb");
	
	//bail if we can't open the file
	if ( !m_dccInfo.m_fp ) return -1;

	//Create a port to listen on
	IPaddress ip;
	TCPsocket waitSock, newSock;
	SDLNet_ResolveHost(&ip, NULL, m_dccInfo.m_uiPort);
	waitSock = SDLNet_TCP_Open(&ip);
	if ( !waitSock ) return -1;

	//Wait for someone to connect to us
	unsigned long ulTime;
	do
	{
		ulTime = GetTickCount() - m_dccInfo.m_ulStartTime;
		newSock = SDLNet_TCP_Accept(waitSock);
	} while ( !newSock && ulTime < k_ulTimeout);

	//Make sure the socket is valid
	if ( !newSock ) return -1;

	//Now data can be sent on this socket
	m_dccInfo.m_sock = Socket(newSock);
	m_dccInfo.m_bIsConnected = true;

	return 0;
}

int CSendFileDialog::SetupRecv()
{
	//Make sure we are not sending...
	ASSERT(!m_dccInfo.m_bIsSender);

	//Remember when we started
	m_dccInfo.m_ulStartTime = GetTickCount();

	//Setup the status info on the dialog
	m_BytesSent.SetWindowText("0 bytes");
	m_ProgressFile.SetRange(0, 100);
	m_ProgressFile.SetPos(0);
	m_FolderName.SetWindowText(LPCTSTR(m_dccInfo.m_directory));
	m_FileName.SetWindowText(LPCTSTR(m_dccInfo.m_fileName));
	m_RecvrName.SetWindowText(LPCTSTR(m_dccInfo.m_partnerName));
	m_TimeLeft.SetWindowText("Infinite");
	m_XferRate.SetWindowText("0 bytes/sec");
	m_XferStatus.SetWindowText("Waiting to Connect...");
	m_SentRecvd.SetWindowText("Recv'd:");
	m_ToFrom.SetWindowText("From:");

	char filesize[32];
	sprintf(filesize, "%lu bytes", m_dccInfo.m_ulFileSize);
	m_Filesize.SetWindowText(filesize);

	//Show the dialog and tell it about this server
	ShowWindow(SW_SHOW);

	//Create an active socket to retrieve data from (close listening socket)
	InetAddr addr;
	addr.host = htonl(m_dccInfo.m_ulPartnerIP);
	addr.port = htons(m_dccInfo.m_uiPort);
	m_dccInfo.m_sock.Connect(addr);
	
	//bail if we can't open the socket
	if ( !m_dccInfo.m_sock ) return -1;
	else m_dccInfo.m_bIsConnected = true;

	//Tell user file transfer has started
	m_XferStatus.SetWindowText("Recieving File...");

	//Create a file to write incoming data to
	CString filename = m_dccInfo.m_directory+m_dccInfo.m_fileName;
	m_dccInfo.m_fp = fopen(filename, "wb");
	
	//bail if we can't open the file
	if ( !m_dccInfo.m_fp ) return -1;
	
	return 0;
}

void CSendFileDialog::SendSomeData()
{
	//Make sure the socket and the file descriptor are valid
	if (m_dccInfo.m_fp == NULL || m_dccInfo.m_sock != true) return;

	//Make sure we're sending
	ASSERT (m_dccInfo.m_bIsSender);

	char szBuf[8193];
	unsigned long cbSend, cbRead, numAck;
	float fracSent;
	unsigned long seconds, minutes, hours, rate;


	//See if the file is done transfering
	if (m_dccInfo.m_ulBytesSent < m_dccInfo.m_ulFileSize)
	{
		//Pull a data block from the file
		cbRead = fread(szBuf, sizeof(char), m_dccInfo.m_uiXferRate, m_dccInfo.m_fp);

		//Send the next chunk out
		cbSend = 0;
		while (cbSend <= 0)
			cbSend = m_dccInfo.m_sock.Send((unsigned char *)szBuf, cbRead);
		
		//Update sent byte count
		m_dccInfo.m_ulBytesSent += cbSend;

		//Make sure chunk gets acknowledged
		cbRead = 0;
		while( cbRead <= 0)
			cbRead = m_dccInfo.m_sock.Receive((unsigned char*)&numAck, 4);

		//Make sure numAck matches current amount of data sent
		if (m_dccInfo.m_ulBytesSent != ntohl(numAck))
		{
			OnCancel();
			return;
		}

		//Update that statistics
		fracSent = float(m_dccInfo.m_ulBytesSent)/float(m_dccInfo.m_ulFileSize);
		seconds = seconds = (GetTickCount() - m_dccInfo.m_ulStartTime)/1000;
		rate = (seconds > 0) ? m_dccInfo.m_ulBytesSent / seconds : 0;
		seconds = (unsigned long)(float(seconds)/fracSent);
		minutes = (seconds/60) % 60;
		hours = (seconds/3600);
		seconds = seconds % 60;

		//Update the file transfer window
		sprintf(szBuf, "%lu bytes", m_dccInfo.m_ulBytesSent);
		m_BytesSent.SetWindowText(szBuf);
		sprintf(szBuf, "%luh %lum %lus", hours, minutes, seconds);
		m_TimeLeft.SetWindowText(szBuf);
		sprintf(szBuf, "%lu bytes/sec", rate);
		m_XferRate.SetWindowText(szBuf);
		m_ProgressFile.SetPos(int(fracSent*100.F));
	}
	else
	{
		//OK, we're all done. Close up shop.
		OnCancel();
	}
}

void CSendFileDialog::RecvSomeData()
{
	//Make sure socket and file descriptor are valid
	if (m_dccInfo.m_sock != true || m_dccInfo.m_fp == NULL) return;

	//Make sure were not sending
	ASSERT (!m_dccInfo.m_bIsSender);

	char szBuf[8193];
	unsigned long cbRead, cbSent, replyTotal;
	float fracSent;
	unsigned long seconds, minutes, hours, rate;


	//Start grabbing data blocks
	if (m_dccInfo.m_ulBytesSent < m_dccInfo.m_ulFileSize)
	{
		//Grab the next buffer
		cbRead = m_dccInfo.m_sock.Receive((unsigned char*)szBuf, sizeof(szBuf)-1);
		
		//Bail if there is nothing to read
		if( cbRead <= 0 ) return;
		else szBuf[cbRead] = '\0';

		//Write it to file
		fwrite(szBuf, cbRead, 1, m_dccInfo.m_fp);

		//update byte count
		m_dccInfo.m_ulBytesSent += cbRead;

		//Update the file transfer statistics
		fracSent = float(m_dccInfo.m_ulBytesSent)/float(m_dccInfo.m_ulFileSize);
		seconds = (GetTickCount() - m_dccInfo.m_ulStartTime)/1000;
		rate = (seconds > 0) ? m_dccInfo.m_ulBytesSent / seconds : 0;
		seconds = (unsigned long)(float(seconds)/fracSent);
		minutes = (seconds/60) % 60;
		hours = (seconds/3600);
		seconds = seconds % 60;

		//Update the file transfer window
		sprintf(szBuf, "%lu bytes", m_dccInfo.m_ulBytesSent);
		m_BytesSent.SetWindowText(szBuf);
		sprintf(szBuf, "%luh %lum %lus", hours, minutes, seconds);
		m_TimeLeft.SetWindowText(szBuf);
		sprintf(szBuf, "%lu bytes/sec", rate);
		m_XferRate.SetWindowText(szBuf);
		m_ProgressFile.SetPos(int(fracSent*100.F));

		//Tell the sender how many bytes we received
		replyTotal = htonl(m_dccInfo.m_ulBytesSent);

		//Send out response
		cbSent = m_dccInfo.m_sock.Send((unsigned char *)&replyTotal, 4);
	}
	else
	{
		OnCancel();
		return;
	}
}
