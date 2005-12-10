// EnterName.cpp : implementation file
//

#define CO_EXIST_WITH_MFC
#include "global.h"
#include "stdafx.h"
#include "smpackage.h"
#include "EnterName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// EnterName dialog


EnterName::EnterName(CWnd* pParent /*=NULL*/)
	: CDialog(EnterName::IDD, pParent)
{
	//{{AFX_DATA_INIT(EnterName)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void EnterName::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EnterName)
	DDX_Control(pDX, IDC_EDIT, m_edit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(EnterName, CDialog)
	//{{AFX_MSG_MAP(EnterName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EnterName message handlers

void EnterName::OnOK() 
{
	// TODO: Add extra validation here
	
	m_edit.GetWindowText( m_sEnteredName );

	CDialog::OnOK();
}
