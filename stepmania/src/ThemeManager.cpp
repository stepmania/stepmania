#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ThemeManager

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ThemeManager.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "RageException.h"
#include "GameState.h"
#include "GameDef.h"
#include "IniFile.h"


ThemeManager*	THEME = NULL;	// global object accessable from anywhere in the program


const CString BASE_THEME_NAME = "default";
const CString THEMES_DIR  = "Themes\\";

ThemeManager::ThemeManager()
{
	m_pIniMetrics = new IniFile;
	m_iHashForCurThemeMetrics = -99;	// garbage value that will never match a real hash
	m_iHashForBaseThemeMetrics = -99;	// garbage value that will never match a real hash


	m_sCurThemeName = BASE_THEME_NAME;	// Use te base theme for now.  It's up to PrefsManager to change this.

	CStringArray arrayThemeNames;
	GetAllThemeNames( arrayThemeNames );

	// Disabled for now... it takes ages here to run - bbf
//	for( int i=0; i<arrayThemeNames.GetSize(); i++ )
//		AssertThemeIsComplete( arrayThemeNames[i] );
}

ThemeManager::~ThemeManager()
{
	delete m_pIniMetrics;
}

void ThemeManager::GetAllThemeNames( CStringArray& AddTo )
{
	GetDirListing( THEMES_DIR+"\\*", AddTo, true );
	
	// strip out the folder called "CVS"
	for( int i=AddTo.GetSize()-1; i>=0; i-- )
		if( 0 == stricmp(AddTo[i],"cvs") )
			AddTo.RemoveAt(i);
}

void ThemeManager::GetThemeNamesForCurGame( CStringArray& AddTo )
{
	GetAllThemeNames( AddTo );

	/*
	// strip out announcers that don't have the current game name in them
	CString sGameName = GAMESTATE->GetCurrentGameDef()->m_szName;
	sGameName.MakeLower();
	for( int i=AddTo.GetSize()-1; i>=0; i-- )
	{
		CString sLowercaseVer = AddTo[i];
		sLowercaseVer.MakeLower();
		if( sLowercaseVer.Find(sGameName)==-1 )
			AddTo.RemoveAt(i);
	}
	*/
}

bool ThemeManager::DoesThemeExist( CString sThemeName )
{
	CStringArray asThemeNames;	
	GetAllThemeNames( asThemeNames );
	for( int i=0; i<asThemeNames.GetSize(); i++ )
	{
		if( 0==stricmp(sThemeName, asThemeNames[i]) )
			return true;
	}
	return false;
}

void ThemeManager::AssertThemeIsComplete( CString sThemeName )
{
	// GetPathTo will assert if it can't find the file in the current theme dir or in the default dir.
	for( int e=0; e<NUM_THEME_ELEMENTS; e++ )
		GetPathTo( (ThemeElement)e, sThemeName );
}

void ThemeManager::SwitchTheme( CString sThemeName )
{
	if( 0==stricmp(BASE_THEME_NAME, sThemeName) )
		m_sCurThemeName = "";	// can't select the base theme
	else if( !DoesThemeExist(sThemeName) )
		m_sCurThemeName = BASE_THEME_NAME;
	else
		m_sCurThemeName = sThemeName;

	// update hashes for metrics files
	m_iHashForCurThemeMetrics = GetHashForFile( GetMetricsPathFromName(m_sCurThemeName) );
	m_iHashForBaseThemeMetrics = GetHashForFile( GetMetricsPathFromName(BASE_THEME_NAME) );

	// read new metrics.  First read base metrics, then read cur theme's metrics, overriding base theme
	m_pIniMetrics->Reset();
	m_pIniMetrics->SetPath( GetMetricsPathFromName(BASE_THEME_NAME) );
	m_pIniMetrics->ReadFile();
	m_pIniMetrics->SetPath( GetMetricsPathFromName(m_sCurThemeName) );
	m_pIniMetrics->ReadFile();
}

CString ThemeManager::GetThemeDirFromName( CString sThemeName )
{
	return THEMES_DIR + sThemeName + "\\";
}

CString ThemeManager::GetPathTo( ThemeElement te ) 
{
	return GetPathTo( te, m_sCurThemeName );
}

