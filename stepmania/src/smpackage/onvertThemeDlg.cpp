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
	ON_BN_CLICKED(IDC_BUTTON_CONVERT, OnButtonConvert)
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

void ConvertThemeDlg::OnButtonConvert() 
{
	// TODO: Add your control notification handler code here
	int iSel = m_listThemes.GetCurSel();
	CString sThemeDir;
	m_listThemes.GetText( iSel, sThemeDir );
	sThemeDir = "Themes\\" + sThemeDir;

	RecursiveRename( sThemeDir, "gameplay cleared", "______OBSOLETE gameplay cleared" );
	RecursiveRename( sThemeDir, "gameplay closing star", "______OBSOLETE gameplay closing star" );
	RecursiveRename( sThemeDir, "gameplay opening star", "______OBSOLETE gameplay opening star" );
	RecursiveRename( sThemeDir, "gameplay here we go", "______OBSOLETE gameplay here we go" );
	RecursiveRename( sThemeDir, "gameplay failed", "______OBSOLETE gameplay failed" );
	RecursiveRename( sThemeDir, "gameplay toasty", "______OBSOLETE gameplay toasty" );
	RecursiveRename( sThemeDir, "gameplay ready", "______OBSOLETE gameplay ready" );
	RecursiveRename( sThemeDir, "keep alive", "______OBSOLETE keep alive" );
	RecursiveRename( sThemeDir, "menu style icons 1x9", "______OBSOLETE menu style icons 1x9" );
	RecursiveRename( sThemeDir, "game over", "______OBSOLETE game over" );
	RecursiveRename( sThemeDir, "stage extra1", "______OBSOLETE stage extra1" );
	RecursiveRename( sThemeDir, "stage extra2", "______OBSOLETE stage extra2" );
	RecursiveRename( sThemeDir, "stage final", "______OBSOLETE stage final" );
	RecursiveRename( sThemeDir, "stage oni", "______OBSOLETE stage oni" );
	RecursiveRename( sThemeDir, "stage parts 5x3", "______OBSOLETE stage parts 5x3" );
	RecursiveRename( sThemeDir, "stage stage", "______OBSOLETE stage stage" );
	RecursiveRename( sThemeDir, "edit background", "______OBSOLETE edit background" );
	RecursiveRename( sThemeDir, "Fonts\\bpmdisplay", "Fonts\\______OBSOLETE bpmdisplay" );
	RecursiveRename( sThemeDir, "Fonts\\evaluation new record", "Fonts\\______OBSOLETE evaluation new record" );
	RecursiveRename( sThemeDir, "Fonts\\evaluation stage", "Fonts\\______OBSOLETE evaluation stage" );
	RecursiveRename( sThemeDir, "Fonts\\gameplay stage", "Fonts\\______OBSOLETE gameplay stage" );
	RecursiveRename( sThemeDir, "Fonts\\normal2", "Fonts\\______OBSOLETE normal2" );
	RecursiveRename( sThemeDir, "Fonts\\select course num songs", "Fonts\\______OBSOLETE select course num songs" );
	RecursiveRename( sThemeDir, "Fonts\\select course total time", "Fonts\\______OBSOLETE select course total time" );
	RecursiveRename( sThemeDir, "Fonts\\select group num songs", "Fonts\\______OBSOLETE select group num songs" );
	RecursiveRename( sThemeDir, "Fonts\\select music hold", "Fonts\\______OBSOLETE select music hold" );
	RecursiveRename( sThemeDir, "Fonts\\select music stage", "Fonts\\______OBSOLETE select music stage" );
	RecursiveRename( sThemeDir, "Fonts\\small titles", "Fonts\\______OBSOLETE small titles" );
	RecursiveRename( sThemeDir, "Fonts\\stage", "Fonts\\______OBSOLETE stage" );
	RecursiveRename( sThemeDir, "Fonts\\survive time", "Fonts\\______OBSOLETE survive time" );
	RecursiveRename( sThemeDir, "menu style icons", "______OBSOLETE menu style icons" );
	RecursiveRename( sThemeDir, "music status icons", "______OBSOLETE music status icons" );
	RecursiveRename( sThemeDir, "ScreenSelectMusic music", "______OBSOLETE ScreenSelectMusic music" );
	RecursiveRename( sThemeDir, "title menu logo", "ScreenLogo" );
	RecursiveRename( sThemeDir, "BGAnimations\\appearance options", "BGAnimations\\ScreenAppearanceOptions background" );
	RecursiveRename( sThemeDir, "BGAnimations\\caution",			"BGAnimations\\ScreenCaution background" );
	RecursiveRename( sThemeDir, "BGAnimations\\game over",			"BGAnimations\\ScreenGameOver background" );
	RecursiveRename( sThemeDir, "BGAnimations\\logo",				"BGAnimations\\ScreenLogo background" );
	RecursiveRename( sThemeDir, "BGAnimations\\edit menu",			"BGAnimations\\ScreenEditMenu background" );
	RecursiveRename( sThemeDir, "BGAnimations\\evaluation",			"BGAnimations\\ScreenEvaluation background" );
	RecursiveRename( sThemeDir, "BGAnimations\\gameplay options",	"BGAnimations\\ScreenGameplayOptions background" );
	RecursiveRename( sThemeDir, "BGAnimations\\edit menu",			"BGAnimations\\ScreenEditMenu background" );
	RecursiveRename( sThemeDir, "BGAnimations\\game options",		"BGAnimations\\ScreenGameOptions background" );
	RecursiveRename( sThemeDir, "BGAnimations\\title menu",			"BGAnimations\\ScreenLogo background" );
	RecursiveRename( sThemeDir, "evaluation summary top edge",		"ScreenEvaluationSummary header" );
	RecursiveRename( sThemeDir, "top edge", "header" );
	RecursiveRename( sThemeDir, "\\edit menu", "\\ScreenEditMenu" );
	RecursiveRename( sThemeDir, "\\evaluation", "\\ScreenEvaluation" );
	RecursiveRename( sThemeDir, "fallback banner", "Common fallback banner" );
	RecursiveRename( sThemeDir, "Fonts\\music scroll", "Fonts\\ScreenMusicScroll titles" );
	RecursiveRename( sThemeDir, "fallback background", "Common fallback background" );
	RecursiveRename( sThemeDir, "all music banner", "Banner all" );
	RecursiveRename( sThemeDir, "try extra stage", "try extra" );
	RecursiveRename( sThemeDir, "fallback cd title", "ScreenSelectMusic fallback cdtitle" );
	RecursiveRename( sThemeDir, "game options", "ScreenGameOptions" );
	RecursiveRename( sThemeDir, "gameplay combo label", "Combo label" );
	RecursiveRename( sThemeDir, "gameplay difficulty icons", "ScreenGameplay difficulty icons" );
	RecursiveRename( sThemeDir, "gameplay extra life frame", "ScreenGameplay extra life frame" );
	RecursiveRename( sThemeDir, "gameplay extra lifemeter bar", "LifeMeterBar extra frame" );
	RecursiveRename( sThemeDir, "gameplay extra lifemeter stream hot", "LifeMeterBar extra hot" );
	RecursiveRename( sThemeDir, "gameplay extra lifemeter stream normal", "LifeMeterBar extra normal" );
	RecursiveRename( sThemeDir, "gameplay lifemeter bar", "LifeMeterBar frame" );
	RecursiveRename( sThemeDir, "gameplay lifemeter battery 1x4", "LifeMeterBattery lives 1x4" );
	RecursiveRename( sThemeDir, "gameplay lifemeter oni", "LifeMeterBattery frame" );
	RecursiveRename( sThemeDir, "gameplay", "ScreenGameplay" );
	RecursiveRename( sThemeDir, "graphic options", "ScreenGraphicOptions" );
	RecursiveRename( sThemeDir, "input options", "ScreenInputOptions" );
	RecursiveRename( sThemeDir, "instructions", "ScreenInstructions" );
	RecursiveRename( sThemeDir, "machine options", "ScreenMachineOptions" );
	RecursiveRename( sThemeDir, "map controllers", "ScreenMapControllers" );
	RecursiveRename( sThemeDir, "menu bottom edge", "_shared footer" );
	RecursiveRename( sThemeDir, "music sort icons 1x4", "MusicSortDisplay icons 1x4" );
	RecursiveRename( sThemeDir, "options arrow", "ScreenOptions bullet" );
	RecursiveRename( sThemeDir, "options cursor", "OptionsCursor cursor" );
	RecursiveRename( sThemeDir, "options underline", "OptionsCursor underline" );
	RecursiveRename( sThemeDir, "player options", "ScreenPlayerOptions" );
	RecursiveRename( sThemeDir, "select course content bar", "CourseEntryDisplay bar" );
	RecursiveRename( sThemeDir, "select course", "ScreenSelectCourse" );
	RecursiveRename( sThemeDir, "select difficulty easy header", "ScreenSelectDifficulty info arcade-easy" );
	RecursiveRename( sThemeDir, "select difficulty easy picture", "ScreenSelectDifficulty picture arcade-easy" );
	RecursiveRename( sThemeDir, "select difficulty medium header", "ScreenSelectDifficulty info arcade-medium" );
	RecursiveRename( sThemeDir, "select difficulty medium picture", "ScreenSelectDifficulty picture arcade-medium" );
	RecursiveRename( sThemeDir, "select difficulty hard header", "ScreenSelectDifficulty info arcade-hard" );
	RecursiveRename( sThemeDir, "select difficulty hard picture", "ScreenSelectDifficulty picture arcade-hard" );
	RecursiveRename( sThemeDir, "select difficulty oni header", "ScreenSelectDifficulty info oni" );
	RecursiveRename( sThemeDir, "select difficulty oni picture", "ScreenSelectDifficulty picture oni" );
	RecursiveRename( sThemeDir, "select difficulty endless header", "ScreenSelectDifficulty info endless" );
	RecursiveRename( sThemeDir, "select difficulty endless picture", "ScreenSelectDifficulty picture endless" );
	RecursiveRename( sThemeDir, "select difficulty", "ScreenSelectDifficulty" );
	RecursiveRename( sThemeDir, "select game", "ScreenSelectGame" );
	RecursiveRename( sThemeDir, "select group button", "GroupList bar" );
	RecursiveRename( sThemeDir, "select group contents header", "ScreenSelectGroup contents" );
	RecursiveRename( sThemeDir, "select group info frame", "ScreenSelectGroup frame" );
	RecursiveRename( sThemeDir, "select group", "ScreenSelectGroup" );
	RecursiveRename( sThemeDir, "select music meter 2x1", "DifficultyMeter bar 2x1" );
	RecursiveRename( sThemeDir, "select music option icons 3x2", "OptionIcon frame 3x2" );
	RecursiveRename( sThemeDir, "select music radar base", "GrooveRadar base" );
	RecursiveRename( sThemeDir, "select music radar labels 1x5", "GrooveRadar labels 1x5" );
	RecursiveRename( sThemeDir, "select music roulette banner", "Banner roulette" );
	RecursiveRename( sThemeDir, "select music scrollbar parts 1x3", "ScrollBar parts 1x3" );
	RecursiveRename( sThemeDir, "select music scrollbar thumb", "ScrollBar thumb" );
	RecursiveRename( sThemeDir, "select music section banner", "Banner abc" );
	RecursiveRename( sThemeDir, "select music Section bar", "MusicWheelItem section" );
	RecursiveRename( sThemeDir, "select music small grades", "SmallGradeDisplay grades" );
	RecursiveRename( sThemeDir, "select music song bar", "MusicWheelItem song" );
	RecursiveRename( sThemeDir, "select music", "ScreenSelectMusic" );
	RecursiveRename( sThemeDir, "select player", "ScreenSelectPlayer" );
	RecursiveRename( sThemeDir, "info dance", "info" );
	RecursiveRename( sThemeDir, "preview dance", "picture" );
	RecursiveRename( sThemeDir, "preview", "picture" );
	RecursiveRename( sThemeDir, "select style", "ScreenSelectStyle" );
	RecursiveRename( sThemeDir, "song options", "ScreenSongOptions" );
	RecursiveRename( sThemeDir, "menu timer", "MenuTimer" );
	RecursiveRename( sThemeDir, "appearance options", "ScreenAppearanceOptions" );
	RecursiveRename( sThemeDir, "music scroll", "ScreenMusicScroll" );
	RecursiveRename( sThemeDir, "how to play", "ScreenInstructions" );
	RecursiveRename( sThemeDir, "background background", "background" );
	RecursiveRename( sThemeDir, "credits", "ScreenManager credits" );
	RecursiveRename( sThemeDir, "header1", "_shared1" );
	RecursiveRename( sThemeDir, "header2", "_shared2" );
	RecursiveRename( sThemeDir, "header3", "_shared3" );
	RecursiveRename( sThemeDir, "header4", "_shared4" );
	RecursiveRename( sThemeDir, "Fonts\\help", "Fonts\\Common help" );
	RecursiveRename( sThemeDir, "Fonts\\musicwheel text banner", "Fonts\\TextBanner" );
	RecursiveRename( sThemeDir, "Fonts\\normal", "Fonts\\Common normal" );
	RecursiveRename( sThemeDir, "Fonts\\option icons", "Fonts\\OptionIcon" );
	RecursiveRename( sThemeDir, "Fonts\\option explanation", "Fonts\\ScreenOptions explanation" );
	RecursiveRename( sThemeDir, "Fonts\\option item", "Fonts\\ScreenOptions item" );
	RecursiveRename( sThemeDir, "Fonts\\option title", "Fonts\\ScreenOptions title" );
	RecursiveRename( sThemeDir, "Fonts\\select course num songs", "Fonts\\ScreenOptions title" );
	RecursiveRename( sThemeDir, "Fonts\\select group button label", "Fonts\\GroupList label" );
	RecursiveRename( sThemeDir, "Fonts\\selectmusicsection", "Fonts\\MusicWheel section" );
	RecursiveRename( sThemeDir, "Fonts\\titlemenu", "Fonts\\ScreenTitleMenu choices" );
	RecursiveRename( sThemeDir, "numbers 5x3", "5x3" );
	RecursiveRename( sThemeDir, " (dither)", "" );
	RecursiveRename( sThemeDir, " (1alpha)", "" );
	RecursiveRename( sThemeDir, " (1 alpha)", "" );
	RecursiveRename( sThemeDir, " (0 alpha)", "" );
	RecursiveRename( sThemeDir, "edit header", "ScreenEdit header" );

	AfxMessageBox( "Conversion Complete!" );
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
