// UninstallOld.cpp : implementation file
//

#define CO_EXIST_WITH_MFC
#include "global.h"
#include "stdafx.h"
#include "smpackage.h"
#include "UninstallOld.h"
#include "SMPackageUtil.h"
#include "archutils/Win32/DialogUtil.h"

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

	DialogUtil::LocalizeDialogAndContents( *this );

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

/*
 * (c) 2002-2005 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
