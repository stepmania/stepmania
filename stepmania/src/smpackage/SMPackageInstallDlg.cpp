// SMPackageInstallDlg.cpp : implementation file
//

#define CO_EXIST_WITH_MFC
#include "global.h"
#include "stdafx.h"
#include "smpackage.h"
#include "SMPackageInstallDlg.h"
#include "RageUtil.h"
#include "smpackageUtil.h"
#include "EditInsallations.h"
#include "ShowComment.h"
#include "IniFile.h"	
#include "UninstallOld.h"	
#include <algorithm>	
#include "RageFileManager.h"	
#include "RageFileDriverZip.h"	

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const RString TEMP_MOUNT_POINT = "/@package/";

/////////////////////////////////////////////////////////////////////////////
// CSMPackageInstallDlg dialog


CSMPackageInstallDlg::CSMPackageInstallDlg(CString sPackagePath, CWnd* pParent /*=NULL*/)
	: CDialog(CSMPackageInstallDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSMPackageInstallDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_sPackagePath = sPackagePath;
}


void CSMPackageInstallDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSMPackageInstallDlg)
	DDX_Control(pDX, IDC_BUTTON_EDIT, m_buttonEdit);
	DDX_Control(pDX, IDC_COMBO_DIR, m_comboDir);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_STATIC_HEADER_TEXT, m_staticHeaderText);
}


BEGIN_MESSAGE_MAP(CSMPackageInstallDlg, CDialog)
	//{{AFX_MSG_MAP(CSMPackageInstallDlg)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSMPackageInstallDlg message handlers

static bool CompareStringNoCase( const RString &s1, const RString &s2 )
{
	return s1.CompareNoCase( s2 ) < 0;
}

BOOL CSMPackageInstallDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	SMPackageUtil::LocalizeDialogAndContents( *this );
	SMPackageUtil::SetHeaderFont( *this );

	// mount the zip
	if( !FILEMAN->Mount( "zip", m_sPackagePath, TEMP_MOUNT_POINT ) )
	{
		AfxMessageBox( ssprintf("'%s' is not a valid zip archive.", m_sPackagePath), MB_ICONSTOP );
		exit( 1 );
	}

	//
	// Set the text of the first Edit box
	//
	RString sMessage1 = ssprintf(
		"You have chosen to install the Stepmania package:\r\n"
		"\r\n"
		"\t%s\r\n"
		"\r\n"
		"This package contains the following files:\r\n",
		m_sPackagePath
	);
	CEdit* pEdit1 = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE1);
	pEdit1->SetWindowText( sMessage1 );


	//
	// Set the text of the second Edit box
	//
	{
		vector<RString> vs;
		GetDirListingRecursive( TEMP_MOUNT_POINT, "*.*", vs );
		CEdit* pEdit2 = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE2);
		RString sText = "\t" + join( "\r\n\t", vs );
		pEdit2->SetWindowText( sText );
	}


	//
	// Set the text of the third Edit box
	//
	RString sMessage3 = "The package will be installed in the following Stepmania program folder:\r\n";

	// Set the message
	CEdit* pEdit3 = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE3);
	pEdit3->SetWindowText( sMessage3 );


	RefreshInstallationList();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CSMPackageInstallDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}
#include <direct.h>

bool CSMPackageInstallDlg::CheckPackages()
{
	IniFile ini;
	if( !ini.ReadFile(TEMP_MOUNT_POINT + "smzip.ctl") )
		return true;

	int version = 0;
	ini.GetValue( "SMZIP", "Version", version );
	if( version != 1 )
		return true;

	int cnt = 0;
	ini.GetValue( "Packages", "NumPackages", cnt );

	vector<RString> Directories;
	for( int i = 0; i < cnt; ++i )
	{
		RString path;
		if( !ini.GetValue( "Packages", ssprintf("%i", i), path) )
			continue;

		/* Does this directory exist? */
		if( !FILEMAN->IsADirectory(path) )
			continue;

		if( !SMPackageUtil::IsValidPackageDirectory(path) )
			continue;
		
		Directories.push_back(path);
	}

	if( Directories.empty() )
		return true;

	{
		UninstallOld UninstallOldDlg;
		UninstallOldDlg.m_sPackages = join("\r\n", Directories);
		int nResponse = UninstallOldDlg.DoModal();
		if( nResponse == IDCANCEL )
			return false;	// cancelled
		if( nResponse == IDIGNORE )
			return true;
	}

	char cwd_[MAX_PATH];
	_getcwd(cwd_, MAX_PATH);
	RString cwd(cwd_);
	if( cwd[cwd.size()-1] != '\\' )
		cwd += "\\";

	for( i = 0; i < (int) Directories.size(); ++i )
	{
		RString path = cwd+Directories[i];
		char buf[1024];
		memcpy( buf, path, path.size()+1 );
		buf[path.size()+1] = 0;

		SHFILEOPSTRUCT op;
		memset(&op, 0, sizeof(op));

		op.wFunc = FO_DELETE;
		op.pFrom = buf;
		op.pTo = NULL;
		op.fFlags = FOF_NOCONFIRMATION;
		if( !SHFileOperation(&op) )
			continue;

		/* Something failed.  SHFileOperation displayed the error dialog, so just cancel. */
		return false;
	}

	return true;
}

