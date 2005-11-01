// onvertThemeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smpackage.h"
#include "onvertThemeDlg.h"
#include "smpackageUtil.h"	
#include "EditMetricsDlg.h"	
#include "EditMetricsDlg.h"	
#include "IniFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ConvertThemeDlg dialog


ConvertThemeDlg::ConvertThemeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(ConvertThemeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(ConvertThemeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void ConvertThemeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ConvertThemeDlg)
	DDX_Control(pDX, IDC_BUTTON_ANALYZE_METRICS, m_buttonAnalyzeMetrics);
	DDX_Control(pDX, IDC_BUTTON_EDIT_METRICS, m_buttonEditMetrics);
	DDX_Control(pDX, IDC_BUTTON_ANALYZE, m_buttonAnalyze);
	DDX_Control(pDX, IDC_BUTTON_CONVERT, m_buttonConvert);
	DDX_Control(pDX, IDC_LIST_THEMES, m_listThemes);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ConvertThemeDlg, CDialog)
	//{{AFX_MSG_MAP(ConvertThemeDlg)
	ON_LBN_SELCHANGE(IDC_LIST_THEMES, OnSelchangeListThemes)
	ON_BN_CLICKED(IDC_BUTTON_ANALYZE, OnButtonAnalyze)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_METRICS, OnButtonEditMetrics)
	ON_BN_CLICKED(IDC_BUTTON_ANALYZE_METRICS, OnButtonAnalyzeMetrics)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ConvertThemeDlg message handlers

BOOL ConvertThemeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	CStringArray asThemes;
	GetDirListing( "Themes\\*.*", asThemes, true, false );
	for( unsigned i=0; i<asThemes.size(); i++ )
//		if( asThemes[i] != "default" )	// allow editing of default
			m_listThemes.AddString( asThemes[i] );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// renames files and directories recursively
void RecursiveRename( CString sDirStart, CString sOld, CString sNew )
{
	if( sDirStart.Right(1) != "\\" )
		sDirStart += "\\";

	CStringArray asFilesAndDirs;
	GetDirListing( sDirStart+"*.*", asFilesAndDirs, false, false );
	for( unsigned i=0; i<asFilesAndDirs.size(); i++ )
	{
		CString sOldFilePath = sDirStart+asFilesAndDirs[i];
		CString sNewFilePath = sOldFilePath;
		sNewFilePath.MakeLower();
		sOld.MakeLower();
		sNewFilePath.Replace( sOld, sNew );
		if( sOldFilePath.CompareNoCase(sNewFilePath) != 0 )
			MoveFile( sOldFilePath, sNewFilePath );
	}

	CStringArray asDirs;
	GetDirListing( sDirStart+"*.*", asDirs, true, false );
	for( i=0; i<asDirs.size(); i++ )
		RecursiveRename( sDirStart+asDirs[i], sOld, sNew );
}

void ConvertThemeDlg::OnSelchangeListThemes() 
{
	// TODO: Add your control notification handler code here
	BOOL bSomethingSelected = m_listThemes.GetCurSel() != LB_ERR;
	m_buttonConvert.EnableWindow( bSomethingSelected );
	m_buttonAnalyze.EnableWindow( bSomethingSelected );
	m_buttonEditMetrics.EnableWindow( bSomethingSelected );
	m_buttonAnalyzeMetrics.EnableWindow( bSomethingSelected );
}

bool FilesAreIdentical( CString sPath1, CString sPath2 )
{
	if( IsADirectory(sPath1) || IsADirectory(sPath2) )
		return false;

	FILE* fp1 = fopen( sPath1, "r" );
	FILE* fp2 = fopen( sPath2, "r" );

	bool bReturn;

	if( GetFileSizeInBytes(sPath1) != GetFileSizeInBytes(sPath2) )
	{
		bReturn = false;
		goto clean_up;
	}

	char buffer1[1024], buffer2[1024];
	ZERO( buffer1 );
	ZERO( buffer2 );
	while( !feof(fp1) )
	{
		int size1 = fread( buffer1, 1, sizeof(buffer1), fp1 );
		int size2 = fread( buffer2, 1, sizeof(buffer2), fp2 );
		if( size1 != size2 )
		{
			bReturn = false;
			goto clean_up;
		}
		if( memcmp( buffer1, buffer2, sizeof(size1) ) != 0 )
		{
			bReturn = false;
			goto clean_up;
		}
	}

	bReturn = true;
	goto clean_up;

clean_up:
	fclose( fp1 );
	fclose( fp2 );
	return bReturn;
}

