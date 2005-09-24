// ChangeGameSettings.cpp : implementation file
//

#include "stdafx.h"
#include "smpackage.h"
#include "ChangeGameSettings.h"
#include "IniFile.h"

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
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ChangeGameSettings message handlers

BOOL ChangeGameSettings::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	//
	// Fill the radio buttons
	//
	IniFile ini;
	ini.SetPath( "Data\\StepMania.ini" );
	ini.ReadFile();

	CString sValue;
	ini.GetValue( "Options", "VideoRenderers", sValue );
	
	if( sValue.CompareNoCase("opengl")==0 )
		CheckDlgButton( IDC_RADIO_OPENGL, BST_CHECKED );
	else if( sValue.CompareNoCase("d3d")==0 )
		CheckDlgButton( IDC_RADIO_DIRECT3D, BST_CHECKED );
	else
		CheckDlgButton( IDC_RADIO_DEFAULT, BST_CHECKED );

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void ChangeGameSettings::OnOK() 
{
	// TODO: Add extra validation here
	IniFile ini;
	ini.SetPath( "Data\\StepMania.ini" );
	ini.ReadFile();
	
	if( BST_CHECKED == IsDlgButtonChecked(IDC_RADIO_OPENGL) )
		ini.SetValue( "Options", "VideoRenderers", "opengl" );
	else if( BST_CHECKED == IsDlgButtonChecked(IDC_RADIO_DIRECT3D) )
		ini.SetValue( "Options", "VideoRenderers", "d3d" );
	else
		ini.SetValue( "Options", "VideoRenderers", "" );

	ini.WriteFile();
	
	CDialog::OnOK();
}