CString ThemeManager::GetPathTo( ThemeElement te, CString sThemeName )	// deprecated
{
	return "";
}
/*
CString ThemeManager::GetPathTo( ThemeElement te, CString sThemeName ) 
{
	CString sAssetCategory+"\\"+sFileName;		// fill this in below

	switch( te )
	{
		case GRAPHIC_ALL_MUSIC_BANNER:					sAssetCategory+"\\"+sFileName = "Graphics\\all music banner";				break;
		case GRAPHIC_ARROWS_LEFT:						sAssetCategory+"\\"+sFileName = "Graphics\\arrows left 1x4";					break;
		case GRAPHIC_ARROWS_RIGHT:						sAssetCategory+"\\"+sFileName = "Graphics\\arrows right 1x4";				break;
		case GRAPHIC_CAUTION:							sAssetCategory+"\\"+sFileName = "Graphics\\Caution";							break;
		case GRAPHIC_EDIT_BACKGROUND:					sAssetCategory+"\\"+sFileName = "Graphics\\edit background";					break;
		case GRAPHIC_EDIT_TOP_EDGE:						sAssetCategory+"\\"+sFileName = "Graphics\\edit top edge";					break;
		case GRAPHIC_EDIT_SNAP_INDICATOR:				sAssetCategory+"\\"+sFileName = "Graphics\\edit snap indicator";				break;
		case GRAPHIC_FALLBACK_BACKGROUND:				sAssetCategory+"\\"+sFileName = "Graphics\\Fallback Background";				break;
		case "Graphics","fallback banner":					sAssetCategory+"\\"+sFileName = "Graphics\\Fallback Banner";					break;
		case "Graphics","fallback cd title":					sAssetCategory+"\\"+sFileName = "Graphics\\Fallback CD Title";				break;
		case GRAPHIC_OPTIONS_CURSOR:					sAssetCategory+"\\"+sFileName = "Graphics\\options cursor";					break;
		case GRAPHIC_OPTIONS_UNDERLINE:					sAssetCategory+"\\"+sFileName = "Graphics\\options underline";				break;
		case GRAPHIC_GAME_OPTIONS_BACKGROUND:			sAssetCategory+"\\"+sFileName = "Graphics\\game options background";			break;
		case GRAPHIC_GAME_OPTIONS_TOP_EDGE:				sAssetCategory+"\\"+sFileName = "Graphics\\game options top edge";			break;
		case GRAPHIC_GAME_OVER:							sAssetCategory+"\\"+sFileName = "Graphics\\game over";						break;
		case GRAPHIC_GAMEPLAY_BOTTOM_FRAME:				sAssetCategory+"\\"+sFileName = "Graphics\\gameplay bottom frame";			break;
		case GRAPHIC_GAMEPLAY_CLEARED:					sAssetCategory+"\\"+sFileName = "Graphics\\gameplay cleared";				break;
		case GRAPHIC_GAMEPLAY_CLOSING_STAR:				sAssetCategory+"\\"+sFileName = "Graphics\\gameplay closing star";			break;
		case GRAPHIC_GAMEPLAY_COMBO:					sAssetCategory+"\\"+sFileName = "Graphics\\gameplay combo";					break;
		case GRAPHIC_GAMEPLAY_DANGER_BACKGROUND:		sAssetCategory+"\\"+sFileName = "Graphics\\gameplay danger background";		break;
		case GRAPHIC_GAMEPLAY_DANGER_TEXT:				sAssetCategory+"\\"+sFileName = "Graphics\\gameplay danger text";			break;
		case GRAPHIC_GAMEPLAY_DIFFICULTY_BANNERS:		sAssetCategory+"\\"+sFileName = "Graphics\\gameplay difficulty banners";		break;
		case GRAPHIC_GAMEPLAY_DEMONSTRATION:			sAssetCategory+"\\"+sFileName = "Graphics\\gameplay demonstration";			break;
		case GRAPHIC_GAMEPLAY_FAILED:					sAssetCategory+"\\"+sFileName = "Graphics\\gameplay failed";					break;
		case GRAPHIC_GAMEPLAY_HERE_WE_GO:				sAssetCategory+"\\"+sFileName = "Graphics\\gameplay here we go";				break;
		case GRAPHIC_GAMEPLAY_HOLD_JUDGEMENT:			sAssetCategory+"\\"+sFileName = "Graphics\\gameplay hold judgement 1x2";		break;
		case GRAPHIC_GAMEPLAY_JUDGEMENT:				sAssetCategory+"\\"+sFileName = "Graphics\\gameplay judgement 1x5";			break;
		case GRAPHIC_GAMEPLAY_OPENING_STAR:				sAssetCategory+"\\"+sFileName = "Graphics\\gameplay opening star";			break;
		case GRAPHIC_GAMEPLAY_READY:					sAssetCategory+"\\"+sFileName = "Graphics\\gameplay Ready";					break;
		case GRAPHIC_GAMEPLAY_MIDDLE_FRAME:				sAssetCategory+"\\"+sFileName = "Graphics\\gameplay middle frame";			break;
		case GRAPHIC_GAMEPLAY_TOP_FRAME:				sAssetCategory+"\\"+sFileName = "Graphics\\gameplay top frame";				break;
		case GRAPHIC_GAMEPLAY_LIFEMETER_BAR:			sAssetCategory+"\\"+sFileName = "Graphics\\gameplay lifemeter bar";			break;
		case GRAPHIC_GAMEPLAY_LIFEMETER_ONI:			sAssetCategory+"\\"+sFileName = "Graphics\\gameplay lifemeter oni";			break;
		case GRAPHIC_GAMEPLAY_LIFEMETER_BATTERY:		sAssetCategory+"\\"+sFileName = "Graphics\\gameplay lifemeter battery 1x4";	break;
		case GRAPHIC_GAMEPLAY_LIFEMETER_STREAM_NORMAL:	sAssetCategory+"\\"+sFileName = "Graphics\\gameplay lifemeter stream normal";break;
		case GRAPHIC_GAMEPLAY_LIFEMETER_STREAM_HOT:		sAssetCategory+"\\"+sFileName = "Graphics\\gameplay lifemeter stream hot";	break;		
		case GRAPHIC_GAMEPLAY_ONI_GAMEOVER:				sAssetCategory+"\\"+sFileName = "Graphics\\gameplay oni gameover";			break;		
		case GRAPHIC_GRAPHIC_OPTIONS_BACKGROUND:		sAssetCategory+"\\"+sFileName = "Graphics\\graphic options background";		break;
		case GRAPHIC_GRAPHIC_OPTIONS_TOP_EDGE:			sAssetCategory+"\\"+sFileName = "Graphics\\graphic options top edge";		break;
		case GRAPHIC_KEEP_ALIVE:						sAssetCategory+"\\"+sFileName = "Graphics\\keep alive";						break;
		case GRAPHIC_MAP_CONTROLLERS_BACKGROUND:		sAssetCategory+"\\"+sFileName = "Graphics\\map controllers background";		break;
		case GRAPHIC_MAP_CONTROLLERS_TOP_EDGE:			sAssetCategory+"\\"+sFileName = "Graphics\\map controllers top edge";		break;
		case GRAPHIC_MENU_BOTTOM_EDGE:					sAssetCategory+"\\"+sFileName = "Graphics\\menu bottom edge";				break;
		case GRAPHIC_MENU_STYLE_ICONS:					sAssetCategory+"\\"+sFileName = "Graphics\\menu style icons";				break;
		case GRAPHIC_MUSIC_SCROLL_BACKGROUND:			sAssetCategory+"\\"+sFileName = "Graphics\\music scroll background";			break;
		case GRAPHIC_MUSIC_SORT_ICONS:					sAssetCategory+"\\"+sFileName = "Graphics\\music sort icons 1x4";			break;
		case GRAPHIC_MUSIC_STATUS_ICONS:				sAssetCategory+"\\"+sFileName = "Graphics\\music status icons 1x4";			break;
		case GRAPHIC_PLAYER_OPTIONS_BACKGROUND:			sAssetCategory+"\\"+sFileName = "Graphics\\player options background";		break;
		case GRAPHIC_PLAYER_OPTIONS_TOP_EDGE:			sAssetCategory+"\\"+sFileName = "Graphics\\player options top edge";			break;
		case GRAPHIC_EVALUATION_BACKGROUND:				sAssetCategory+"\\"+sFileName = "Graphics\\evaluation background";			break;
		case GRAPHIC_EVALUATION_BANNER_FRAME:			sAssetCategory+"\\"+sFileName = "Graphics\\evaluation banner frame";			break;
		case GRAPHIC_EVALUATION_BONUS_FRAME_P1:			sAssetCategory+"\\"+sFileName = "Graphics\\evaluation bonus frame p1";		break;
		case GRAPHIC_EVALUATION_BONUS_FRAME_P2:			sAssetCategory+"\\"+sFileName = "Graphics\\evaluation bonus frame p2";		break;
		case GRAPHIC_EVALUATION_STAGE_FRAME_P1:			sAssetCategory+"\\"+sFileName = "Graphics\\evaluation stage frame p1";		break;
		case GRAPHIC_EVALUATION_STAGE_FRAME_P2:			sAssetCategory+"\\"+sFileName = "Graphics\\evaluation stage frame p2";		break;
		case GRAPHIC_EVALUATION_GRADE_FRAME:			sAssetCategory+"\\"+sFileName = "Graphics\\evaluation grade frame 1x2";		break;
		case GRAPHIC_EVALUATION_GRADES:					sAssetCategory+"\\"+sFileName = "Graphics\\evaluation grades 1x7";			break;
		case GRAPHIC_EVALUATION_JUDGE_LABELS:			sAssetCategory+"\\"+sFileName = "Graphics\\evaluation judge labels 1x6";		break;
		case GRAPHIC_EVALUATION_SCORE_LABELS:			sAssetCategory+"\\"+sFileName = "Graphics\\evaluation score labels 1x2";		break;
		case GRAPHIC_EVALUATION_SUMMARY_TOP_EDGE:		sAssetCategory+"\\"+sFileName = "Graphics\\evaluation summary top edge";		break;
		case GRAPHIC_EVALUATION_TOP_EDGE:				sAssetCategory+"\\"+sFileName = "Graphics\\evaluation top edge";				break;
		case GRAPHIC_SELECT_COURSE_INFO_FRAME:			sAssetCategory+"\\"+sFileName = "Graphics\\select course info frame";		break;
		case GRAPHIC_SELECT_COURSE_TOP_EDGE:			sAssetCategory+"\\"+sFileName = "Graphics\\select course top edge";			break;
		case GRAPHIC_SELECT_COURSE_BACKGROUND:			sAssetCategory+"\\"+sFileName = "Graphics\\select course background";		break;
		case GRAPHIC_SELECT_COURSE_CONTENT_BAR:			sAssetCategory+"\\"+sFileName = "Graphics\\select course content bar";		break;
		case GRAPHIC_SELECT_DIFFICULTY_ARROWS:			sAssetCategory+"\\"+sFileName = "Graphics\\select difficulty arrows 1x2";	break;
		case GRAPHIC_SELECT_DIFFICULTY_BACKGROUND:		sAssetCategory+"\\"+sFileName = "Graphics\\select difficulty background";	break;
		case GRAPHIC_SELECT_DIFFICULTY_EASY_HEADER:		sAssetCategory+"\\"+sFileName = "Graphics\\select difficulty easy header";	break;
		case GRAPHIC_SELECT_DIFFICULTY_EASY_PICTURE:	sAssetCategory+"\\"+sFileName = "Graphics\\select difficulty easy picture";	break;
		case GRAPHIC_SELECT_DIFFICULTY_EXPLANATION:		sAssetCategory+"\\"+sFileName = "Graphics\\select difficulty explanation";	break;
		case GRAPHIC_SELECT_DIFFICULTY_HARD_HEADER:		sAssetCategory+"\\"+sFileName = "Graphics\\select difficulty hard header";	break;
		case GRAPHIC_SELECT_DIFFICULTY_HARD_PICTURE:	sAssetCategory+"\\"+sFileName = "Graphics\\select difficulty hard picture";	break;
		case GRAPHIC_SELECT_DIFFICULTY_MEDIUM_HEADER:	sAssetCategory+"\\"+sFileName = "Graphics\\select difficulty medium header";	break;
		case GRAPHIC_SELECT_DIFFICULTY_MEDIUM_PICTURE:	sAssetCategory+"\\"+sFileName = "Graphics\\select difficulty medium picture";break;
		case GRAPHIC_SELECT_DIFFICULTY_ONI_HEADER:		sAssetCategory+"\\"+sFileName = "Graphics\\select difficulty oni header";	break;
		case GRAPHIC_SELECT_DIFFICULTY_ONI_PICTURE:		sAssetCategory+"\\"+sFileName = "Graphics\\select difficulty oni picture";	break;
		case GRAPHIC_SELECT_DIFFICULTY_ENDLESS_HEADER:	sAssetCategory+"\\"+sFileName = "Graphics\\select difficulty endless header";	break;
		case GRAPHIC_SELECT_DIFFICULTY_ENDLESS_PICTURE:	sAssetCategory+"\\"+sFileName = "Graphics\\select difficulty endless picture";	break;
		case GRAPHIC_SELECT_DIFFICULTY_OK:				sAssetCategory+"\\"+sFileName = "Graphics\\select difficulty ok";			break;
		case GRAPHIC_SELECT_DIFFICULTY_TOP_EDGE:		sAssetCategory+"\\"+sFileName = "Graphics\\select difficulty top edge";		break;
		case GRAPHIC_SELECT_GAME_BACKGROUND:			sAssetCategory+"\\"+sFileName = "Graphics\\select game background";			break;
		case GRAPHIC_SELECT_GAME_TOP_EDGE:				sAssetCategory+"\\"+sFileName = "Graphics\\select game top edge";			break;
		case GRAPHIC_SELECT_GROUP_BACKGROUND:			sAssetCategory+"\\"+sFileName = "Graphics\\select group background";			break;
		case GRAPHIC_SELECT_GROUP_BUTTON:				sAssetCategory+"\\"+sFileName = "Graphics\\select group button";				break;
		case GRAPHIC_SELECT_GROUP_CONTENTS_HEADER:		sAssetCategory+"\\"+sFileName = "Graphics\\select group contents header";	break;
		case GRAPHIC_SELECT_GROUP_EXPLANATION:			sAssetCategory+"\\"+sFileName = "Graphics\\select group explanation";		break;
		case GRAPHIC_SELECT_GROUP_INFO_FRAME:			sAssetCategory+"\\"+sFileName = "Graphics\\select group info frame";			break;
		case GRAPHIC_SELECT_GROUP_TOP_EDGE:				sAssetCategory+"\\"+sFileName = "Graphics\\select group top edge";			break;
		case GRAPHIC_SELECT_MUSIC_BACKGROUND:			sAssetCategory+"\\"+sFileName = "Graphics\\select music background";			break;
		case GRAPHIC_SELECT_MUSIC_DIFFICULTY_ICONS:		sAssetCategory+"\\"+sFileName = "Graphics\\select music difficulty icons";	break;
		case GRAPHIC_SELECT_MUSIC_DIFFICULTY_FRAME:		sAssetCategory+"\\"+sFileName = "Graphics\\select music difficulty frame";	break;
		case GRAPHIC_SELECT_MUSIC_INFO_FRAME:			sAssetCategory+"\\"+sFileName = "Graphics\\select music info frame";			break;
		case GRAPHIC_SELECT_MUSIC_METER_FRAME:			sAssetCategory+"\\"+sFileName = "Graphics\\select music meter frame";		break;
		case GRAPHIC_SELECT_MUSIC_RADAR_BASE:			sAssetCategory+"\\"+sFileName = "Graphics\\select music radar base";			break;
		case GRAPHIC_SELECT_MUSIC_RADAR_WORDS:			sAssetCategory+"\\"+sFileName = "Graphics\\select music radar words 1x5";	break;
		case GRAPHIC_SELECT_MUSIC_ROULETTE_BANNER:		sAssetCategory+"\\"+sFileName = "Graphics\\select music roulette banner";	break;
		case GRAPHIC_SELECT_MUSIC_SCROLLBAR:			sAssetCategory+"\\"+sFileName = "Graphics\\select music scrollbar";			break;
		case GRAPHIC_SELECT_MUSIC_SCORE_FRAME:			sAssetCategory+"\\"+sFileName = "Graphics\\select music score frame";		break;
		case GRAPHIC_SELECT_MUSIC_SECTION_BANNER:		sAssetCategory+"\\"+sFileName = "Graphics\\select music section banner";		break;
		case GRAPHIC_SELECT_MUSIC_SECTION_BAR:			sAssetCategory+"\\"+sFileName = "Graphics\\select music section bar";		break;
		case GRAPHIC_SELECT_MUSIC_SMALL_GRADES:			sAssetCategory+"\\"+sFileName = "Graphics\\select music small grades 1x7";	break;
		case GRAPHIC_SELECT_MUSIC_SONG_BAR:				sAssetCategory+"\\"+sFileName = "Graphics\\select music song bar";			break;
		case GRAPHIC_SELECT_MUSIC_SONG_HIGHLIGHT:		sAssetCategory+"\\"+sFileName = "Graphics\\select music song highlight";		break;
		case GRAPHIC_SELECT_MUSIC_TOP_EDGE:				sAssetCategory+"\\"+sFileName = "Graphics\\select music top edge";			break;
		case GRAPHIC_SELECT_MUSIC_LONG_BALLOON:			sAssetCategory+"\\"+sFileName = "Graphics\\select music long balloon";		break;
		case GRAPHIC_SELECT_MUSIC_MARATHON_BALLOON:		sAssetCategory+"\\"+sFileName = "Graphics\\select music marathon balloon";	break;
		case GRAPHIC_SELECT_STYLE_BACKGROUND:			sAssetCategory+"\\"+sFileName = "Graphics\\select Style background";			break;
		case GRAPHIC_SELECT_STYLE_EXPLANATION:			sAssetCategory+"\\"+sFileName = "Graphics\\select Style explanation";		break;
		case GRAPHIC_SELECT_STYLE_TOP_EDGE:				sAssetCategory+"\\"+sFileName = "Graphics\\select Style top edge";			break;
		case GRAPHIC_SELECT_STYLE_ICONS:				sAssetCategory+"\\"+sFileName = "Graphics\\select Style icons";				break;
		case GRAPHIC_SELECT_STYLE_INFO_GAME_0_STYLE_0:		sAssetCategory+"\\"+sFileName = "Graphics\\select style info game 0 style 0";	break;
		case GRAPHIC_SELECT_STYLE_INFO_GAME_0_STYLE_1:		sAssetCategory+"\\"+sFileName = "Graphics\\select style info game 0 style 1";	break;
		case GRAPHIC_SELECT_STYLE_INFO_GAME_0_STYLE_2:		sAssetCategory+"\\"+sFileName = "Graphics\\select style info game 0 style 2";	break;
		case GRAPHIC_SELECT_STYLE_INFO_GAME_0_STYLE_3:		sAssetCategory+"\\"+sFileName = "Graphics\\select style info game 0 style 3";	break;
		case GRAPHIC_SELECT_STYLE_INFO_GAME_0_STYLE_4:		sAssetCategory+"\\"+sFileName = "Graphics\\select style info game 0 style 4";	break;
		case GRAPHIC_SELECT_STYLE_INFO_GAME_1_STYLE_0:		sAssetCategory+"\\"+sFileName = "Graphics\\select style info game 1 style 0";	break;
		case GRAPHIC_SELECT_STYLE_INFO_GAME_1_STYLE_1:		sAssetCategory+"\\"+sFileName = "Graphics\\select style info game 1 style 1";	break;
		case GRAPHIC_SELECT_STYLE_INFO_GAME_1_STYLE_2:		sAssetCategory+"\\"+sFileName = "Graphics\\select style info game 1 style 2";	break;
		case GRAPHIC_SELECT_STYLE_INFO_GAME_2_STYLE_0:		sAssetCategory+"\\"+sFileName = "Graphics\\select style info game 2 style 0";	break;
		case GRAPHIC_SELECT_STYLE_INFO_GAME_2_STYLE_1:		sAssetCategory+"\\"+sFileName = "Graphics\\select style info game 2 style 1";	break;
		case GRAPHIC_SELECT_STYLE_INFO_GAME_2_STYLE_2:		sAssetCategory+"\\"+sFileName = "Graphics\\select style info game 2 style 2";	break;
		case GRAPHIC_SELECT_STYLE_INFO_GAME_2_STYLE_3:		sAssetCategory+"\\"+sFileName = "Graphics\\select style info game 2 style 3";	break;
		case GRAPHIC_SELECT_STYLE_PREVIEW_GAME_0_STYLE_0:	sAssetCategory+"\\"+sFileName = "Graphics\\select style preview game 0 style 0";	break;
		case GRAPHIC_SELECT_STYLE_PREVIEW_GAME_0_STYLE_1:	sAssetCategory+"\\"+sFileName = "Graphics\\select style preview game 0 style 1";	break;
		case GRAPHIC_SELECT_STYLE_PREVIEW_GAME_0_STYLE_2:	sAssetCategory+"\\"+sFileName = "Graphics\\select style preview game 0 style 2";	break;
		case GRAPHIC_SELECT_STYLE_PREVIEW_GAME_0_STYLE_3:	sAssetCategory+"\\"+sFileName = "Graphics\\select style preview game 0 style 3";	break;
		case GRAPHIC_SELECT_STYLE_PREVIEW_GAME_0_STYLE_4:	sAssetCategory+"\\"+sFileName = "Graphics\\select style preview game 0 style 4";	break;
		case GRAPHIC_SELECT_STYLE_PREVIEW_GAME_1_STYLE_0:	sAssetCategory+"\\"+sFileName = "Graphics\\select style preview game 1 style 0";	break;
		case GRAPHIC_SELECT_STYLE_PREVIEW_GAME_1_STYLE_1:	sAssetCategory+"\\"+sFileName = "Graphics\\select style preview game 1 style 1";	break;
		case GRAPHIC_SELECT_STYLE_PREVIEW_GAME_1_STYLE_2:	sAssetCategory+"\\"+sFileName = "Graphics\\select style preview game 1 style 2";	break;
		case GRAPHIC_SELECT_STYLE_PREVIEW_GAME_2_STYLE_0:	sAssetCategory+"\\"+sFileName = "Graphics\\select style preview game 2 style 0";	break;
		case GRAPHIC_SELECT_STYLE_PREVIEW_GAME_2_STYLE_1:	sAssetCategory+"\\"+sFileName = "Graphics\\select style preview game 2 style 1";	break;
		case GRAPHIC_SELECT_STYLE_PREVIEW_GAME_2_STYLE_2:	sAssetCategory+"\\"+sFileName = "Graphics\\select style preview game 2 style 2";	break;
		case GRAPHIC_SELECT_STYLE_PREVIEW_GAME_2_STYLE_3:	sAssetCategory+"\\"+sFileName = "Graphics\\select style preview game 2 style 3";	break;
		case GRAPHIC_SONG_OPTIONS_BACKGROUND:			sAssetCategory+"\\"+sFileName = "Graphics\\song options background";			break;	
		case GRAPHIC_SONG_OPTIONS_TOP_EDGE:				sAssetCategory+"\\"+sFileName = "Graphics\\song options top edge";			break;
		case GRAPHIC_STAGE_ENDLESS:						sAssetCategory+"\\"+sFileName = "Graphics\\stage endless";					break;
		case GRAPHIC_STAGE_EXTRA1:						sAssetCategory+"\\"+sFileName = "Graphics\\stage extra1";					break;
		case GRAPHIC_STAGE_EXTRA2:						sAssetCategory+"\\"+sFileName = "Graphics\\stage extra2";					break;
		case GRAPHIC_STAGE_FINAL:						sAssetCategory+"\\"+sFileName = "Graphics\\stage final";						break;
		case GRAPHIC_STAGE_NUMBERS:						sAssetCategory+"\\"+sFileName = "Graphics\\stage numbers";					break;
		case GRAPHIC_STAGE_ONI:							sAssetCategory+"\\"+sFileName = "Graphics\\stage oni";						break;
		case GRAPHIC_STAGE_STAGE:						sAssetCategory+"\\"+sFileName = "Graphics\\stage stage";						break;
		case GRAPHIC_TITLE_MENU_BACKGROUND:				sAssetCategory+"\\"+sFileName = "Graphics\\title menu background";			break;
		case GRAPHIC_TITLE_MENU_LOGO_GAME_0:			sAssetCategory+"\\"+sFileName = "Graphics\\title menu logo game 0";			break;
		case GRAPHIC_TITLE_MENU_LOGO_GAME_1:			sAssetCategory+"\\"+sFileName = "Graphics\\title menu logo game 1";			break;
		case GRAPHIC_TITLE_MENU_LOGO_GAME_2:			sAssetCategory+"\\"+sFileName = "Graphics\\title menu logo game 2";			break;

		case SOUND_ATTRACT_INSERT_COIN:			sAssetCategory+"\\"+sFileName = "Sounds\\attract insert coin";			break;
		case SOUND_EDIT_CHANGE_BPM:				sAssetCategory+"\\"+sFileName = "Sounds\\edit change bpm";				break;
		case SOUND_EDIT_CHANGE_FREEZE:			sAssetCategory+"\\"+sFileName = "Sounds\\edit change freeze";			break;
		case SOUND_EDIT_CHANGE_LINE:			sAssetCategory+"\\"+sFileName = "Sounds\\edit change line";				break;
		case SOUND_EDIT_CHANGE_SNAP:			sAssetCategory+"\\"+sFileName = "Sounds\\edit change snap";				break;
		case SOUND_EDIT_SAVE:					sAssetCategory+"\\"+sFileName = "Sounds\\edit save";						break;
		case SOUND_EVALUATION_EXTRA_STAGE:		sAssetCategory+"\\"+sFileName = "Sounds\\evaluation extra stage";		break;
		case SOUND_GAMEPLAY_ASSIST_TICK:		sAssetCategory+"\\"+sFileName = "Sounds\\gameplay assist tick";			break;
		case SOUND_GAMEPLAY_FAILED:				sAssetCategory+"\\"+sFileName = "Sounds\\gameplay failed";				break;
		case SOUND_GAMEPLAY_ONI_GAIN_LIFE:		sAssetCategory+"\\"+sFileName = "Sounds\\gameplay oni gain life";		break;
		case SOUND_GAMEPLAY_ONI_LOSE_LIFE:		sAssetCategory+"\\"+sFileName = "Sounds\\gameplay oni lose life";		break;
		case SOUND_GAMEPLAY_ONI_DIE:			sAssetCategory+"\\"+sFileName = "Sounds\\gameplay oni die";				break;
		case SOUND_INSERT_COIN:					sAssetCategory+"\\"+sFileName = "Sounds\\insert coin";					break;
		case "Sounds","menu back":					sAssetCategory+"\\"+sFileName = "Sounds\\menu back";						break;
		case SOUND_MENU_INVALID:				sAssetCategory+"\\"+sFileName = "Sounds\\menu invalid";					break;
		case SOUND_MENU_MUSIC:					sAssetCategory+"\\"+sFileName = "Sounds\\menu music";					break;
		case SOUND_MENU_PROMPT:					sAssetCategory+"\\"+sFileName = "Sounds\\menu prompt";					break;
		case "Sounds","menu start":					sAssetCategory+"\\"+sFileName = "Sounds\\menu start";					break;
		case SOUND_MENU_SWOOSH:					sAssetCategory+"\\"+sFileName = "Sounds\\menu swoosh";					break;
		case SOUND_MENU_TIMER:					sAssetCategory+"\\"+sFileName = "Sounds\\menu timer";					break;
		case SOUND_MUSIC_SCROLL_MUSIC:			sAssetCategory+"\\"+sFileName = "Sounds\\music scroll music";			break;
		case SOUND_OPTION_CHANGE_COL:			sAssetCategory+"\\"+sFileName = "Sounds\\option change col";				break;
		case SOUND_OPTION_CHANGE_ROW:			sAssetCategory+"\\"+sFileName = "Sounds\\option change row";				break;
		case SOUND_SELECT_DIFFICULTY_CHANGE:	sAssetCategory+"\\"+sFileName = "Sounds\\select difficulty change";		break;
		case SOUND_SELECT_GROUP_CHANGE:			sAssetCategory+"\\"+sFileName = "Sounds\\select group change";			break;
		case SOUND_SELECT_MUSIC_SECTION_EXPAND:	sAssetCategory+"\\"+sFileName = "Sounds\\select music section expand";	break;
		case SOUND_SELECT_MUSIC_CHANGE_MUSIC:	sAssetCategory+"\\"+sFileName = "Sounds\\select music change music";		break;
		case SOUND_SELECT_MUSIC_CHANGE_OPTIONS:	sAssetCategory+"\\"+sFileName = "Sounds\\select music change options";	break;
		case SOUND_SELECT_MUSIC_CHANGE_SORT:	sAssetCategory+"\\"+sFileName = "Sounds\\select music change sort";		break;
		case SOUND_SELECT_MUSIC_CHANGE_NOTES:	sAssetCategory+"\\"+sFileName = "Sounds\\select music change notes";		break;
		case SOUND_SELECT_MUSIC_WHEEL_LOCKED:	sAssetCategory+"\\"+sFileName = "Sounds\\select music wheel locked";		break;
		case SOUND_SELECT_STYLE_CHANGE:			sAssetCategory+"\\"+sFileName = "Sounds\\select style change";			break;
		case SOUND_TITLE_MENU_CHANGE:			sAssetCategory+"\\"+sFileName = "Sounds\\title menu change";				break;

		case "Fonts","header1":						sAssetCategory+"\\"+sFileName = "Fonts\\Header1";						break;
		case "Fonts","Header2":						sAssetCategory+"\\"+sFileName = "Fonts\\Header2";						break;
		case "Fonts","normal":						sAssetCategory+"\\"+sFileName = "Fonts\\Normal";							break;
		case FONT_ITALIC:						sAssetCategory+"\\"+sFileName = "Fonts\\Italic";							break;
		case FONT_COMBO_NUMBERS:				sAssetCategory+"\\"+sFileName = "Fonts\\Combo Numbers";					break;
		case "Fonts","meter":						sAssetCategory+"\\"+sFileName = "Fonts\\Meter";							break;
		case "Fonts","score numbers":				sAssetCategory+"\\"+sFileName = "Fonts\\Score Numbers";					break;
		case "Fonts","timer numbers":				sAssetCategory+"\\"+sFileName = "Fonts\\Timer Numbers";					break;
		case "Fonts","text banner":					sAssetCategory+"\\"+sFileName = "Fonts\\Text Banner";					break;
		case FONT_STAGE:						sAssetCategory+"\\"+sFileName = "Fonts\\stage";							break;

		default:	ASSERT(0);  // Unhandled theme element
	}
	CStringArray asPossibleElementFilePaths;
	CString sCurrentThemeDir = GetThemeDirFromName( sThemeName );
	CString sDefaultThemeDir = GetThemeDirFromName( BASE_THEME_NAME );

	///////////////////////////////////////
	// Search both the current theme and the default theme dirs for this element
	///////////////////////////////////////
	if( -1 != sAssetCategory+"\\"+sFileName.Find("Graphics\\") )
	{
		GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + "*.sprite", asPossibleElementFilePaths, false, true );
		GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + "*.png", asPossibleElementFilePaths, false, true );
		GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + "*.jpg", asPossibleElementFilePaths, false, true );
		GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + "*.bmp", asPossibleElementFilePaths, false, true );
		GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + "*.gif", asPossibleElementFilePaths, false, true );

		GetDirListing( sDefaultThemeDir + sAssetCategory+"\\"+sFileName + "*.sprite", asPossibleElementFilePaths, false, true );
		GetDirListing( sDefaultThemeDir + sAssetCategory+"\\"+sFileName + "*.png", asPossibleElementFilePaths, false, true );
		GetDirListing( sDefaultThemeDir + sAssetCategory+"\\"+sFileName + "*.jpg", asPossibleElementFilePaths, false, true );
		GetDirListing( sDefaultThemeDir + sAssetCategory+"\\"+sFileName + "*.bmp", asPossibleElementFilePaths, false, true );
		GetDirListing( sDefaultThemeDir + sAssetCategory+"\\"+sFileName + "*.gif", asPossibleElementFilePaths, false, true );
	}
	else if( -1 != sAssetCategory+"\\"+sFileName.Find("Sounds\\") )
	{
		GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + ".set", asPossibleElementFilePaths, false, true );
		GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + ".mp3", asPossibleElementFilePaths, false, true );
		GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + ".ogg", asPossibleElementFilePaths, false, true );
		GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + ".wav", asPossibleElementFilePaths, false, true );

		GetDirListing( sDefaultThemeDir + sAssetCategory+"\\"+sFileName + ".set", asPossibleElementFilePaths, false, true );
		GetDirListing( sDefaultThemeDir + sAssetCategory+"\\"+sFileName + ".mp3", asPossibleElementFilePaths, false, true );
		GetDirListing( sDefaultThemeDir + sAssetCategory+"\\"+sFileName + ".ogg", asPossibleElementFilePaths, false, true );
		GetDirListing( sDefaultThemeDir + sAssetCategory+"\\"+sFileName + ".wav", asPossibleElementFilePaths, false, true );
	}
	else if( -1 != sAssetCategory+"\\"+sFileName.Find("Fonts\\") )
	{
		GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + ".font", asPossibleElementFilePaths, false, true );

		GetDirListing( sDefaultThemeDir + sAssetCategory+"\\"+sFileName + ".font", asPossibleElementFilePaths, false, true );
	}
	else
	{
		ASSERT(0); // Unknown theme asset dir;
	}

	if( asPossibleElementFilePaths.GetSize() > 0 )
		return asPossibleElementFilePaths[0];
	else
	{
		ASSERT(0);
		throw RageException( "Theme element '%s' could not be found in '%s' or '%s'.", 
			sAssetCategory+"\\"+sFileName, 
			GetThemeDirFromName(m_sCurThemeName), 
			GetThemeDirFromName(BASE_THEME_NAME) );
	}

	return "";
}
*/

