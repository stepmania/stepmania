// ChangeGameSettings.cpp : implementation file
//

#define CO_EXIST_WITH_MFC
#include "global.h"
#include "stdafx.h"
#include "smpackage.h"
#include "ChangeGameSettings.h"
#include "IniFile.h"
#include "SMPackageUtil.h"
#include "SpecialFiles.h"
#include ".\changegamesettings.h"
#include "archutils/Win32/DialogUtil.h"
#include "LocalizedString.h"
#include "RageUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ChangeGameSettings dialog


ChangeGameSettings::ChangeGameSettings(CWnd* pParent /*=NULL*/)
	: CDialog(ChangeGameSettings::IDD, pParent)
{
	//{{AFX_DATA_INIT(ChangeGameSettings)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void ChangeGameSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ChangeGameSettings)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ChangeGameSettings, CDialog)
	//{{AFX_MSG_MAP(ChangeGameSettings)
	//}}AFX_MSG_MAP
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ChangeGameSettings message handlers

BOOL ChangeGameSettings::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	DialogUtil::LocalizeDialogAndContents( *this );

	//
	// Fill the radio buttons
	//
	IniFile ini;
	ini.ReadFile( SpecialFiles::PREFERENCES_INI_PATH );

	RString sValue;


	sValue = "";
	ini.GetValue( "Options", "VideoRenderers", sValue );	
	if( sValue.CompareNoCase("opengl")==0 )
		CheckDlgButton( IDC_RADIO_OPENGL, BST_CHECKED );
	else if( sValue.CompareNoCase("d3d")==0 )
		CheckDlgButton( IDC_RADIO_DIRECT3D, BST_CHECKED );
	else
		CheckDlgButton( IDC_RADIO_DEFAULT, BST_CHECKED );


	sValue = "";
	ini.GetValue( "Options", "SoundDrivers", sValue );
	if( sValue.CompareNoCase("DirectSound")==0 )
		CheckDlgButton( IDC_RADIO_SOUND_DIRECTSOUND_HARDWARE, BST_CHECKED );
	else if( sValue.CompareNoCase("DirectSound-sw")==0 )
		CheckDlgButton( IDC_RADIO_SOUND_DIRECTSOUND_SOFTWARE, BST_CHECKED );
	else if( sValue.CompareNoCase("WaveOut")==0 )
		CheckDlgButton( IDC_RADIO_SOUND_WAVEOUT, BST_CHECKED );
	else if( sValue.CompareNoCase("null")==0 )
		CheckDlgButton( IDC_RADIO_SOUND_NULL, BST_CHECKED );
	else
		CheckDlgButton( IDC_RADIO_SOUND_DEFAULT, BST_CHECKED );

	{
		int iValue = 0;
		ini.GetValue( "Options", "RefreshRate", iValue );
		CheckDlgButton( IDC_CHECK_FORCE_60HZ, iValue == 60 ? BST_CHECKED : BST_UNCHECKED );
	}
	{
		bool bValue = false;
		ini.GetValue( "Options", "LogToDisk", bValue );
		CheckDlgButton( IDC_CHECK_LOG_TO_DISK, bValue ? BST_CHECKED : BST_UNCHECKED );
	}
	{
		bool bValue = false;
		ini.GetValue( "Options", "ShowLogOutput", bValue );
		CheckDlgButton( IDC_CHECK_SHOW_LOG_WINDOW, bValue ? BST_CHECKED : BST_UNCHECKED );
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

static LocalizedString ERROR_WRITING_FILE( "ChangeGameSettings", "Error writing file '%s': %s" );
void ChangeGameSettings::OnOK() 
{
	// TODO: Add extra validation here
	IniFile ini;
	ini.ReadFile( SpecialFiles::PREFERENCES_INI_PATH );

	
	if( BST_CHECKED == IsDlgButtonChecked(IDC_RADIO_OPENGL) )
		ini.SetValue( "Options", "VideoRenderers", (RString)"opengl" );
	else if( BST_CHECKED == IsDlgButtonChecked(IDC_RADIO_DIRECT3D) )
		ini.SetValue( "Options", "VideoRenderers", (RString)"d3d" );
	else
		ini.SetValue( "Options", "VideoRenderers", RString() );


	if( BST_CHECKED == IsDlgButtonChecked(IDC_RADIO_SOUND_DIRECTSOUND_HARDWARE) )
		ini.SetValue( "Options", "SoundDrivers", (RString)"DirectSound" );
	else if( BST_CHECKED == IsDlgButtonChecked(IDC_RADIO_SOUND_DIRECTSOUND_SOFTWARE) )
		ini.SetValue( "Options", "SoundDrivers", (RString)"DirectSound-sw" );
	else if( BST_CHECKED == IsDlgButtonChecked(IDC_RADIO_SOUND_WAVEOUT) )
		ini.SetValue( "Options", "SoundDrivers", (RString)"WaveOut" );
	else if( BST_CHECKED == IsDlgButtonChecked(IDC_RADIO_SOUND_NULL) )
		ini.SetValue( "Options", "SoundDrivers", (RString)"null" );
	else
		ini.SetValue( "Options", "SoundDrivers", RString() );


	if( BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_FORCE_60HZ) )
	{
		ini.SetValue( "Options", "RefreshRate", 60 );
	}
	else
	{
		int iRefresh = 0;
		ini.GetValue( "Options", "RefreshRate", iRefresh );
		if( iRefresh == 60 )
			ini.SetValue( "Options", "RefreshRate", 0 );
	}
	ini.SetValue( "Options", "LogToDisk",		BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_LOG_TO_DISK) );
	ini.SetValue( "Options", "ShowLogOutput",	BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_SHOW_LOG_WINDOW) );
	

	if( !ini.WriteFile(SpecialFiles::PREFERENCES_INI_PATH) )
	{
		RString sError = ssprintf( ERROR_WRITING_FILE.GetValue(), SpecialFiles::PREFERENCES_INI_PATH.c_str(), ini.GetError().c_str() );
		AfxMessageBox( sError );
	}
	
	CDialog::OnOK();
}

HBRUSH ChangeGameSettings::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
