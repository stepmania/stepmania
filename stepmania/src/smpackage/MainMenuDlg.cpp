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
#include "mainmenudlg.h"
#include "LanguagesDlg.h"
#include ".\mainmenudlg.h"
#include "archutils/Win32/DialogUtil.h"
#include "LocalizedString.h"
#include "RageFileManager.h"

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
	ON_BN_CLICKED(IDC_LANGUAGES, OnBnClickedLanguages)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MainMenuDlg message handlers

void MainMenuDlg::OnExportPackages() 
{
	// TODO: Add your control notification handler code here
	CSmpackageExportDlg dlg;
	dlg.DoModal();
//	if (nResponse == IDOK)	
}

void MainMenuDlg::OnEditInstallations() 
{
	// TODO: Add your control notification handler code here
	EditInsallations dlg;
	dlg.DoModal();	
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

static LocalizedString MUSIC_FILE				( "MainMenuDlg", "Music file" );
static LocalizedString THE_SONG_DIRECTORY_ALREADY_EXISTS	( "MainMenuDlg", "The song directory '%s' already exists.  You cannot override an existing song." );
static LocalizedString FAILED_TO_COPY_MUSIC_FILE		( "MainMenuDlg", "Failed to copy music file '%s' to '%s': %s" );
static LocalizedString FAILED_TO_CREATE_THE_SONG_FILE		( "MainMenuDlg", "Failed to create the song file '%s'" );
static LocalizedString SUCCESS_CREATED_THE_SONG			( "MainMenuDlg", "Success.  Created the song '%s'" );
void MainMenuDlg::OnCreateSong() 
{
	// TODO: Add your control notification handler code here
	ASSERT(0);
	// fix compile
	CFileDialog dialog (
		TRUE,	// file open?
		NULL,	// default file extension
		NULL,	// default file name
		OFN_HIDEREADONLY | OFN_NOCHANGEDIR,		// flags
		ConvertUTF8ToACP(MUSIC_FILE.GetValue()+" (*.mp3;*.ogg)|*.mp3;*.ogg|||").c_str()
		);
	int iRet = dialog.DoModal();
	RString sMusicFile = dialog.GetPathName();
	if( iRet != IDOK )
		return;

	RString sSongDirectory = "Songs/My Creations/" + GetFileNameWithoutExtension(sMusicFile) + "/";
	RString sNewMusicFile = sSongDirectory + Basename(sMusicFile);

	if( DoesFileExist(sSongDirectory) )
	{
		MessageBox( ssprintf(THE_SONG_DIRECTORY_ALREADY_EXISTS.GetValue(),sSongDirectory.c_str()) );
		return;
	}

	RageFileOsAbsolute fileIn;
	fileIn.Open( sMusicFile, RageFile::READ );
	RageFile fileOut;
	fileOut.Open( sNewMusicFile, RageFile::WRITE );
	RString sError;
	bool bSuccess = FileCopy( fileIn, fileOut, sError );
	if( !bSuccess )
	{
		MessageBox( ssprintf(FAILED_TO_COPY_MUSIC_FILE.GetValue(),sMusicFile.c_str(),sNewMusicFile.c_str(),sError.c_str()) );
		return;
	}

	// create a blank .sm file
	RString sNewSongFile = sMusicFile;
	SetExtension( sNewSongFile, "sm" );
	RageFile file;
	if( file.Open(sNewSongFile, RageFile::WRITE) )
	{
		MessageBox( ssprintf(FAILED_TO_CREATE_THE_SONG_FILE.GetValue(),sNewSongFile.c_str()) );
		return;
	}
	file.Close();

	MessageBox( ssprintf(SUCCESS_CREATED_THE_SONG.GetValue(),sSongDirectory.c_str()) );
}

BOOL MainMenuDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	DialogUtil::LocalizeDialogAndContents( *this );
	DialogUtil::SetHeaderFont( *this, IDC_STATIC_HEADER_TEXT );

	TCHAR szCurDir[MAX_PATH];
	GetCurrentDirectory( ARRAYSIZE(szCurDir), szCurDir );
	GetDlgItem( IDC_EDIT_INSTALLATION )->SetWindowText( szCurDir );
	SMPackageUtil::AddGameInstallDir( szCurDir );
	SMPackageUtil::SetDefaultInstallDir( szCurDir );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

static LocalizedString FAILED_TO_DELETE_FILE	( "MainMenuDlg", "Failed to delete file '%s'." );
static LocalizedString IS_ALREADY_CLEARED	( "MainMenuDlg", "'%s' is already cleared." );
static LocalizedString CLEARED( "MainMenuDlg", "'%s' cleared" );
void MainMenuDlg::OnBnClickedClearKeymaps()
{
	// TODO: Add your control notification handler code here
	
	if( !DoesFileExist( SpecialFiles::KEYMAPS_PATH ) )
	{
		MessageBox( ssprintf(IS_ALREADY_CLEARED.GetValue(),SpecialFiles::KEYMAPS_PATH.c_str()) );
		return;
	}

	if( !FILEMAN->Remove(SpecialFiles::KEYMAPS_PATH) )
		MessageBox( ssprintf(FAILED_TO_DELETE_FILE.GetValue(), SpecialFiles::KEYMAPS_PATH.c_str()) );

	MessageBox( ssprintf(CLEARED.GetValue(),SpecialFiles::PREFERENCES_INI_PATH.c_str()) );
}

void MainMenuDlg::OnBnClickedChangePreferences()
{
	// TODO: Add your control notification handler code here
	ChangeGameSettings dlg;
	dlg.DoModal();	
}

static LocalizedString DOESNT_EXIST_IT_WILL_BE_CREATED	( "MainMenuDlg", "'%s' doesn't exist.  It will be created next time you start the game." );
static LocalizedString FAILED_TO_OPEN					( "MainMenuDlg", "Failed to open '%s': %s" );
void MainMenuDlg::OnBnClickedOpenPreferences()
{
	// TODO: Add your control notification handler code here
	if( !DoesFileExist( SpecialFiles::PREFERENCES_INI_PATH ) )
	{
		MessageBox( ssprintf(DOESNT_EXIST_IT_WILL_BE_CREATED.GetValue(),SpecialFiles::PREFERENCES_INI_PATH.c_str()) );
	}
	else
	{
		if( NULL == ::ShellExecute( this->m_hWnd, "open", SpecialFiles::PREFERENCES_INI_PATH, "", "", SW_SHOWNORMAL ) )
			MessageBox( ssprintf(FAILED_TO_OPEN.GetValue(),SpecialFiles::PREFERENCES_INI_PATH.c_str(),GetLastErrorString().c_str()) );
	}
}

void MainMenuDlg::OnBnClickedClearPreferences()
{
	// TODO: Add your control notification handler code here
	if( !DoesFileExist(SpecialFiles::PREFERENCES_INI_PATH) )
	{
		MessageBox( ssprintf(IS_ALREADY_CLEARED.GetValue(),SpecialFiles::PREFERENCES_INI_PATH.c_str()) );
		return;
	}

	if( !FILEMAN->Remove(SpecialFiles::PREFERENCES_INI_PATH) )
	{
		MessageBox( ssprintf(FAILED_TO_DELETE_FILE.GetValue(),SpecialFiles::PREFERENCES_INI_PATH.c_str()) );
		return;
	}

	MessageBox( ssprintf(CLEARED.GetValue(),SpecialFiles::PREFERENCES_INI_PATH.c_str()) );
}

void MainMenuDlg::OnBnClickedButtonLaunchGame()
{
	// TODO: Add your control notification handler code here
	if( SMPackageUtil::LaunchGame() )
		exit(0);
}

void MainMenuDlg::OnBnClickedLanguages()
{
	// TODO: Add your control notification handler code here
	LanguagesDlg dlg;
	dlg.DoModal();	
}

HBRUSH MainMenuDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	if( pWnd->GetDlgCtrlID() == IDC_STATIC_HEADER_TEXT )
	{
        hbr = (HBRUSH)::GetStockObject(HOLLOW_BRUSH); 
        pDC->SetBkMode(TRANSPARENT); 
	}

	// TODO:  Return a different brush if the default is not desired
	return hbr;
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