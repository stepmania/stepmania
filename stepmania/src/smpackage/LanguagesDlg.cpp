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
#include ".\languagesdlg.h"
#include "CsvFile.h"

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
	DDX_Control(pDX, IDC_CHECK_EXPORT_ALREADY_TRANSLATED, m_buttonExportAlreadyTranslated);
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
	GetDlgItem(IDC_CHECK_EXPORT_ALREADY_TRANSLATED)->EnableWindow( !sLanguage.empty() ); 
}


BEGIN_MESSAGE_MAP(LanguagesDlg, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_THEMES, OnSelchangeListThemes)
	ON_LBN_SELCHANGE(IDC_LIST_LANGUAGES, OnSelchangeListLanguages)
	ON_BN_CLICKED(IDC_BUTTON_CREATE, OnBnClickedButtonCreate)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnBnClickedButtonDelete)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT, OnBnClickedButtonExport)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT, OnBnClickedButtonImport)
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

	int iRet = MessageBox( ssprintf("This will permanently delete '%s'. Continue?",sLanguageFile.c_str()), NULL, MB_OKCANCEL );
	if( iRet != IDOK )
		return;

	FILEMAN->Remove( sLanguageFile );
	FlushDirCache();

	OnSelchangeListThemes();
}

struct TranslationLine
{
	RString sSection, sID, sBaseLanguage, sCurrentLanguage;
};

void LanguagesDlg::OnBnClickedButtonExport()
{
	// TODO: Add your control notification handler code here
	RString sTheme = GetCurrentString( m_listThemes );
	ASSERT( !sTheme.empty() );
	RString sLanguage = GetCurrentString( m_listLanguages );
	ASSERT( !sLanguage.empty() );
	sLanguage = SMPackageUtil::GetLanguageCodeFromDisplayString( sLanguage );

	RString sBaseLanguageFile = GetLanguageFile( sTheme, SpecialFiles::BASE_LANGUAGE );
	RString sLanguageFile = GetLanguageFile( sTheme, sLanguage );

	bool bExportAlreadyTranslated = !!m_buttonExportAlreadyTranslated.GetCheck();

	CsvFile csv;
	IniFile ini1;
	ini1.ReadFile( sBaseLanguageFile );
	IniFile ini2;
	ini2.ReadFile( sLanguageFile );
	int iNumExpored = 0;
	FOREACH_CONST_Child( &ini1, key )
	{
		FOREACH_CONST_Attr( key, value )
		{
			TranslationLine tl;
			tl.sSection = key->m_sName;
			tl.sID = value->first;
			tl.sBaseLanguage = value->second;
			ini2.GetValue( tl.sSection, tl.sID, tl.sCurrentLanguage );

			if( tl.sCurrentLanguage.empty() || bExportAlreadyTranslated )
			{
				vector<RString> vs;
				vs.push_back( tl.sSection );
				vs.push_back( tl.sID );
				vs.push_back( tl.sBaseLanguage );
				vs.push_back( tl.sCurrentLanguage );
				csv.m_vvs.push_back( vs );
				iNumExpored++;
			}
		}
	}
	RString sFile = sTheme+"-"+sLanguage+".csv";
	RString sFullFile = "Desktop/"+sFile;
	if( iNumExpored == 0 )
	{
		MessageBox( "There are no strings to export for this language." );	
		return;
	}
	if( csv.WriteFile(sFullFile) )
		MessageBox( ssprintf("Exported to your Desktop as '%s'.",sFile.c_str()) );
	else
		MessageBox( ssprintf("Failed to save '%s'.",sFullFile.c_str()) );
}

void LanguagesDlg::OnBnClickedButtonImport()
{
	// TODO: Add your control notification handler code here
	CFileDialog dialog (
		TRUE,	// file open?
		NULL,	// default file extension
		NULL,	// default file name
		OFN_HIDEREADONLY | OFN_NOCHANGEDIR,		// flags
		"CSV file (*.csv)|*.csv|||"
		);
	int iRet = dialog.DoModal();
	RString sCsvFile = dialog.GetPathName();
	if( iRet != IDOK )
		return;

	RString sTheme = GetCurrentString( m_listThemes );
	ASSERT( !sTheme.empty() );
	RString sLanguage = GetCurrentString( m_listLanguages );
	ASSERT( !sLanguage.empty() );
	sLanguage = SMPackageUtil::GetLanguageCodeFromDisplayString( sLanguage );

	RageFileOsAbsolute cvsFile;
	if( !cvsFile.Open(sCsvFile) )
	{
		MessageBox( ssprintf("Error reading file '%s'.",sCsvFile.c_str()) );
		return;
	}
	CsvFile csv;
	if( !csv.ReadFile(cvsFile) )
	{
		MessageBox( ssprintf("Error parsing file '%s'.",sCsvFile.c_str()) );
		return;
	}

	RString sLanguageFile = GetLanguageFile( sTheme, sLanguage );

	{
		int iRet = MessageBox( ssprintf("Importing these strings will override all data in '%s'. Continue?",sLanguageFile.c_str()), NULL, MB_OKCANCEL );
		if( iRet != IDOK )
			return;
	}

	IniFile ini;
	if( !ini.ReadFile(sLanguageFile) )
	{
		MessageBox( ssprintf("Error reading '%s'.",sLanguageFile.c_str()) );
		return;
	}

	int iNumImported = 0;
	int iNumIgnored = 0;
	FOREACH_CONST( CsvFile::StringVector, csv.m_vvs, line ) 
	{
		TranslationLine tl;
		int iNumValues = line->size();
		if( iNumValues != 3 && iNumValues != 4 )
		{
			MessageBox( ssprintf("Error reading '%s': Each line must have either 3 or 4 values.  This row has %d values",sCsvFile.c_str(),iNumValues) );
			return;
		}
		tl.sSection = (*line)[0];
		tl.sID = (*line)[1];
		tl.sBaseLanguage = (*line)[2];
		tl.sCurrentLanguage = iNumValues == 4 ? (*line)[3] : RString();

		if( tl.sCurrentLanguage.empty() )
		{
			iNumIgnored++;
		}
		else
		{
			ini.SetValue( tl.sSection, tl.sID, tl.sCurrentLanguage );
			iNumImported++;
		}
	}

	if( ini.WriteFile(sLanguageFile) )
		MessageBox( ssprintf("Imported %d strings into '%s'. %d empty strings were ignored.",iNumImported,sLanguageFile.c_str(),iNumIgnored) );
	else
		MessageBox( ssprintf("Failed to save '%s'.",sLanguageFile.c_str()) );
}
