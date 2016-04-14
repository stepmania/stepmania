// EnterComment.cpp : implementation file

#define CO_EXIST_WITH_MFC
#include "global.h"
#include "stdafx.h"
#include "smpackage.h"
#include "EnterComment.h"
#include "archutils/Win32/DialogUtil.h"
#include ".\entercomment.h"

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
	, m_bShowAComment(FALSE)
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
	DDX_Check(pDX, IDC_SHOW_A_COMMENT, m_bShowAComment);
}

BOOL EnterComment::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	DialogUtil::LocalizeDialogAndContents( *this );

	OnBnClickedShowAComment();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(EnterComment, CDialog)
	//{{AFX_MSG_MAP(EnterComment)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_SHOW_A_COMMENT, OnBnClickedShowAComment)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EnterComment message handlers

void EnterComment::OnOK() 
{
	// TODO: Add extra validation here

	UpdateData( TRUE );

	if( m_bShowAComment )
		m_edit.GetWindowText( m_sEnteredComment );

	CDialog::OnOK();
}

void EnterComment::OnBnClickedShowAComment()
{
	UpdateData( TRUE );

	// TODO: Add your control notification handler code here
	GetDlgItem( IDC_EDIT )->EnableWindow( m_bShowAComment );
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
 * 
 * (c) 2016- Electromuis, Anton Grootes
 * This branch of https://github.com/stepmania/stepmania
 * will from here on out be released as GPL v3 (wich converts from the previous MIT license)
 */