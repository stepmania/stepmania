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
// CSendFileDialog dialog


CSendFileDialog::CSendFileDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSendFileDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSendFileDialog)
	m_BytesSent = _T("");
	m_Filesize = _T("");
	m_FolderName = _T("");
	m_FileName = _T("");
	m_RecvrName = _T("");
	m_TimeLeft = _T("");
	m_XferRate = _T("");
	m_XferStatus = _T("");
	m_SentRecvd = _T("");
	m_ToFrom = _T("");
	//}}AFX_DATA_INIT

	m_pDCCServer = NULL;
}


void CSendFileDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSendFileDialog)
	DDX_Control(pDX, IDC_PROGRESS1, m_ProgressFile);
	DDX_Text(pDX, IDC_BYTESSENT, m_BytesSent);
	DDX_Text(pDX, IDC_FILESIZE, m_Filesize);
	DDX_Text(pDX, IDC_FOLDERNAME, m_FolderName);
	DDX_Text(pDX, IDC_FILENAME, m_FileName);
	DDX_Text(pDX, IDC_RECVRNAME, m_RecvrName);
	DDX_Text(pDX, IDC_TIMELEFT, m_TimeLeft);
	DDX_Text(pDX, IDC_XFERRATE, m_XferRate);
	DDX_Text(pDX, IDC_XFERSTATUS, m_XferStatus);
	DDX_Text(pDX, IDC_SENTRECVD, m_SentRecvd);
	DDX_Text(pDX, IDC_TOFROM, m_ToFrom);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSendFileDialog, CDialog)
	//{{AFX_MSG_MAP(CSendFileDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSendFileDialog message handlers

void CSendFileDialog::OnCancel() 
{
	//Tell the DCC Server to abort as the file isn't transfering
	// or the user has beome impatient :)
	if (m_pDCCServer)
	{
		CIrcDCCServer* pServer = (CIrcDCCServer *)m_pDCCServer;
		pServer->Stop(0);
	}

	CDialog::OnCancel();
}
