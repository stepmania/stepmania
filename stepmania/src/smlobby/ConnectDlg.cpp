// ConnectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource2.h"
#include "ConnectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConnectDlg dialog


CConnectDlg::CConnectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConnectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConnectDlg)
	m_sFullName = _T("");
	m_sNick = _T("");
	m_sPassword = _T("");
	m_uiPort = 0;
	m_sServer = _T("");
	m_sUserID = _T("");
	//}}AFX_DATA_INIT
}


void CConnectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConnectDlg)
	DDX_Text(pDX, IDC_FULLNAME, m_sFullName);
	DDX_Text(pDX, IDC_NICK, m_sNick);
	DDX_Text(pDX, IDC_PASSWORD, m_sPassword);
	DDX_Text(pDX, IDC_PORT, m_uiPort);
	DDX_Text(pDX, IDC_SERVER, m_sServer);
	DDX_Text(pDX, IDC_USERID, m_sUserID);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConnectDlg, CDialog)
	//{{AFX_MSG_MAP(CConnectDlg)
	ON_EN_CHANGE(IDC_FULLNAME, OnChange)
	ON_EN_CHANGE(IDC_NICK, OnChange)
	ON_EN_CHANGE(IDC_PASSWORD, OnChange)
	ON_EN_CHANGE(IDC_PORT, OnChange)
	ON_EN_CHANGE(IDC_SERVER, OnChange)
	ON_EN_CHANGE(IDC_USERID, OnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConnectDlg message handlers

void CConnectDlg::OnChange() 
{
	UpdateButtons();
}

void CConnectDlg::UpdateButtons()
{
	GetDlgItem(IDOK)->EnableWindow(
							GetDlgItem(IDC_FULLNAME)->GetWindowTextLength() > 0 &&
							GetDlgItem(IDC_NICK)->GetWindowTextLength() > 0 &&
							GetDlgItem(IDC_SERVER)->GetWindowTextLength() > 0 &&
							GetDlgItem(IDC_USERID)->GetWindowTextLength() > 0 &&
							GetDlgItemInt(IDC_PORT) != 0
						);
}

BOOL CConnectDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	UpdateButtons();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
