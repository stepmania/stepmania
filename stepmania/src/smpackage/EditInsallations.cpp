// EditInsallations.cpp : implementation file
//

#define CO_EXIST_WITH_MFC
#include "global.h"
#include "stdafx.h"
#include "smpackage.h"
#include "EditInsallations.h"
#include "smpackageUtil.h"

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
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EditInsallations message handlers

BOOL EditInsallations::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	SMPackageUtil::LocalizeDialogAndContents( *this );	

	vector<RString> vs;
	SMPackageUtil::GetStepManiaInstallDirs( vs );
	for( unsigned i=0; i<vs.size(); i++ )
		m_list.AddString( vs[i] );


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void EditInsallations::OnButtonRemove() 
{
	// TODO: Add your control notification handler code here
	if( m_list.GetCount() == 1 )
	{
		AfxMessageBox( "You cannot remove the list item in the list." );
		return;
	}

	if( m_list.GetCurSel() == LB_ERR )
	{
		AfxMessageBox( "You must select an item from the list first." );
		return;
	}

	m_list.DeleteString( m_list.GetCurSel() );
}

void EditInsallations::OnButtonMakeDefault() 
{
	// TODO: Add your control notification handler code here
	if( m_list.GetCurSel() == LB_ERR )
	{
		AfxMessageBox( "You must select an item from the list first." );
		return;
	}
	
	CString sText;
	m_list.GetText( m_list.GetCurSel(), sText );
	m_list.DeleteString( m_list.GetCurSel() );
	m_list.InsertString( 0, sText );
}

void EditInsallations::OnButtonAdd() 
{
	// TODO: Add your control notification handler code here
	CString sText;
	m_edit.GetWindowText( sText );
	
	if( sText == "" )
	{
		AfxMessageBox( "You must type a SM or DWI program directory before clicking add." );
		return;
	}

	m_list.AddString( sText );
}

void EditInsallations::OnOK() 
{
	m_vsReturnedInstallDirs.clear();

	for( int i=0; i<m_list.GetCount(); i++ )
	{
		CString sDir;
		m_list.GetText( i, sDir );
		RString s = sDir;
		m_vsReturnedInstallDirs.push_back( s );
	}

	CDialog::OnOK();
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