CString ThemeManager::GetPathTo( CString sAssetCategory, CString sFileName ) 
{
#ifdef _DEBUG
try_element_again:
#endif

	sAssetCategory.MakeLower();
	sFileName.MakeLower();

	const CString sCurrentThemeDir = GetThemeDirFromName( m_sCurThemeName );
	const CString sDefaultThemeDir = GetThemeDirFromName( BASE_THEME_NAME );	

	CStringArray asPossibleElementFilePaths;

	// look for a redirect
	GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + "*.redir", asPossibleElementFilePaths, false, true );
	if( asPossibleElementFilePaths.GetSize() > 0 )
	{
		CStdioFile file;
		file.Open( asPossibleElementFilePaths[0], CFile::modeRead );
		CString sLine;
		file.ReadString( sLine );
	}


	///////////////////////////////////////
	// Search both the current theme and the default theme dirs for this element
	///////////////////////////////////////
	if( sAssetCategory == "graphics" )
	{
		const char *masks[] = {
			"*.sprite", "*.png", "*.jpg", "*.bmp", "*.gif", "*.redir",
			"*.avi", "*.mpg", "*.mpeg", NULL
		};

		int i;
		for(i = 0; masks[i]; ++i)
			GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + masks[i],
				asPossibleElementFilePaths, false, true );

		for(i = 0; masks[i]; ++i)
			GetDirListing( sDefaultThemeDir + sAssetCategory+"\\"+sFileName + masks[i],
				asPossibleElementFilePaths, false, true );
	}
	else if( sAssetCategory == "sounds" )
	{
		GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + ".set", asPossibleElementFilePaths, false, true );
		GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + ".mp3", asPossibleElementFilePaths, false, true );
		GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + ".ogg", asPossibleElementFilePaths, false, true );
		GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + ".wav", asPossibleElementFilePaths, false, true );
		GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + ".redir", asPossibleElementFilePaths, false, true );

		GetDirListing( sDefaultThemeDir + sAssetCategory+"\\"+sFileName + ".set", asPossibleElementFilePaths, false, true );
		GetDirListing( sDefaultThemeDir + sAssetCategory+"\\"+sFileName + ".mp3", asPossibleElementFilePaths, false, true );
		GetDirListing( sDefaultThemeDir + sAssetCategory+"\\"+sFileName + ".ogg", asPossibleElementFilePaths, false, true );
		GetDirListing( sDefaultThemeDir + sAssetCategory+"\\"+sFileName + ".wav", asPossibleElementFilePaths, false, true );
		GetDirListing( sDefaultThemeDir + sAssetCategory+"\\"+sFileName + ".redir", asPossibleElementFilePaths, false, true );
	}
	else if( sAssetCategory == "fonts" )
	{
		GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + ".font", asPossibleElementFilePaths, false, true );
		GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + ".redir", asPossibleElementFilePaths, false, true );

		GetDirListing( sDefaultThemeDir + sAssetCategory+"\\"+sFileName + ".font", asPossibleElementFilePaths, false, true );
		GetDirListing( sDefaultThemeDir + sAssetCategory+"\\"+sFileName + ".redir", asPossibleElementFilePaths, false, true );
	}
	else
	{
		ASSERT(0); // Unknown theme asset category;
	}

	
	if( asPossibleElementFilePaths.GetSize() == 0 )
	{
#ifdef _DEBUG
		if( IDRETRY == AfxMessageBox( ssprintf("The theme element %s/%s is missing.  Correct this and click Retry, or Cancel to break.",sAssetCategory,sFileName), MB_RETRYCANCEL ) )
			goto try_element_again;
#endif
		throw RageException( "Theme element '%s/%s' could not be found in '%s' or '%s'.", 
			sAssetCategory,
			sFileName, 
			GetThemeDirFromName(m_sCurThemeName), 
			GetThemeDirFromName(BASE_THEME_NAME) );
	}
	else
	{
		asPossibleElementFilePaths[0].MakeLower();
		if( asPossibleElementFilePaths[0].GetLength() > 5  &&  asPossibleElementFilePaths[0].Right(5) == "redir" )	// this is a redirect file
		{
			CString sRedirFilePath = asPossibleElementFilePaths[0];
			
			CString sDir, sFName, sExt;
			splitrelpath( sRedirFilePath, sDir, sFName, sExt );

			CStdioFile file;
			file.Open( sRedirFilePath, CFile::modeRead );
			CString sNewFileName;
			file.ReadString( sNewFileName );
			CString sNewFilePath = sDir+"\\"+sNewFileName;
			if( sNewFileName == ""  ||  !DoesFileExist(sNewFilePath) )
			{
				throw RageException( "The redirect '%s' points to the file '%s', which does not exist.  Verify that this redirect is correct.", sRedirFilePath, sNewFilePath ); 
			}
			else
				return sNewFilePath;
		}
		else
		{
			return asPossibleElementFilePaths[0];
		}
	}

	return "";
}


