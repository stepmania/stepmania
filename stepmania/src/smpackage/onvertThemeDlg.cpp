// onvertThemeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "smpackage.h"
#include "onvertThemeDlg.h"
#include "smpackageUtil.h"	

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
	DDX_Control(pDX, IDC_BUTTON_CONVERT, m_buttonConvert);
	DDX_Control(pDX, IDC_LIST_THEMES, m_listThemes);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ConvertThemeDlg, CDialog)
	//{{AFX_MSG_MAP(ConvertThemeDlg)
	ON_BN_CLICKED(IDC_BUTTON_CONVERT, OnButtonConvert)
	ON_LBN_SELCHANGE(IDC_LIST_THEMES, OnSelchangeListThemes)
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
	for( int i=0; i<asThemes.GetSize(); i++ )
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
	for( int i=0; i<asFilesAndDirs.GetSize(); i++ )
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
	for( i=0; i<asDirs.GetSize(); i++ )
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
	RecursiveRename( sThemeDir, "title menu logo", "ScreenLogo" );
	RecursiveRename( sThemeDir, "BGAnimations\\appearance options", "BGAnimations\\ScreenAppearanceOptions background" );
	RecursiveRename( sThemeDir, "BGAnimations\\caution",			"BGAnimations\\ScreenCaution background" );
	RecursiveRename( sThemeDir, "BGAnimations\\game over",			"BGAnimations\\ScreenGameOver background" );
	RecursiveRename( sThemeDir, "BGAnimations\\logo",				"BGAnimations\\ScreenLogo background" );
	RecursiveRename( sThemeDir, "BGAnimations\\edit menu",			"BGAnimations\\ScreenEditMenu background" );
	RecursiveRename( sThemeDir, "BGAnimations\\evaluation",			"BGAnimations\\ScreenEvaluation background" );
	RecursiveRename( sThemeDir, "BGAnimations\\gameplay options",	"BGAnimations\\ScreenGameplayOptions background" );
	RecursiveRename( sThemeDir, "evaluation summary top edge",		"ScreenEvaluationSummary header" );
	RecursiveRename( sThemeDir, "top edge", "header" );
	RecursiveRename( sThemeDir, "\\edit menu", "\\ScreenEditMenu" );
	RecursiveRename( sThemeDir, "\\evaluation", "\\ScreenEvaluation" );
	RecursiveRename( sThemeDir, "fallback banner", "Banner fallback" );
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
	RecursiveRename( sThemeDir, "select difficulty", "ScreenSelectDifficulty" );
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
	RecursiveRename( sThemeDir, "preview dance", "preview" );
	RecursiveRename( sThemeDir, "select style", "ScreenSelectStyle" );
	RecursiveRename( sThemeDir, "song options", "ScreenSongOptions" );
	RecursiveRename( sThemeDir, "menu timer", "MenuTimer" );
	RecursiveRename( sThemeDir, "appearance options", "ScreenAppearanceOptions" );
	RecursiveRename( sThemeDir, "music scroll", "ScreenMusicScroll" );

	AfxMessageBox( "Conversion Complete!" );
}

void ConvertThemeDlg::OnSelchangeListThemes() 
{
	// TODO: Add your control notification handler code here
	BOOL bSomethingSelected = m_listThemes.GetCurSel() != LB_ERR;
	m_buttonConvert.EnableWindow( bSomethingSelected );
}