CString StripExtension( CString sPath )
{
	CString sDir, sFName, sExt;
	splitrelpath( sPath, sDir, sFName, sExt );
	return sDir + sFName;
}

void LaunchNotepad( CString sPathToOpen )
{
	PROCESS_INFORMATION pi;
	STARTUPINFO	si;
	ZeroMemory( &si, sizeof(si) );

	char szCommand[MAX_PATH] = "notepad.exe ";
	strcat( szCommand, sPathToOpen );
	CreateProcess(
		NULL,		// pointer to name of executable module
		szCommand,		// pointer to command line string
		NULL,  // process security attributes
		NULL,   // thread security attributes
		false,  // handle inheritance flag
		0, // creation flags
		NULL,  // pointer to new environment block
		NULL,   // pointer to current directory name
		&si,  // pointer to STARTUPINFO
		&pi  // pointer to PROCESS_INFORMATION
	);
}

void ConvertThemeDlg::OnButtonAnalyze() 
{
	CString sBaseDir = "Themes\\default\\";
	int iSel = m_listThemes.GetCurSel();
	CString sThemeName;
	m_listThemes.GetText( iSel, sThemeName );
	CString sThemeDir = "Themes\\"+sThemeName+"\\";

	CStringArray asBaseFilePaths;
	GetDirListing( sBaseDir+"BGAnimations\\*.*", asBaseFilePaths, false, true );
	GetDirListing( sBaseDir+"Fonts\\*.*", asBaseFilePaths, false, true );
	GetDirListing( sBaseDir+"Graphics\\*.*", asBaseFilePaths, false, true );
	GetDirListing( sBaseDir+"Numbers\\*.*", asBaseFilePaths, false, true );
	GetDirListing( sBaseDir+"Sounds\\*.*", asBaseFilePaths, false, true );

	CStringArray asThemeFilePaths;
	GetDirListing( sThemeDir+"BGAnimations\\*.*", asThemeFilePaths, false, true );
	GetDirListing( sThemeDir+"Fonts\\*.*", asThemeFilePaths, false, true );
	GetDirListing( sThemeDir+"Graphics\\*.*", asThemeFilePaths, false, true );
	GetDirListing( sThemeDir+"Numbers\\*.*", asThemeFilePaths, false, true );
	GetDirListing( sThemeDir+"Sounds\\*.*", asThemeFilePaths, false, true );

	CStringArray asRedundant;
	CStringArray asWarning;
	unsigned i;
	for( i=0; i<asThemeFilePaths.size(); i++ )
	{
		CString sThemeElement = asThemeFilePaths[i];
		sThemeElement.Replace( sThemeDir, "" );
		sThemeElement = StripExtension( sThemeElement );
		bool bFoundMatch = false;

		for( unsigned j=0; j<asBaseFilePaths.size(); j++ )
		{
			CString sBaseElement = asBaseFilePaths[j];
			sBaseElement.Replace( sBaseDir, "" );
			sBaseElement = StripExtension( sBaseElement );

			if( sThemeElement.CompareNoCase(sBaseElement)==0 )	// file names match
			{
				bFoundMatch = true;
 				if( FilesAreIdentical( asThemeFilePaths[i], asBaseFilePaths[j] ) )
					asRedundant.push_back( asThemeFilePaths[i] );
				break;	// skip to next file in asThemeFilePaths
			}
		}
		if( !bFoundMatch )
			asWarning.push_back( asThemeFilePaths[i] );
	}

	SortCStringArray( asRedundant );
	SortCStringArray( asWarning );

	FILE* fp = fopen( "elements_report.txt", "w" );
	ASSERT( fp );
	fprintf( fp, "Theme elements report for '"+sThemeName+"'.\n\n" );
	fprintf( fp, "The following elements are REDUNDANT.\n"
		"    (These elements are identical to the elements in the base theme.\n"
		"    They are unnecessary and should be deleted.)\n" );
	for( i=0; i<asRedundant.size(); i++ )
		fprintf( fp, asRedundant[i] + "\n" );
	fprintf( fp, "\n" );
	fprintf( fp, "The following elements are possibly MISNAMED.\n"
		"    (These files do not have a corresponding element in the base theme.\n"
		"    This likely means that there is an error in the file name.)\n" );
	for( i=0; i<asWarning.size(); i++ )
		fprintf( fp, asWarning[i] + "\n" );
	fclose( fp );

	LaunchNotepad( "elements_report.txt" );
}

