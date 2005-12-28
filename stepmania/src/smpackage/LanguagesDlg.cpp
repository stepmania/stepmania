// LanguagesDlg.cpp : implementation file
//

#define CO_EXIST_WITH_MFC
#include "global.h"
#include "stdafx.h"
#include "smpackage.h"
#include "LanguagesDlg.h"
#include "SpecialFiles.h"
#include "RageUtil.h"


// LanguagesDlg dialog

IMPLEMENT_DYNAMIC(LanguagesDlg, CDialog)
LanguagesDlg::LanguagesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(LanguagesDlg::IDD, pParent)
{
}

LanguagesDlg::~LanguagesDlg()
{
}

void LanguagesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_THEMES, m_listThemes);
	DDX_Control(pDX, IDC_LIST_LANGUAGES, m_listLanguages);
}

BOOL LanguagesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	vector<RString> vs;
	GetDirListing( SpecialFiles::THEMES_DIR+"*", vs, true );
	FOREACH_CONST( RString, vs, s )
		m_listThemes.AddString( *s );
	if( !vs.empty() )
		m_listThemes.SetSel( 0 );
	else
		OnSelchangeListThemes();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

static RString GetCurrentString( const CListBox &list )
{
	// TODO: Add your control notification handler code here
	int iSel = list.GetCurSel();
	if( iSel == LB_ERR )
		return RString();
	CString s;
	list.GetText( list.GetCurSel(), s );
	return RString( s );
}

void LanguagesDlg::OnSelchangeListThemes() 
{
	// TODO: Add your control notification handler code here
	m_listLanguages.ResetContent();

	RString sTheme = GetCurrentString( m_listThemes );
	if( sTheme.empty() )
	{
		OnSelchangeListLanguages();
		return;
	}

	RString sLanguagesDir = SpecialFiles::THEMES_DIR + sTheme + "/" + SpecialFiles::LANGUAGES_SUBDIR;

	vector<RString> vs;
	GetDirListing( sLanguagesDir+"*.ini", vs, false );
	FOREACH_CONST( RString, vs, s )
		m_listLanguages.AddString( GetFileNameWithoutExtension(*s) );
	if( !vs.empty() )
		m_listLanguages.SetSel( 0 );
	else
		OnSelchangeListLanguages();
}

void LanguagesDlg::OnSelchangeListLanguages() 
{
	// TODO: Add your control notification handler code here
	int iTotalStrings = -1;
	int iNeedTranslation = -1;

	RString sTheme = GetCurrentString( m_listThemes );
	RString sLanguage = GetCurrentString( m_listLanguages );

	if( !sTheme.empty() && !sLanguage.empty() )
	{
		RString sThemeDir = "Themes/" + sTheme + "/";
		RString sLanguageFile = sThemeDir + SpecialFiles::LANGUAGES_SUBDIR + sLanguage + ".ini";

		iTotalStrings = rand()%100;
		iNeedTranslation = rand()%100;
	}

	GetDlgItem(IDC_STATIC_TOTAL_STRINGS		)->SetWindowText( ssprintf(iTotalStrings==-1?"":"%d",iTotalStrings) ); 
	GetDlgItem(IDC_STATIC_NEED_TRANSLATION	)->SetWindowText( ssprintf(iNeedTranslation==-1?"":"%d",iNeedTranslation) ); 
}


BEGIN_MESSAGE_MAP(LanguagesDlg, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_THEMES, OnSelchangeListThemes)
	ON_LBN_SELCHANGE(IDC_LIST_LANGUAGES, OnSelchangeListLanguages)
END_MESSAGE_MAP()


// LanguagesDlg message handlers
