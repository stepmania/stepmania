// smlobbyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smlobby.h"
#include "smlobbyDlg.h"
#include "RageUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#include "ConnectDlg.h"


/////////////////////////////////////////////////////////////////////////////
// global objects
static const NetworkInit g_wsInit;
static const unsigned short FILE_XFER_RATE = 4096;

irc::CIrcSession g_ircSession;
irc::CIrcDCCServer g_DCCServer;


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSmlobbyDlg dialog

CSmlobbyDlg::CSmlobbyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSmlobbyDlg::IDD, pParent), irc::CIrcDefaultMonitor(g_ircSession)
{
	//{{AFX_DATA_INIT(CSmlobbyDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSmlobbyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSmlobbyDlg)
	DDX_Control(pDX, IDC_COMBO_MUSIC, m_comboMusic);
	DDX_Control(pDX, IDC_EDIT_GAME_NAME, m_editGameName);
	DDX_Control(pDX, IDC_EDIT_GAME_INFO, m_editGameInfo);
	DDX_Control(pDX, IDC_LIST_USERS, m_listUsers);
	DDX_Control(pDX, IDC_EDIT_ENTRY, m_editEntry);
	DDX_Control(pDX, IDC_EDIT_CHAT_MESSAGES, m_editChatMessages);
	DDX_Control(pDX, IDC_LIST_GAMES, m_listGames);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSmlobbyDlg, CDialog)
	//{{AFX_MSG_MAP(CSmlobbyDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_QUERYDRAGICON()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_LBN_DBLCLK(IDC_LIST_GAMES, OnDblclkListGames)
	ON_LBN_SELCHANGE(IDC_LIST_GAMES, OnSelchangeListGames)
	ON_BN_CLICKED(IDC_BUTTON_CREATE_GAME, OnButtonCreateGame)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSmlobbyDlg message handlers

