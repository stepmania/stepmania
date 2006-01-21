// EditInsallations.cpp : implementation file
//

#define CO_EXIST_WITH_MFC
#include "global.h"
#include "stdafx.h"
#include "smpackage.h"
#include "EditInsallations.h"
#include "SMPackageUtil.h"
#include "archutils/Win32/DialogUtil.h"
#include ".\editinsallations.h"
#include "LocalizedString.h"
#include "RageUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// EditInsallations dialog


EditInsallations::EditInsallations(CWnd* pParent /*=NULL*/)
	: CDialog(EditInsallations::IDD, pParent)
{
	//{{AFX_DATA_INIT(EditInsallations)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void EditInsallations::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EditInsallations)
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Control(pDX, IDC_EDIT, m_edit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(EditInsallations, CDialog)
	//{{AFX_MSG_MAP(EditInsallations)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_MAKE_DEFAULT, OnButtonMakeDefault)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	//}}AFX_MSG_MAP
	ON_LBN_SELCHANGE(IDC_LIST, OnLbnSelchangeList)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EditInsallations message handlers

BOOL EditInsallations::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	DialogUtil::LocalizeDialogAndContents( *this );	

	vector<RString> vs;
	SMPackageUtil::GetGameInstallDirs( vs );
	for( unsigned i=0; i<vs.size(); i++ )
		m_list.AddString( vs[i] );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void EditInsallations::OnButtonRemove() 
{
	// TODO: Add your control notification handler code here
	ASSERT( m_list.GetCount() > 1 );	// cannot remove the list item in the list
	ASSERT( m_list.GetCurSel() != LB_ERR );

	m_list.DeleteString( m_list.GetCurSel() );
}

void EditInsallations::OnButtonMakeDefault() 
{
	// TODO: Add your control notification handler code here
	ASSERT( m_list.GetCurSel() != LB_ERR );
	
	CString sText;
	m_list.GetText( m_list.GetCurSel(), sText );
	m_list.DeleteString( m_list.GetCurSel() );
	m_list.InsertString( 0, sText );
}

static LocalizedString YOU_MUST_TYPE_IN			("EditInstallations","You must type a program directory before clicking Add.");
static LocalizedString NOT_A_VALID_INSTALLATION_DIR	("EditInstallations","'%s' is not a valid installation directory.");
void EditInsallations::OnButtonAdd() 
{
	// TODO: Add your control notification handler code here
	RString sNewDir;
	{
		CString s;
		m_edit.GetWindowText( s );
		sNewDir = s;
	}
	
	if( sNewDir == "" )
	{
		AfxMessageBox( YOU_MUST_TYPE_IN.GetValue() );
		return;
	}

	bool bAlreadyInList = false;
	for( int i=0; i<m_list.GetCount(); i++ )
	{
		CString sDir;
		m_list.GetText( i, sDir );
		if( sDir.CompareNoCase(sNewDir)==0 )
			return;
	}

	if( !SMPackageUtil::IsValidInstallDir(sNewDir) )
	{
		AfxMessageBox( ssprintf(NOT_A_VALID_INSTALLATION_DIR.GetValue(),sNewDir.c_str()) );
		return;
	}

	m_list.AddString( sNewDir );
}

void EditInsallations::OnOK() 
{
	vector<RString> vs;

	for( int i=0; i<m_list.GetCount(); i++ )
	{
		CString sDir;
		m_list.GetText( i, sDir );
		RString s = sDir;
		vs.push_back( s );
	}
	SMPackageUtil::WriteGameInstallDirs( vs );

	CDialog::OnOK();
}

void EditInsallations::OnLbnSelchangeList()
{
	// TODO: Add your control notification handler code here
	bool bSomethingSelected = m_list.GetCurSel() != LB_ERR;
	bool bMoreThanOne = m_list.GetCurSel() != LB_ERR;

	this->GetDlgItem(IDC_BUTTON_MAKE_DEFAULT)->EnableWindow( bSomethingSelected );
	this->GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow( bMoreThanOne );
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
