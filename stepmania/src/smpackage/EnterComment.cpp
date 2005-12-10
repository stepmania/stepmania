// EnterComment.cpp : implementation file
//

#define CO_EXIST_WITH_MFC
#include "global.h"
#include "stdafx.h"
#include "smpackage.h"
#include "EnterComment.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// EnterComment dialog


EnterComment::EnterComment(CWnd* pParent /*=NULL*/)
	: CDialog(EnterComment::IDD, pParent)
	, m_bDontAsk(FALSE)
{
	//{{AFX_DATA_INIT(EnterComment)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void EnterComment::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EnterComment)
	DDX_Control(pDX, IDC_EDIT, m_edit);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_DONTASK, m_bDontAsk);
}


BEGIN_MESSAGE_MAP(EnterComment, CDialog)
	//{{AFX_MSG_MAP(EnterComment)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EnterComment message handlers

void EnterComment::OnOK() 
{
	// TODO: Add extra validation here

	m_edit.GetWindowText( m_sEnteredComment );

	CDialog::OnOK();
}