BOOL CSmlobbyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//Tell windows we have no default push button
	SetDefID(-1); 

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	
	//Add IRC reply call backs
	IRC_MAP_ENTRY(CSmlobbyDlg, "JOIN", OnIrc_JOIN)
	IRC_MAP_ENTRY(CSmlobbyDlg, "KICK", OnIrc_KICK)
	IRC_MAP_ENTRY(CSmlobbyDlg, "MODE", OnIrc_MODE)
	IRC_MAP_ENTRY(CSmlobbyDlg, "NICK", OnIrc_NICK)
	IRC_MAP_ENTRY(CSmlobbyDlg, "PART", OnIrc_PART)
	IRC_MAP_ENTRY(CSmlobbyDlg, "PRIVMSG", OnIrc_PRIVMSG)
	IRC_MAP_ENTRY(CSmlobbyDlg, "DCC", OnIrc_DCC_SEND)
	IRC_MAP_ENTRY(CSmlobbyDlg, "002", OnIrc_YOURHOST)
	IRC_MAP_ENTRY(CSmlobbyDlg, "321", OnIrc_RPL_LISTSTART)
	IRC_MAP_ENTRY(CSmlobbyDlg, "322", OnIrc_RPL_LIST)
	IRC_MAP_ENTRY(CSmlobbyDlg, "323", OnIrc_IgnoreMesg)			//RPL_LISTEND
	IRC_MAP_ENTRY(CSmlobbyDlg, "331", OnIrc_IgnoreMesg)			//RPL_NOTOPIC
	IRC_MAP_ENTRY(CSmlobbyDlg, "332", OnIrc_RPL_TOPIC)
	IRC_MAP_ENTRY(CSmlobbyDlg, "353", OnIrc_RPL_NAMREPLY)
	IRC_MAP_ENTRY(CSmlobbyDlg, "366", OnIrc_IgnoreMesg)			//RPL_ENDOFNAMES
	IRC_MAP_ENTRY(CSmlobbyDlg, "376", OnIrc_IgnoreMesg)			//RPL_ENDOFMOTD

	
	//Initialize IRC Server connection dialog
	CConnectDlg dlg;

	dlg.m_sServer = _T("128.208.46.94");
	dlg.m_uiPort = 6667;
	dlg.m_sNick = _T("smuser");
	dlg.m_sUserID = _T("vm");
	dlg.m_sFullName = _T("vm");

	if( dlg.DoModal() != IDOK )
		return TRUE;

	// set this document object as the session's monitor
	g_ircSession.AddMonitor(this);

	CIrcSessionInfo si;
	si.sServer = dlg.m_sServer;
	si.iPort = dlg.m_uiPort;
	si.sNick = dlg.m_sNick;
	si.sUserID = dlg.m_sUserID;
	si.sFullName = dlg.m_sFullName;
	si.sPassword = dlg.m_sPassword;
	si.bIdentServer = true;
	si.iIdentServerPort = 113;
	si.sIdentServerType = "UNIX";
	si.sCurrentChatRoom = "";

	bool m_bOk = g_ircSession.Connect(si);
	if( !m_bOk )
		return FALSE;
		

	//
	// populate drop-down list of songs
	//
	CString sDir = "Songs";	// search only in "Songs" directory in program folder

	// Find all group directories in "Songs" folder
	CStringArray arrayGroupDirs;
	GetDirListing( sDir+"\\*.*", arrayGroupDirs, true );
	SortCStringArray( arrayGroupDirs );
	
	for( unsigned i=0; i< arrayGroupDirs.GetSize(); i++ )	// for each dir in /Songs/
	{
		CString sGroupDirName = arrayGroupDirs[i];

		if( 0 == stricmp( sGroupDirName, "cvs" ) )	// the directory called "CVS"
			continue;		// ignore it

		// Find all Song folders in this group directory
		CStringArray arraySongDirs;
		GetDirListing( ssprintf("%s\\%s\\*.*", sDir, sGroupDirName), arraySongDirs, true );
		SortCStringArray( arraySongDirs );

		for( unsigned j=0; j< arraySongDirs.GetSize(); j++ )	// for each song dir
		{
			CString sSongDirName = arraySongDirs[j];

			if( 0 == stricmp( sSongDirName, "cvs" ) )	// the directory called "CVS"
				continue;		// ignore it

			CString sSongDir = ssprintf("%s\\%s\\%s", sDir, sGroupDirName, sSongDirName);

			m_comboMusic.AddString( sSongDir );
		}
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSmlobbyDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSmlobbyDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSmlobbyDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


bool CSmlobbyDlg::OnIrc_YOURHOST(const CIrcMessage* pmsg)
{
	CIrcDefaultMonitor::OnIrc_YOURHOST(pmsg);

	//query the server for a list of chat rooms
	g_ircSession << irc::CIrcMessage(CString("list"));

	return false;
}

bool CSmlobbyDlg::OnIrc_NICK(const CIrcMessage* pmsg)
{
	CIrcDefaultMonitor::OnIrc_NICK(pmsg);

	if( pmsg->prefix.sNick == m_session.GetInfo().sNick && (pmsg->parameters.size() > 0) )
	{
	}

	return false;
}

bool CSmlobbyDlg::OnIrc_PRIVMSG(const CIrcMessage* pmsg)
{
	//See if were being sent a DCC command
	if (OnIrc_DCC_RECV(pmsg)) 
		return true;
	
	UpdateChatMessages( pmsg );

	return true;
}

bool CSmlobbyDlg::OnIrc_DCC_SEND(const CIrcMessage *pmsg)
{
	//incoming:
	// /DCC SEND <recipient> 
	//outgoing: 
	// PRIVMSG <recipient> :<0x01>DCC SEND <filename> <ipaddress> <port> <filesize><0x01>
	const kRecptParm = 1;

	//Make sure we have a parameter
	if ( pmsg->parameters.size() < (kRecptParm + 1) ) 
		return false;

	//Make sure we're not trying to send a file to ourselves (people can be silly :p)
	CString partnerName = pmsg->parameters[kRecptParm].c_str();
	if ( partnerName == g_ircSession.GetInfo().sNick.c_str())
		return false;

	//Make sure partner is currently in the room
	if (LB_ERR == m_listUsers.FindString(-1, partnerName))
		return false;

	//open a file dialog box to get the file the user wants to send
	CFileDialog fileDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER, 
							NULL, AfxGetMainWnd());
	if (IDOK != fileDialog.DoModal())
		return false;

	//Setup retrieval params
	IPaddress ipaddr;
	char szHostName[256];
	gethostname(szHostName, 256);
	SDLNet_ResolveHost(&ipaddr, szHostName, 0);

	//Get filename and directory
	CString filename = fileDialog.GetFileName();
	CString fullpath = fileDialog.GetPathName();
	CString dirname = fullpath.Left(fullpath.GetLength() - filename.GetLength());

	//Place all of the connection info into a dcc info structure
	irc::CIrcDCCServer::DCCTransferInfo dccInfo;
	dccInfo.m_bIsSender = true;
	dccInfo.m_fileName = filename;
	dccInfo.m_directory = dirname;
	dccInfo.m_partnerName = partnerName;
	dccInfo.m_uiPort = g_DCCServer.MakePortReservation();
	dccInfo.m_uiXferRate = FILE_XFER_RATE;
	dccInfo.m_ulFileSize = GetFileSizeInBytes(fullpath);
	dccInfo.m_ulPartnerIP = ipaddr.host;

	//Create a dcc command to send to the user to which our file is going
	char szDCCString[512];
	sprintf(szDCCString, "PRIVMSG %s :\001DCC SEND %s %ul %u %ul\001",
			dccInfo.m_partnerName, dccInfo.m_fileName, ipaddr.host,
			dccInfo.m_uiPort, dccInfo.m_ulFileSize);

	//Send of the dcc command to the other party
	g_ircSession << irc::CIrcMessage(szDCCString);

	//We now have enough info to fire off a thread that will
	//wait for the other party to connect to us and then
	//transfer the file to them
	g_DCCServer.Start(dccInfo);  

	return true;
}