void ConvertThemeDlg::OnButtonEditMetrics() 
{
	// TODO: Add your control notification handler code here
	EditMetricsDlg dlg;
	int iSel = m_listThemes.GetCurSel();
	CString sThemeName;
	m_listThemes.GetText( iSel, sThemeName );
	dlg.m_sTheme = sThemeName;
	int nResponse = dlg.DoModal();	
	
}

void ConvertThemeDlg::OnButtonAnalyzeMetrics() 
{
	int iSel = m_listThemes.GetCurSel();
	CString sThemeName;
	m_listThemes.GetText( iSel, sThemeName );

	IniFile iniBase;
	iniBase.SetPath( "Themes\\default\\metrics.ini" );
	iniBase.ReadFile();

	IniFile iniTheme;
	iniTheme.SetPath( "Themes\\"+sThemeName+"\\metrics.ini" );
	iniTheme.ReadFile();

	CMapStringToString mapBaseClassPlusNameToValue;
	unsigned i;
	IniFile::const_iterator it;
	for( it = iniBase.begin(); it != iniBase.end(); ++it )
	{
		CString sKey = it->first;
		const IniFile::key &Key = it->second;
		for( IniFile::key::const_iterator val = Key.begin(); val != Key.end(); ++val )
		{
			CString sName = val->first, sValue = val->second;
			mapBaseClassPlusNameToValue[sKey+"-"+sName] = sValue;
		}
	}

	CMapStringToString mapThemeClassPlusNameToValue;
	for( it = iniTheme.begin(); it != iniTheme.end(); ++it )
	{
		CString sKey = it->first;
		const IniFile::key &Key = it->second;
		for( IniFile::key::const_iterator val = Key.begin(); val != Key.end(); ++val )
		{
			CString sName = val->first, sValue = val->second;
			mapThemeClassPlusNameToValue[sKey+"-"+sName] = sValue;
		}
	}


	CStringArray asRedundant;
	CStringArray asWarning;
	for( POSITION pos1=mapThemeClassPlusNameToValue.GetStartPosition(); pos1!=NULL; )
	{
		CString sThemeKey, sThemeValue;
		mapThemeClassPlusNameToValue.GetNextAssoc( pos1, sThemeKey, sThemeValue );
		bool bFoundMatch = false;

		for( POSITION pos2=mapBaseClassPlusNameToValue.GetStartPosition(); pos2!=NULL; )
		{
			CString sBaseKey, sBaseValue;
			mapBaseClassPlusNameToValue.GetNextAssoc( pos2, sBaseKey, sBaseValue );

			if( sThemeKey == sBaseKey )	// match
			{
				bFoundMatch = true;
				if( sThemeValue == sBaseValue )
					asRedundant.push_back( sThemeKey );
				break;	// skip to next file in asThemeFilePaths
			}
		}
		if( !bFoundMatch )
			asWarning.push_back( sThemeKey );
	}

	SortCStringArray( asRedundant );
	SortCStringArray( asWarning );

	FILE* fp = fopen( "metrics_report.txt", "w" );
	ASSERT( fp );
	fprintf( fp, "Theme metrics report for '"+sThemeName+"'.\n\n" );
	fprintf( fp, "The following metrics are REDUNDANT.\n"
		"    (These metrics are identical to the metrics in the base theme.\n"
		"    They are unnecessary and should be deleted.)\n" );
	for( i=0; i<asRedundant.size(); i++ )
		fprintf( fp, asRedundant[i] + "\n" );
	fprintf( fp, "\n" );
	fprintf( fp, "The following elements are possibly MISNAMED.\n"
		"    (These metrics do not have a corresponding metric in\n"
		"    the base theme.  This likely means that there is an error in the metric name.)\n" );
	for( i=0; i<asWarning.size(); i++ )
		fprintf( fp, asWarning[i] + "\n" );
	fclose( fp );

	LaunchNotepad( "metrics_report.txt" );	
}
