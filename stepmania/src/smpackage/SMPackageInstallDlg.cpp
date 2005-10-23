// SMPackageInstallDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smpackage.h"
#include "SMPackageInstallDlg.h"
#include "RageUtil.h"
#include "smpackageUtil.h"
#include "EditInsallations.h"
#include "ShowComment.h"
#include "IniFile.h"	
#include "UninstallOld.h"	

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
}


BEGIN_MESSAGE_MAP(CSMPackageInstallDlg, CDialog)
	//{{AFX_MSG_MAP(CSMPackageInstallDlg)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSMPackageInstallDlg message handlers

BOOL CSMPackageInstallDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	int i;

	//
	// Set the text of the first Edit box
	//
	CString sMessage1 = ssprintf(
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
	CString sMessage2;
	try
	{	
		m_zip.Open( m_sPackagePath, CZipArchive::zipOpenReadOnly );
	}
	catch (CException* e)
	{
		AfxMessageBox( ssprintf("'%s' is not a valid zip archive.", m_sPackagePath), MB_ICONSTOP );
		e->Delete();
		exit( 1 );
	}

	for( i=0; i<m_zip.GetCount(); i++ )
	{
		CZipFileHeader fh;
		m_zip.GetFileInfo(fh, (WORD)i);

		if( fh.IsDirectory() )
			continue;
		if( !fh.GetFileName().CompareNoCase( "smzip.ctl" ) )
			continue;

		sMessage2 += ssprintf( "\t%s\r\n", fh.GetFileName() );
	}
	CEdit* pEdit2 = (CEdit*)GetDlgItem(IDC_EDIT_MESSAGE2);
	pEdit2->SetWindowText( sMessage2 );


	//
	// Set the text of the third Edit box
	//
	CString sMessage3 = "The package will be installed in the following Stepmania program folder:\r\n";

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
	CZipWordArray ar;
	m_zip.FindMatches("smzip.ctl", ar);
	if( ar.GetSize() != 1 )
		return true;

	CZipMemFile control;
	m_zip.ExtractFile( ar[0], control );

	char *buf = new char[control.GetLength()];
	control.Seek( 0, CZipAbstractFile::begin );
	control.Read(buf, control.GetLength());
	IniFile ini;
	ini.ReadBuf( CString(buf, control.GetLength()) );
	delete[] buf;

	int version = 0;
	ini.GetValueI( "SMZIP", "Version", version );
	if( version != 1 )
		return true;

	int cnt = 0;
	ini.GetValueI( "Packages", "NumPackages", cnt );

	int i;
	CStringArray Directories;
	for( i = 0; i < cnt; ++i )
	{
		CString path;
		if( !ini.GetValue( "Packages", ssprintf("%i", i), path) )
			continue;

		/* Does this directory exist? */
		if( !IsADirectory(path) )
			continue;

		if( !IsValidPackageDirectory(path) )
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
	CString cwd(cwd_);
	if( cwd[cwd.GetLength()-1] != '\\' )
		cwd += "\\";

	for( i = 0; i < (int) Directories.size(); ++i )
	{
		CString path = cwd+Directories[i];
		char buf[1024];
		memcpy(buf, path, path.GetLength()+1);
		buf[path.GetLength()+1] = 0;

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

	CString sInstallDir;
	m_comboDir.GetWindowText( sInstallDir );
	
	int iSelectedInstallDirIndex = m_comboDir.GetCurSel();
	if( iSelectedInstallDirIndex == -1 )
	{
		MessageBox( "No Installations", "Error", MB_OK|MB_ICONEXCLAMATION );
		return;
	}

	SetDefaultInstallDir( iSelectedInstallDirIndex );

	// Show comment (if any)
	CString sComment = m_zip.GetGlobalComment();
	bool DontShowComment;
	if( sComment != "" && (!GetPref("DontShowComment", DontShowComment) || !DontShowComment) )
	{
		ShowComment commentDlg;
		commentDlg.m_sComment = sComment;
		int nResponse = commentDlg.DoModal();
		if( nResponse != IDOK )
			return;	// cancelled
		if( commentDlg.m_bDontShow )
			SetPref( "DontShowComment", true );
	}

	/* Check for installed packages that should be deleted before installing. */
	if( !CheckPackages() )
		return;	// cancelled


	// Unzip the SMzip package into the Stepmania installation folder
	for( int i=0; i<m_zip.GetCount(); i++ )
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
			pProgress1->SetRange( 0, m_zip.GetCount());
            pProgress1->SetStep(1);
			pProgress1->SetPos(0);
			SendMessage( WM_PAINT );
		    UpdateWindow();		// Force the silly thing to hadle WM_PAINT now!
			ProgressInit = 1;
		}

retry_unzip:

		// Extract the files
		try
		{	
			// skip extracting "thumbs.db" files
			CZipFileHeader fhInfo;
			if( m_zip.GetFileInfo(fhInfo, (WORD)i) )
			{
				CString sFileName = fhInfo.GetFileName();
				sFileName.MakeLower();
				if( sFileName.Find("thumbs.db") != -1 )
					continue;	// skip to next file
			}

			m_zip.ExtractFile( (WORD)i, sInstallDir, true );	// extract file to current directory
			pProgress1->StepIt(); //increase the progress bar of 1 step
		}
		catch (CException* e)
		{
			char szError[4096];
			e->GetErrorMessage( szError, sizeof(szError) );
			e->Delete();

			switch( MessageBox( szError, "Error Extracting File", MB_ABORTRETRYIGNORE|MB_ICONEXCLAMATION ) )
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
		WriteStepManiaInstallDirs( dlg.m_asReturnedInstallDirs );
		RefreshInstallationList();
	}
}


void CSMPackageInstallDlg::RefreshInstallationList() 
{
	m_comboDir.ResetContent();

	CStringArray asInstallDirs;
	GetStepManiaInstallDirs( asInstallDirs );
	for( unsigned i=0; i<asInstallDirs.size(); i++ )
		m_comboDir.AddString( asInstallDirs[i] );
	m_comboDir.SetCurSel( 0 );	// guaranteed to be at least one item
}