bool CSmlobbyDlg::OnIrc_DCC_RECV(const CIrcMessage *pmsg)
{
	//incoming::
	// PRIVMSG <recipient> :<0x01>DCC SEND <filename> <ipaddress> <port> <filesize><0x01>

	//Make sure we have a parameter
	if ( pmsg->parameters.size() < 1 ) 
		return false;

	//Make sure it's a DCC send
	if ( std::string::npos == pmsg->parameters[1].find("\001DCC SEND") ) 
		return false;
	
	//Make sure we can read out the dcc send fields
	const char* pszRawString = pmsg->parameters[1].c_str();
	char szFilename[256];
	unsigned long ulPartnerIP;
	unsigned short uiPartnerPort;
	unsigned long ulFileSize;

	//Snag the dcc info fields from the message string
	if ( 4 != sscanf(pszRawString, "\001DCC SEND %255s %lu %u %lu", 
				&szFilename, &ulPartnerIP, &uiPartnerPort, &ulFileSize) )
		return false;

	//Open a dialog box so user can decide where to save file
	CString pathname = SelectFolder();
	if (pathname.GetLength() <= 1)
		return false;

	//Assemble all of this info into a dcc info structure
	irc::CIrcDCCServer::DCCTransferInfo dccInfo;
	dccInfo.m_bIsSender = false;
	dccInfo.m_directory = pathname;
	dccInfo.m_fileName = szFilename;
	dccInfo.m_partnerName = pmsg->prefix.sNick.c_str();
	dccInfo.m_uiPort = uiPartnerPort;
	dccInfo.m_uiXferRate = FILE_XFER_RATE;
	dccInfo.m_ulFileSize = ulFileSize;
	dccInfo.m_ulPartnerIP = ulPartnerIP;

	//We now have enough info to fire off a thread that downloads the file
	g_DCCServer.Start(dccInfo);  

	return true;
}

bool CSmlobbyDlg::OnIrc_JOIN(const CIrcMessage* pmsg)
{
	//Put update on message window
	UpdateChatMessages( pmsg );

	//Remember which chat room we joined
	String chatRoomName(pmsg->parameters[0].c_str());
	g_ircSession.GetInfo().sCurrentChatRoom = chatRoomName;

	//query the server for a list of names in this room
	g_ircSession << irc::CIrcMessage(CString("names ") + CString(chatRoomName.c_str()));

	//query the server for a list of rooms (could have changed)
	g_ircSession << irc::CIrcMessage(CString("list"));

	return true;
}

bool CSmlobbyDlg::OnIrc_PART(const CIrcMessage* pmsg)
{
	if( !pmsg->prefix.sNick.length() || pmsg->prefix.sNick == m_session.GetInfo().sNick )
		return false;

	//make sure to reset the names lists since were leaving current room
	m_listUsers.ResetContent();

	//forget which chat room we joined
	g_ircSession.GetInfo().sCurrentChatRoom = "";

	//query the server for a list of chat rooms
	g_ircSession << irc::CIrcMessage(CString("list"));

	UpdateChatMessages( pmsg );

	return true;
}

bool CSmlobbyDlg::OnIrc_KICK(const CIrcMessage* pmsg)
{
	if( !pmsg->prefix.sNick.length() )
		return false;

	UpdateChatMessages( pmsg );

	return true;
}

bool CSmlobbyDlg::OnIrc_MODE(const CIrcMessage* pmsg)
{
	if( !pmsg->prefix.sNick.length() )
		return false;
	if( pmsg->prefix.sNick == m_session.GetInfo().sNick )
		return false;

	UpdateChatMessages( pmsg );

	return true;
}

