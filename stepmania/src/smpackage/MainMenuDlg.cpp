// MainMenuDlg.cpp : implementation file
//

#define CO_EXIST_WITH_MFC
#include "global.h"
#include "stdafx.h"
#include "smpackage.h"
#include "MainMenuDlg.h"
#include "EditInsallations.h"
#include "SmpackageExportDlg.h"
#include "ChangeGameSettings.h"
#include "RageUtil.h"
#include "SMPackageUtil.h"
#include "mainmenudlg.h"
#include "archutils/Win32/SpecialDirs.h"
#include "SpecialFiles.h"
#include "ProductInfo.h"

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
	ON_BN_CLICKED(IDC_CREATE_SONG, OnCreateSong)
	ON_BN_CLICKED(IDC_CLEAR_KEYMAPS, OnBnClickedClearKeymaps)
	ON_BN_CLICKED(IDC_CHANGE_PREFERENCES, OnBnClickedChangePreferences)
	ON_BN_CLICKED(IDC_OPEN_PREFERENCES, OnBnClickedOpenPreferences)
	ON_BN_CLICKED(IDC_CLEAR_PREFERENCES, OnBnClickedClearPreferences)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_LAUNCH_GAME, OnBnClickedButtonLaunchGame)
	ON_BN_CLICKED(IDC_VIEW_STATISTICS, OnBnClickedViewStatistics)
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

RString GetLastErrorString()
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
	RString s = (LPCTSTR)lpMsgBuf;
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
	RString sMusicFile = dialog.GetPathName();
	if( iRet != IDOK )
		return;

	BOOL bSuccess;

	RString sSongDirectory = "Songs\\My Creations\\";
	bSuccess = CreateDirectory( sSongDirectory, NULL );
	if( !bSuccess )
	{
		DWORD dwError = ::GetLastError();
		switch( dwError )
		{
		case ERROR_ALREADY_EXISTS:
			// This failure is ok.  We probably created this directory already while importing another song.
			break;
		default:
			MessageBox( "Failed to create directory '" + sSongDirectory + "': " + GetLastErrorString() );
			return;
		}
	}

	sSongDirectory += Basename( sMusicFile );
	bSuccess = CreateDirectory( sSongDirectory, NULL );	// CreateDirectory doesn't like a trailing slash
	if( !bSuccess )
	{
		MessageBox( "Failed to create song directory '" + sSongDirectory + "': " + GetLastErrorString() );
		return;
	}
	sSongDirectory += "\\";

	RString sNewMusicFile = sSongDirectory + Basename(sMusicFile);
	bSuccess = CopyFile( sMusicFile, sNewMusicFile, TRUE );
	if( !bSuccess )
	{
		MessageBox( "Failed to copy music file to '" + sNewMusicFile + "': " + GetLastErrorString() );
		return;
	}

	// create a blank .sm file
	RString sNewSongFile = sMusicFile;
	SetExtension( sNewSongFile, "sm" );
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
	SMPackageUtil::AddStepManiaInstallDir( szCurDir );
	SMPackageUtil::SetDefaultInstallDir( szCurDir );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void MainMenuDlg::OnBnClickedClearKeymaps()
{
	// TODO: Add your control notification handler code here
	
	if( !DoesFileExist( SpecialFiles::KEYMAPS_PATH ) )
	{
		MessageBox( SpecialFiles::KEYMAPS_PATH + " is already cleared." );
	}
	else
	{
		if( !DeleteFile( SpecialFiles::KEYMAPS_PATH ) )
			MessageBox( "Failed to delete file " + SpecialFiles::KEYMAPS_PATH + "." );
	}
}

void MainMenuDlg::OnBnClickedChangePreferences()
{
	// TODO: Add your control notification handler code here
	ChangeGameSettings dlg;
	int nResponse = dlg.DoModal();	
}

void MainMenuDlg::OnBnClickedOpenPreferences()
{
	// TODO: Add your control notification handler code here
	if( !DoesFileExist( SpecialFiles::PREFERENCES_INI_PATH ) )
	{
		MessageBox( SpecialFiles::PREFERENCES_INI_PATH + " doesn't exist.  It will be created next time you start the game." );
	}
	else
	{
		if( NULL == ::ShellExecute( this->m_hWnd, "open", SpecialFiles::PREFERENCES_INI_PATH, "", "", SW_SHOWNORMAL ) )
			MessageBox( "Failed to open " + SpecialFiles::PREFERENCES_INI_PATH + ": " + GetLastErrorString() );
	}
}

void MainMenuDlg::OnBnClickedClearPreferences()
{
	// TODO: Add your control notification handler code here
	if( !DoesFileExist( SpecialFiles::PREFERENCES_INI_PATH ) )
	{
		MessageBox( SpecialFiles::PREFERENCES_INI_PATH + " is already cleared." );
		return;
	}

	if( !DeleteFile( SpecialFiles::PREFERENCES_INI_PATH ) )
	{
		MessageBox( "Failed to delete file " + SpecialFiles::PREFERENCES_INI_PATH + "." );
		return;
	}

	MessageBox( SpecialFiles::PREFERENCES_INI_PATH + " cleared." );
}

void MainMenuDlg::OnBnClickedButtonLaunchGame()
{
	// TODO: Add your control notification handler code here
	SMPackageUtil::LaunchGame();
	exit(0);
}

void MainMenuDlg::OnBnClickedViewStatistics()
{
	// TODO: Add your control notification handler code here
	RString sPersonalDir = GetMyDocumentsDir();
	RString sFile = sPersonalDir + PRODUCT_ID +"/Save/MachineProfile/Stats.xml";
	if( NULL == ::ShellExecute( this->m_hWnd, "open", sFile, "", "", SW_SHOWNORMAL ) )
		MessageBox( "Failed to open '" + sFile + "': " + GetLastErrorString() );
}
