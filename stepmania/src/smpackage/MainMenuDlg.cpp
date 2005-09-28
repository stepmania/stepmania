// MainMenuDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smpackage.h"
#include "MainMenuDlg.h"
#include "EditInsallations.h"
#include "SmpackageExportDlg.h"
#include "onvertThemeDlg.h"
#include "ChangeGameSettings.h"
#include "RageUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MainMenuDlg dialog


MainMenuDlg::MainMenuDlg(CWnd* pParent /*=NULL*/)
	: CDialog(MainMenuDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(MainMenuDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void MainMenuDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MainMenuDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(MainMenuDlg, CDialog)
	//{{AFX_MSG_MAP(MainMenuDlg)
	ON_BN_CLICKED(IDC_EXPORT_PACKAGES, OnExportPackages)
	ON_BN_CLICKED(IDC_EDIT_INSTALLATIONS, OnEditInstallations)
	ON_BN_CLICKED(IDC_ANALYZE_ELEMENTS, OnAnalyzeElements)
	ON_BN_CLICKED(IDC_CHANGE_API, OnChangeApi)
	ON_BN_CLICKED(IDC_CREATE_SONG, OnCreateSong)
	ON_BN_CLICKED(IDC_OPEN_STEPMANIA_INI, OnOpenStepmaniaIni)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MainMenuDlg message handlers

void MainMenuDlg::OnExportPackages() 
{
	// TODO: Add your control notification handler code here
	CSmpackageExportDlg dlg;
	int nResponse = dlg.DoModal();
//	if (nResponse == IDOK)	
}

void MainMenuDlg::OnEditInstallations() 
{
	// TODO: Add your control notification handler code here
	EditInsallations dlg;
	int nResponse = dlg.DoModal();	
}

void MainMenuDlg::OnAnalyzeElements() 
{
	// TODO: Add your control notification handler code here
	ConvertThemeDlg dlg;
	int nResponse = dlg.DoModal();	
}

void MainMenuDlg::OnChangeApi() 
{
	// TODO: Add your control notification handler code here
	ChangeGameSettings dlg;
	int nResponse = dlg.DoModal();	
}

CString GetLastErrorString()
{
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);
	// Process any inserts in lpMsgBuf.
	// ...
	// Display the string.
	CString s = (LPCTSTR)lpMsgBuf;
	// Free the buffer.
	LocalFree( lpMsgBuf );

	return s;
}

void MainMenuDlg::OnCreateSong() 
{
	// TODO: Add your control notification handler code here
	CFileDialog dialog (
		TRUE,	// file open?
		NULL,	// default file extension
		NULL,	// default file name
		OFN_HIDEREADONLY | OFN_NOCHANGEDIR,		// flags
		"Music file (*.mp3;*.ogg)|*.mp3;*.ogg|||"
		);
	int iRet = dialog.DoModal();
	CString sMusicFile = dialog.GetPathName();
	if( iRet != IDOK )
		return;

	CString sFileNameNoExt, sExt, sThrowAway;
	splitrelpath( 
		sMusicFile, 
		sThrowAway, 
		sFileNameNoExt, 
		sExt
		);

	BOOL bSuccess;

	CString sSongDirectory = "Songs\\My Creations\\";
	CreateDirectory( sSongDirectory, NULL );	// ok if this fails.  It will already exist if we've created another song.
	DWORD dwError = ::GetLastError();
	sSongDirectory += sFileNameNoExt;
	bSuccess = CreateDirectory( sSongDirectory, NULL );	// CreateDirectory doesn't like a trailing slash
	if( !bSuccess )
	{
		MessageBox( "Failed to song directory '" + sSongDirectory + "': " + GetLastErrorString() );
		return;
	}
	sSongDirectory += "\\";

	CString sNewMusicFile = sSongDirectory + sFileNameNoExt + "." + sExt;
	bSuccess = CopyFile( sMusicFile, sNewMusicFile, TRUE );
	if( !bSuccess )
	{
		MessageBox( "Failed to copy music file to '" + sNewMusicFile + "': " + GetLastErrorString() );
		return;
	}

	// create a blank .sm file
	CString sNewSongFile = sSongDirectory + sFileNameNoExt + ".sm";
	FILE *fp = fopen( sNewSongFile, "w" );
	if( fp == NULL )
	{
		MessageBox( "Failed to create the song file '" + sNewSongFile + "'" );
		return;
	}
	fclose( fp );

	MessageBox( "Success.  Created the song '" + sSongDirectory + "'" );
}

BOOL MainMenuDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	TCHAR szCurDir[MAX_PATH];
	GetCurrentDirectory( ARRAYSIZE(szCurDir), szCurDir );
	GetDlgItem( IDC_EDIT_INSTALLATION )->SetWindowText( szCurDir );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void MainMenuDlg::OnOpenStepmaniaIni() 
{
	// TODO: Add your control notification handler code here
	::ShellExecute( this->m_hWnd, "open", "Data\\StepMania.ini", "", "", SW_SHOWNORMAL  );
}