bool CSmlobbyDlg::OnIrc_RPL_LISTSTART(const CIrcMessage *pmsg)
{
	//Were going to be getting a new list of rooms
	//so clear out the old ones
	m_listGames.ResetContent();

	return true;
}

bool CSmlobbyDlg::OnIrc_RPL_LIST(const CIrcMessage *pmsg)
{
	//Add the current game to the list
	//msg Format: "<channel> <# visible> :<topic>"
	String name = pmsg->parameters[1] + " (" + pmsg->parameters[2] + ")";
	m_listGames.AddString(name.c_str());

	return true;
}

bool CSmlobbyDlg::OnIrc_RPL_TOPIC(const CIrcMessage *pmsg)
{
	//Clean out the game info box
	m_editGameInfo.SetSel(0, -1);
	m_editGameInfo.Clear();

	//Put the info for a game up in the game info box
	//msg Format: "<channel> :<topic>"
	//topic format: "GameName\nserverIP\nServerIRCName\nSongHash"
	std::vector<String> vecGameParams;
	const char* p1 = pmsg->parameters[2].c_str();
	const char* p2 = p1;

	//Extract all of the topic params
	while( *p1 )
	{
		// seek end of name
		while( *p2 && *p2!=' ' ) p2++;

		//add name to the list
		vecGameParams.push_back(String(p1, p2 - p1));
		
		// eat white space
		while( *p2 && *p2==' ' ) p2++;
		p1 = p2;
	}

	//somehow we lost some game info
	if (vecGameParams.size() != 4) return false;

	//Display game info 
	m_editGameInfo.SetSel(-1, 0);
	m_editGameInfo.ReplaceSel(("Game Name: " + vecGameParams[0] + "\n").c_str());
	m_editGameInfo.ReplaceSel(("Server IP: " + vecGameParams[1] + "\n").c_str());
	m_editGameInfo.ReplaceSel(("Host IRC Name: " + vecGameParams[2] + "\n").c_str());
	m_editGameInfo.ReplaceSel(("Song Hash: " + vecGameParams[3] + "\n").c_str());

	return true;
}

bool CSmlobbyDlg::OnIrc_RPL_NAMREPLY(const CIrcMessage *pmsg)
{
	//Reset the names list
	m_listUsers.ResetContent();

	//now dump all of the users into the user list
	//msg format: "<channel> :[[@|+]<nick> [[@|+]<nick> [...]]]"
	const char* p1 = pmsg->parameters[3].c_str();
	const char* p2 = p1;

	//keep trying to read off names while the list isn't null
	while( *p1 )
	{
		// seek end of name
		while( *p2 && !isspace(*p2) ) p2++;

		//add name to the list
		m_listUsers.AddString(String(p1, p2 - p1).c_str());
		
		// eat white space
		while( *p2 && isspace(*p2) ) p2++;
		p1 = p2;
	}

	return true;
}

void CSmlobbyDlg::OnIrcDefault(const CIrcMessage* pmsg)
{
	CIrcDefaultMonitor::OnIrcDefault(pmsg);

	if( pmsg && m_session.GetInfo().sServerName.length() )
	{
		UpdateChatMessages( pmsg );
	}
}

void CSmlobbyDlg::OnIrcDisconnected()
{
	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_FILE_CLOSE);
}


void CSmlobbyDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	g_ircSession.Disconnect();
	g_ircSession.RemoveMonitor(this);

	//Close the parent application
	AfxGetApp()->ExitInstance();
}


void CSmlobbyDlg::UpdateChatMessages( const CIrcMessage* p )
{
	if( p )
	{
		m_editChatMessages.SetSel(-1, 0);
		m_editChatMessages.SendMessage(EM_SCROLLCARET);

		#ifdef _DEBUG
		m_editChatMessages.ReplaceSel((p->sCommand + " ").c_str());
		#endif

		if (p->sCommand == "NOTICE")
		{
			String param = p->parameters[1];

			const char *c_param = param.c_str();
			p=p;
		}

		if( p->prefix.sNick.length() )
		{
			m_editChatMessages.ReplaceSel(("<" + p->prefix.sNick + "> ").c_str());
		}

		for(int i=1; i < p->parameters.size(); i++)
		{
			m_editChatMessages.ReplaceSel((p->parameters[i] + " ").c_str());
		}
		m_editChatMessages.ReplaceSel("\r\n");
	}
}


