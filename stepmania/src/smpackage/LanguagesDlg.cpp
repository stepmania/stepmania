// LanguagesDlg.cpp : implementation file
//

#define CO_EXIST_WITH_MFC
#include "global.h"
#include "stdafx.h"
#include "smpackage.h"
#include "LanguagesDlg.h"
#include "SpecialFiles.h"
#include "RageUtil.h"
#include "IniFile.h"
#include "languagesdlg.h"
#include "SMPackageUtil.h"
#include "CreateLanguageDlg.h"
#include "RageFile.h"
#include "languagesdlg.h"
#include "RageFileManager.h"


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
	StripCvs( vs );
	FOREACH_CONST( RString, vs, s )
		m_listThemes.AddString( *s );
	if( !vs.empty() )
		m_listThemes.SetSel( 0 );

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

static void SelectString( CListBox &list, const RString &sToSelect )
{
	for( int i=0; i<list.GetCount(); i++ )
	{
		CString s;
		list.GetText( i, s );
		if( s == sToSelect )
		{
			list.SetCurSel( i );
			break;
		}
	}
}

void LanguagesDlg::OnSelchangeListThemes() 
{
	// TODO: Add your control notification handler code here
	m_listLanguages.ResetContent();

	RString sTheme = GetCurrentString( m_listThemes );
	if( !sTheme.empty() )
	{
		RString sLanguagesDir = SpecialFiles::THEMES_DIR + sTheme + "/" + SpecialFiles::LANGUAGES_SUBDIR;

		vector<RString> vs;
		GetDirListing( sLanguagesDir+"*.ini", vs, false );
		FOREACH_CONST( RString, vs, s )
		{
			RString sIsoCode = GetFileNameWithoutExtension(*s);
			m_listLanguages.AddString( SMPackageUtil::GetLanguageDisplayString(sIsoCode) );
		}
		if( !vs.empty() )
			m_listLanguages.SetSel( 0 );
	}

	OnSelchangeListLanguages();
}

static RString GetLanguageFile( const RString &sTheme, const RString &sLanguage )
{
	return SpecialFiles::THEMES_DIR + sTheme + "/" + SpecialFiles::LANGUAGES_SUBDIR + sLanguage + ".ini";
}

static int GetNumValuesInIniFile( const RString &sIniFile )
{
	int count = 0;
	IniFile ini;
	ini.ReadFile( sIniFile );
	FOREACH_CONST_Child( &ini, key )
	{
		FOREACH_CONST_Attr( key, value )
			count++;
	}
	return count;
}

static int GetNumIntersectingIniValues( const RString &sIniFile1, const RString &sIniFile2 )
{
	int count = 0;
	IniFile ini1;
	ini1.ReadFile( sIniFile1 );
	IniFile ini2;
	ini2.ReadFile( sIniFile2 );
	FOREACH_CONST_Child( &ini1, key1 )
	{
		const XNode *key2 = ini2.GetChild( key1->m_sName );
		if( key2 == NULL )
			continue;
		FOREACH_CONST_Attr( key1, attr1 )
		{
			if( key2->GetAttr(attr1->first) == NULL)
				continue;
			count++;
		}
	}
	return count;
}

void LanguagesDlg::OnSelchangeListLanguages() 
{
	// TODO: Add your control notification handler code here
	int iTotalStrings = -1;
	int iNeedTranslation = -1;

	RString sTheme = GetCurrentString( m_listThemes );
	RString sLanguage = GetCurrentString( m_listLanguages );

	if( !sTheme.empty() )
	{
		RString sBaseLanguageFile = GetLanguageFile( sTheme, SpecialFiles::BASE_LANGUAGE );
		iTotalStrings = GetNumValuesInIniFile( sBaseLanguageFile );

		if( !sLanguage.empty() )
		{
			sLanguage = SMPackageUtil::GetLanguageCodeFromDisplayString( sLanguage );

			RString sLanguageFile = GetLanguageFile( sTheme, sLanguage );
			iNeedTranslation = iTotalStrings - GetNumIntersectingIniValues( sBaseLanguageFile, sLanguageFile );
		}
	}

	GetDlgItem(IDC_STATIC_TOTAL_STRINGS		)->SetWindowText( ssprintf(iTotalStrings==-1?"":"%d",iTotalStrings) ); 
	GetDlgItem(IDC_STATIC_NEED_TRANSLATION	)->SetWindowText( ssprintf(iNeedTranslation==-1?"":"%d",iNeedTranslation) ); 

	GetDlgItem(IDC_BUTTON_CREATE)->EnableWindow( !sTheme.empty() ); 
	GetDlgItem(IDC_BUTTON_DELETE)->EnableWindow( !sLanguage.empty() ); 
	GetDlgItem(IDC_BUTTON_EXPORT)->EnableWindow( !sLanguage.empty() ); 
	GetDlgItem(IDC_BUTTON_IMPORT)->EnableWindow( !sLanguage.empty() );
}


BEGIN_MESSAGE_MAP(LanguagesDlg, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_THEMES, OnSelchangeListThemes)
	ON_LBN_SELCHANGE(IDC_LIST_LANGUAGES, OnSelchangeListLanguages)
	ON_BN_CLICKED(IDC_BUTTON_CREATE, OnBnClickedButtonCreate)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnBnClickedButtonDelete)
END_MESSAGE_MAP()


// LanguagesDlg message handlers

void LanguagesDlg::OnBnClickedButtonCreate()
{
	// TODO: Add your control notification handler code here
	CreateLanguageDlg dlg;
	int nResponse = dlg.DoModal();
	if( nResponse != IDOK )
		return;

	RString sTheme = GetCurrentString( m_listThemes );
	ASSERT( !sTheme.empty() );

	RString sLanguageFile = GetLanguageFile( sTheme, RString(dlg.m_sChosenLanguageCode) );

	// create empty file
	RageFile file;
	file.Open( sLanguageFile, RageFile::WRITE );
	file.Close();	// flush file

	FlushDirCache();

	OnSelchangeListThemes();
	SelectString( m_listLanguages, SMPackageUtil::GetLanguageDisplayString(RString(dlg.m_sChosenLanguageCode)) );
	OnSelchangeListLanguages();
}

void LanguagesDlg::OnBnClickedButtonDelete()
{
	// TODO: Add your control notification handler code here
	RString sTheme = GetCurrentString( m_listThemes );
	ASSERT( !sTheme.empty() );
	RString sLanguage = GetCurrentString( m_listLanguages );
	ASSERT( !sLanguage.empty() );
	sLanguage = SMPackageUtil::GetLanguageCodeFromDisplayString( sLanguage );

	RString sLanguageFile = GetLanguageFile( sTheme, sLanguage );
	FILEMAN->Remove( sLanguageFile );
	FlushDirCache();

	OnSelchangeListThemes();
}