CString ThemeManager::GetMetricsPathFromName( CString sThemeName )
{
	return GetThemeDirFromName( sThemeName ) + "metrics.ini";
}

CString ThemeManager::GetMetric( CString sScreenName, CString sValueName )
{
#ifdef _DEBUG
try_metric_again:
#endif
	CString sCurMetricPath = GetMetricsPathFromName(m_sCurThemeName);
	CString sDefaultMetricPath = GetMetricsPathFromName(BASE_THEME_NAME);

	// is our metric cache out of date?
	if( m_iHashForCurThemeMetrics != GetHashForFile(sCurMetricPath)  ||
		m_iHashForBaseThemeMetrics != GetHashForFile(sDefaultMetricPath) )
	{
		SwitchTheme(m_sCurThemeName);	// force a reload of the metrics cache
	}

	CString sValue;
	if( m_pIniMetrics->GetValue(sScreenName,sValueName,sValue) )
		return sValue;

#ifdef _DEBUG
	if( IDRETRY == AfxMessageBox( ssprintf("The theme metric %s-%s is missing.  Correct this and click Retry, or Cancel to break.",sScreenName,sValueName), MB_RETRYCANCEL ) )
		goto try_metric_again;
#endif

	throw RageException( "Theme metric '%s : %s' could not be found in '%s' or '%s'.", 
		sScreenName,
		sValueName,
		sCurMetricPath, 
		sDefaultMetricPath
		);
}

int ThemeManager::GetMetricI( CString sScreenName, CString sValueName )
{
	return atoi( GetMetric(sScreenName,sValueName) );
}

float ThemeManager::GetMetricF( CString sScreenName, CString sValueName )
{
	return (float)atof( GetMetric(sScreenName,sValueName) );
}

bool ThemeManager::GetMetricB( CString sScreenName, CString sValueName )
{
	return atoi( GetMetric(sScreenName,sValueName) ) != 0;
}

D3DXCOLOR ThemeManager::GetMetricC( CString sScreenName, CString sValueName )
{
	float r=1,b=1,g=1,a=1;	// initialize in case sscanf fails
	CString sValue = GetMetric(sScreenName,sValueName);
	char szValue[40];
	strcpy( szValue, sValue );
	int result = sscanf( szValue, "%f,%f,%f,%f", &r, &g, &b, &a );
	if( result != 4 )
	{
		LOG->Warn( "The color value '%s' for theme metric '%s : %s' is invalid.", szValue, sScreenName, sValueName );
		ASSERT(0);
	}

	return D3DXCOLOR(r,g,b,a);
}