void CSmlobbyDlg::OnDblclkListGames() 
{
	//Get the game name that was currently selected
	int nIndex = m_listGames.GetCurSel();
	CString strGameName;

	if (nIndex != LB_ERR)
	{
		m_listGames.GetText(nIndex, strGameName);
		
		//Tell the server that we wanted to join a chat room
		g_ircSession << irc::CIrcMessage(CString("join ") + strGameName);
	}
}

void CSmlobbyDlg::OnSelchangeListGames() 
{
	//Get the game name that was currently selected
	int nIndex = m_listGames.GetCurSel();
	char szGameName[128];
	int nDummy;

	if (nIndex != LB_ERR)
	{
		m_listGames.GetText(nIndex, szGameName);
		sscanf(szGameName, "%s (%d)", &szGameName, &nDummy);
		
		//Tell the server that we wanted to join a chat room
		g_ircSession << irc::CIrcMessage(CString("topic ") + CString(szGameName));
	}
}

void CSmlobbyDlg::OnButtonCreateGame() 
{
	//Strip all non alphanumeric characters from the name
	// so irc will be ok with it as a chat room name
	CString gameName;
	char szRawGameName[256];
	m_editGameName.GetLine(0, szRawGameName, 255);

	for (int i = 0; i < strlen(szRawGameName); i++)
	{
		if ( isalnum(szRawGameName[i]) )
			gameName += szRawGameName[i];
	}

	//Make sure user has given us a game name
	if (gameName.GetLength() <= 0)
	{
		MessageBox("You need some letters or numbers in your name!");
		return;
	}

	//Make sure no one else has this name
	if (!IsUniqueGameName(gameName))
	{
		MessageBox("Game already exists with that name!");		
		return;
	}

	//Make sure a song is selected
	int iIndex = m_comboMusic.GetCurSel();
	if( iIndex <= 0 )
	{
		MessageBox("Need to select a song to play!");
		return;
	}

	//Tell the server that we wanted to create a chat room
	g_ircSession << irc::CIrcMessage(CString("join ") + "#" + gameName);
	
	//Get the IP address of our machine in network byte order
	IPaddress ipaddr;
	char szHostName[256];
	gethostname(szHostName, 256);
	SDLNet_ResolveHost(&ipaddr, szHostName, 0);
	unsigned char *ip = (unsigned char *)&(ipaddr.host);
	char host_ip_str[20];
	sprintf(host_ip_str, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

	//Insert song name/hash code here
	CString sSongDir;
	m_comboMusic.GetLBText( iIndex, sSongDir );
	unsigned long hash = GetHashForDirectory( sSongDir );

	//Now tell the server our game info
	CString sIrcMessage = ssprintf( "topic #%s :%s %s %s %u",
		gameName,
		gameName,
		host_ip_str,
		g_ircSession.GetInfo().sNick.c_str(),
		hash );

	g_ircSession << irc::CIrcMessage(sIrcMessage);
}

bool CSmlobbyDlg::IsUniqueGameName(const CString GameName)
{
	CString aName;

	//Search list of games to make sure ours is unique
	for (int i = 0; i < m_listGames.GetCount(); i++)
	{
		m_listGames.GetText(i, aName);
		if (aName == GameName)
			return false;
	}

	return true;
}

CString CSmlobbyDlg::SelectFolder()
{
	char szTitle[]                 = "Select a Folder";
	char szDisplayName[MAX_PATH] = "";
	char szPath[MAX_PATH] = "";
	BROWSEINFO bi;

	szPath[0] = '\0';

	bi.hwndOwner      = m_hWnd;               // Handle of the owner window
	bi.pidlRoot       = NULL;                 // Desktop folder is used
	bi.lpszTitle      = szTitle;              // Title of the dialog box
	bi.pszDisplayName = szDisplayName;        // Buffer for selected folder name
	bi.ulFlags        = BIF_RETURNONLYFSDIRS; // Only returns file system directories
	bi.lpfn           = NULL;
	bi.lParam         = 0;
	
	LPITEMIDLIST pItemIDList = SHBrowseForFolder(&bi);

	if (pItemIDList) {
		if (SHGetPathFromIDList(pItemIDList, szPath)) {
			SetWindowText(szPath);
		}

		// Avoid memory leaks by deleting the PIDL using the shell's task allocator
		IMalloc* pMalloc;
		if (SHGetMalloc(&pMalloc) != NOERROR) {
			TRACE("Failed to get pointer to shells task allocator");
			return CString(szPath);
		}
		pMalloc->Free(pItemIDList);
		if (pMalloc)
			pMalloc->Release();
	}

	return CString(szPath);
}
