// ShowComment.cpp : implementation file
//

#define CO_EXIST_WITH_MFC
#include "global.h"
#include "stdafx.h"
#include "smpackage.h"
#include "ShowComment.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ShowComment dialog


ShowComment::ShowComment(CWnd* pParent /*=NULL*/)
	: CDialog(ShowComment::IDD, pParent)
	, m_bDontShow(FALSE)
{
	//{{AFX_DATA_INIT(ShowComment)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void ShowComment::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ShowComment)
	DDX_Control(pDX, IDC_EDIT, m_edit);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_DONTSHOW, m_bDontShow);
}


BEGIN_MESSAGE_MAP(ShowComment, CDialog)
	//{{AFX_MSG_MAP(ShowComment)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ShowComment message handlers

BOOL ShowComment::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	m_edit.SetWindowText( m_sComment );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
