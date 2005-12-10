// UninstallOld.cpp : implementation file
//

#define CO_EXIST_WITH_MFC
#include "global.h"
#include "stdafx.h"
#include "smpackage.h"
#include "UninstallOld.h"


// UninstallOld dialog

UninstallOld::UninstallOld(CWnd* pParent /*=NULL*/)
	: CDialog(UninstallOld::IDD, pParent)
	, m_sPackages(_T(""))
{
}

UninstallOld::~UninstallOld()
{
}

void UninstallOld::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PACKAGES, m_sPackages);
}

BOOL UninstallOld::OnInitDialog()
{
	CDialog::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(UninstallOld, CDialog)
	//{{AFX_MSG_MAP(UninstallOld)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedNo)
END_MESSAGE_MAP()




// UninstallOld message handlers

void UninstallOld::OnOK()
{
	EndDialog(IDOK);
}

void UninstallOld::OnCancel()
{
	EndDialog(IDCANCEL);
}

void UninstallOld::OnBnClickedNo()
{
	EndDialog(IDIGNORE);
}