void CSMPackageInstallDlg::OnOK() 
{
	// TODO: Add extra validation here

	int ProgressInit = 0;

	m_comboDir.EnableWindow( FALSE );
	m_buttonEdit.EnableWindow( FALSE );

	RString sInstallDir;
	{
		CString s;
		m_comboDir.GetWindowText( s );
		sInstallDir = s;
	}
	
	int iSelectedInstallDirIndex = m_comboDir.GetCurSel();
	if( iSelectedInstallDirIndex == -1 )
	{
		MessageBox( "No Installations", "Error", MB_OK|MB_ICONEXCLAMATION );
		return;
	}

	SMPackageUtil::SetDefaultInstallDir( iSelectedInstallDirIndex );

	// Show comment (if any)
	{
		RageFileDriverZip zip;
		zip.Load( m_sPackagePath );
		RString sComment = zip.GetGlobalComment();
		bool DontShowComment;
		if( sComment != "" && (!SMPackageUtil::GetPref("DontShowComment", DontShowComment) || !DontShowComment) )
		{
			ShowComment commentDlg;
			commentDlg.m_sComment = sComment;
			int nResponse = commentDlg.DoModal();
			if( nResponse != IDOK )
				return;	// cancelled
			if( commentDlg.m_bDontShow )
				SMPackageUtil::SetPref( "DontShowComment", true );
		}
	}

	/* Check for installed packages that should be deleted before installing. */
	if( !CheckPackages() )
		return;	// cancelled


	// Unzip the SMzip package into the Stepmania installation folder
	vector<RString> vs;
	GetDirListingRecursive( TEMP_MOUNT_POINT, "*.*", vs );
	for( unsigned i=0; i<vs.size(); i++ )
	{
		// Throw some text up so the user has something to look at during the long pause.
		CEdit* pEdit1 = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE1);
		pEdit1->SetWindowText( ssprintf("Installing '%s'.  Please wait...", m_sPackagePath) );
		CEdit* pEdit2 = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE2);
		pEdit2->SetWindowText( "" );
		CEdit* pEdit3 = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE3);
		pEdit3->SetWindowText( "" );
		CProgressCtrl* pProgress1 = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS1);
		//Show the hided progress bar
	    if(!pProgress1->IsWindowVisible())
        {
			pProgress1->ShowWindow(SW_SHOWNORMAL);
        }
		//Initialize the progress bar and update the window 1 time (it's enough)
        if(!ProgressInit)
		{
			pProgress1->SetRange( 0, (short)vs.size() );
            pProgress1->SetStep(1);
			pProgress1->SetPos(0);
			SendMessage( WM_PAINT );
		    UpdateWindow();		// Force the silly thing to hadle WM_PAINT now!
			ProgressInit = 1;
		}

retry_unzip:

		// Extract the files
		const RString sFile = vs[i];
		// skip extracting "thumbs.db" files
		if( Basename(sFile).CompareNoCase("thumbs.db") == 0 )
			continue;

		RString sBareFile = sFile;
		sBareFile.erase( sBareFile.begin(), sBareFile.begin()+TEMP_MOUNT_POINT.length() );
		RString sTo = sInstallDir + sTo;
		if( !FileCopy( sFile, sTo ) )
		{
			RString sError = ssprintf( "Error copying file '%s'", sBareFile.c_str() );
			switch( MessageBox( sError, "Error Extracting File", MB_ABORTRETRYIGNORE|MB_ICONEXCLAMATION ) )
			{
			case IDABORT:
				exit(1);
				break;
			case IDRETRY:
				goto retry_unzip;
				break;
			case IDIGNORE:
				// do nothing
				break;
			}
		}

		pProgress1->StepIt(); //increase the progress bar of 1 step
	}

	AfxMessageBox( "Package installed successfully!" );

	// close the dialog
	CDialog::OnOK();
}


void CSMPackageInstallDlg::OnButtonEdit() 
{
	// TODO: Add your control notification handler code here
	EditInsallations dlg;
	int nResponse = dlg.DoModal();
	if( nResponse == IDOK )
	{
		SMPackageUtil::WriteStepManiaInstallDirs( dlg.m_vsReturnedInstallDirs );
		RefreshInstallationList();
	}
}


void CSMPackageInstallDlg::RefreshInstallationList() 
{
	m_comboDir.ResetContent();

	vector<RString> asInstallDirs;
	SMPackageUtil::GetStepManiaInstallDirs( asInstallDirs );
	for( unsigned i=0; i<asInstallDirs.size(); i++ )
		m_comboDir.AddString( asInstallDirs[i] );
	m_comboDir.SetCurSel( 0 );	// guaranteed to be at least one item
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
