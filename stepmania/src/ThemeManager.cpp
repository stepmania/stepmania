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
#include "ErrorCatcher/ErrorCatcher.h"


ThemeManager*	THEME = NULL;	// global object accessable from anywhere in the program


const CString DEFAULT_THEME_NAME = "default";
const CString THEME_BASE_DIR  = "Themes\\";

ThemeManager::ThemeManager()
{
	CStringArray arrayThemeNames;
	GetThemeNames( arrayThemeNames );
	for( int i=0; i<arrayThemeNames.GetSize(); i++ )
		AssertThemeIsComplete( arrayThemeNames[i] );

	SwitchTheme( DEFAULT_THEME_NAME );
}

void ThemeManager::GetThemeNames( CStringArray& AddTo )
{
	GetDirListing( DEFAULT_THEME_NAME+"\\*", AddTo, true );
	
	// strip out the folder called "CVS"
	for( int i=AddTo.GetSize()-1; i>=0; i-- )
	{
		if( 0 == stricmp( AddTo[i], "cvs" ) )
			AddTo.RemoveAt(i);
	}
}

void ThemeManager::SwitchTheme( CString sThemeName )
{
	m_sCurThemeName = sThemeName;
	CString sThemeDir = GetThemeDirFromName(m_sCurThemeName);
	if( !DoesFileExist( sThemeDir ) )
		FatalError( "Error loading the theme in diretory '%s'.", sThemeDir );
}


void ThemeManager::AssertThemeIsComplete( CString sAnnouncerName )
{
	CString sOriginalThemeName = m_sCurThemeName;

	CStringArray asThemeNames;
	GetThemeNames( asThemeNames );
	for( int t=0; t<asThemeNames.GetSize(); t++ )
	{
		SwitchTheme( asThemeNames[t] );
		for( int e=0; e<NUM_THEME_ELEMENTS; e++ )
		{
			// GetPathTo will assert if it can't find the file in the current theme dir
			// or in the default dir.
			GetPathTo( (ThemeElement)e );
		}
	}
}

CString ThemeManager::GetThemeDirFromName( CString sThemeName )
{
	return THEME_BASE_DIR + sThemeName + "\\";
}

CString ThemeManager::GetPathTo( ThemeElement te ) 
{
	CString sAssetPrefix;		// fill this in below

	switch( te )
	{

		case GRAPHIC_ALL_MUSIC_BANNER:					sAssetPrefix = "Graphics\\all music banner";				break;
		case GRAPHIC_ARROWS_LEFT:						sAssetPrefix = "Graphics\\arrows left 1x4";					break;
		case GRAPHIC_ARROWS_RIGHT:						sAssetPrefix = "Graphics\\arrows right 1x4";				break;
		case GRAPHIC_CAUTION:							sAssetPrefix = "Graphics\\Caution";							break;
		case GRAPHIC_DANCER_P1:							sAssetPrefix = "Graphics\\dancer p1";						break;
		case GRAPHIC_DANCER_P2:							sAssetPrefix = "Graphics\\dancer p2";						break;
		case GRAPHIC_DIFFICULTY_ICONS:					sAssetPrefix = "Graphics\\select music difficulty icons";	break;
		case GRAPHIC_EDIT_BACKGROUND:					sAssetPrefix = "Graphics\\edit background";					break;
		case GRAPHIC_EDIT_SNAP_INDICATOR:				sAssetPrefix = "Graphics\\edit snap indicator";				break;
		case GRAPHIC_FALLBACK_BACKGROUND:				sAssetPrefix = "Graphics\\Fallback Background";				break;
		case GRAPHIC_FALLBACK_BANNER:					sAssetPrefix = "Graphics\\Fallback Banner";					break;
		case GRAPHIC_FALLBACK_CD_TITLE:					sAssetPrefix = "Graphics\\Fallback CD Title";				break;
		case GRAPHIC_GAME_OPTIONS_BACKGROUND:			sAssetPrefix = "Graphics\\game options background";			break;
		case GRAPHIC_GAME_OPTIONS_TOP_EDGE:				sAssetPrefix = "Graphics\\game options top edge";			break;
		case GRAPHIC_GAME_OVER:							sAssetPrefix = "Graphics\\game over";						break;
		case GRAPHIC_GAMEPLAY_BOTTOM_FRAME:				sAssetPrefix = "Graphics\\gameplay bottom frame";			break;
		case GRAPHIC_GAMEPLAY_CLEARED:					sAssetPrefix = "Graphics\\gameplay cleared";				break;
		case GRAPHIC_GAMEPLAY_CLOSING_STAR:				sAssetPrefix = "Graphics\\gameplay closing star";			break;
		case GRAPHIC_GAMEPLAY_COMBO:					sAssetPrefix = "Graphics\\gameplay combo";					break;
		case GRAPHIC_GAMEPLAY_DANGER_BACKGROUND:		sAssetPrefix = "Graphics\\gameplay danger background";		break;
		case GRAPHIC_GAMEPLAY_DANGER_TEXT:				sAssetPrefix = "Graphics\\gameplay danger text";			break;
		case GRAPHIC_GAMEPLAY_DIFFICULTY_FRAME:			sAssetPrefix = "Graphics\\gameplay difficulty frame";		break;
		case GRAPHIC_GAMEPLAY_FAILED:					sAssetPrefix = "Graphics\\gameplay failed";					break;
		case GRAPHIC_GAMEPLAY_HERE_WE_GO:				sAssetPrefix = "Graphics\\gameplay here we go";				break;
		case GRAPHIC_GAMEPLAY_JUDGEMENT:				sAssetPrefix = "Graphics\\gameplay judgement 1x9";			break;
		case GRAPHIC_GAMEPLAY_OPENING_STAR:				sAssetPrefix = "Graphics\\gameplay opening star";			break;
		case GRAPHIC_GAMEPLAY_READY:					sAssetPrefix = "Graphics\\gameplay Ready";					break;
		case GRAPHIC_GAMEPLAY_TOP_FRAME:				sAssetPrefix = "Graphics\\gameplay top frame";				break;
		case GRAPHIC_GRAPHIC_OPTIONS_BACKGROUND:		sAssetPrefix = "Graphics\\graphic options background";		break;
		case GRAPHIC_GRAPHIC_OPTIONS_TOP_EDGE:			sAssetPrefix = "Graphics\\graphic options top edge";		break;
		case GRAPHIC_KEEP_ALIVE:						sAssetPrefix = "Graphics\\keep alive";						break;
		case GRAPHIC_LIFEMETER_FRAME:					sAssetPrefix = "Graphics\\Life Meter Frame";				break;
		case GRAPHIC_LIFEMETER_PILLS:					sAssetPrefix = "Graphics\\life meter pills 17x1";			break;
		case GRAPHIC_MENU_BOTTOM_EDGE:					sAssetPrefix = "Graphics\\menu bottom edge";				break;
		case GRAPHIC_MENU_STYLE_ICONS:					sAssetPrefix = "Graphics\\menu style icons";				break;
		case GRAPHIC_MUSIC_SCROLL_BACKGROUND:			sAssetPrefix = "Graphics\\music scroll background";			break;
		case GRAPHIC_MUSIC_SORT_ICONS:					sAssetPrefix = "Graphics\\music sort icons 1x4";			break;
		case GRAPHIC_MUSIC_STATUS_ICONS:				sAssetPrefix = "Graphics\\music status icons 1x4";			break;
		case GRAPHIC_PAD_DOUBLE:						sAssetPrefix = "Graphics\\Pad double";						break;
		case GRAPHIC_PAD_SINGLE:						sAssetPrefix = "Graphics\\Pad single";						break;
		case GRAPHIC_PAD_SOLO:							sAssetPrefix = "Graphics\\Pad solo";						break;
		case GRAPHIC_PLAYER_OPTIONS_BACKGROUND:			sAssetPrefix = "Graphics\\player options background";		break;
		case GRAPHIC_PLAYER_OPTIONS_TOP_EDGE:			sAssetPrefix = "Graphics\\player options top edge";			break;
		case GRAPHIC_RESULTS_BACKGROUND:				sAssetPrefix = "Graphics\\results background";				break;
		case GRAPHIC_RESULTS_BANNER_FRAME:				sAssetPrefix = "Graphics\\results banner frame";			break;
		case GRAPHIC_RESULTS_BONUS_FRAME_P1:			sAssetPrefix = "Graphics\\results bonus frame p1";			break;
		case GRAPHIC_RESULTS_BONUS_FRAME_P2:			sAssetPrefix = "Graphics\\results bonus frame p2";			break;
		case GRAPHIC_RESULTS_GRADE_FRAME:				sAssetPrefix = "Graphics\\results grade frame 1x2";			break;
		case GRAPHIC_RESULTS_GRADES:					sAssetPrefix = "Graphics\\results grades 1x7";				break;
		case GRAPHIC_RESULTS_JUDGE_LABELS:				sAssetPrefix = "Graphics\\results judge labels 1x6";		break;
		case GRAPHIC_RESULTS_SCORE_LABELS:				sAssetPrefix = "Graphics\\results score labels 1x2";		break;
		case GRAPHIC_RESULTS_TOP_EDGE:					sAssetPrefix = "Graphics\\results top edge";				break;
		case GRAPHIC_SELECT_DIFFICULTY_ARROW_P1:		sAssetPrefix = "Graphics\\select difficulty arrow p1";		break;
		case GRAPHIC_SELECT_DIFFICULTY_ARROW_P2:		sAssetPrefix = "Graphics\\select difficulty arrow p2";		break;
		case GRAPHIC_SELECT_DIFFICULTY_BACKGROUND:		sAssetPrefix = "Graphics\\select difficulty background";	break;
		case GRAPHIC_SELECT_DIFFICULTY_EASY_HEADER:		sAssetPrefix = "Graphics\\select difficulty easy header";	break;
		case GRAPHIC_SELECT_DIFFICULTY_EASY_PICTURE:	sAssetPrefix = "Graphics\\select difficulty easy picture";	break;
		case GRAPHIC_SELECT_DIFFICULTY_EXPLANATION:		sAssetPrefix = "Graphics\\select difficulty explanation";	break;
		case GRAPHIC_SELECT_DIFFICULTY_HARD_HEADER:		sAssetPrefix = "Graphics\\select difficulty hard header";	break;
		case GRAPHIC_SELECT_DIFFICULTY_HARD_PICTURE:	sAssetPrefix = "Graphics\\select difficulty hard picture";	break;
		case GRAPHIC_SELECT_DIFFICULTY_MEDIUM_HEADER:	sAssetPrefix = "Graphics\\select difficulty medium header";	break;
		case GRAPHIC_SELECT_DIFFICULTY_MEDIUM_PICTURE:	sAssetPrefix = "Graphics\\select difficulty medium picture";break;
		case GRAPHIC_SELECT_DIFFICULTY_OK:				sAssetPrefix = "Graphics\\select difficulty ok";			break;
		case GRAPHIC_SELECT_DIFFICULTY_TOP_EDGE:		sAssetPrefix = "Graphics\\select difficulty top edge";		break;
		case GRAPHIC_SELECT_GAME_BACKGROUND:			sAssetPrefix = "Graphics\\select game background";			break;
		case GRAPHIC_SELECT_GAME_TOP_EDGE:				sAssetPrefix = "Graphics\\select game top edge";			break;
		case GRAPHIC_SELECT_GROUP_BACKGROUND:			sAssetPrefix = "Graphics\\select group background";			break;
		case GRAPHIC_SELECT_GROUP_BUTTON:				sAssetPrefix = "Graphics\\select group button";				break;
		case GRAPHIC_SELECT_GROUP_CONTENTS_HEADER:		sAssetPrefix = "Graphics\\select group contents header";	break;
		case GRAPHIC_SELECT_GROUP_EXPLANATION:			sAssetPrefix = "Graphics\\select group explanation";		break;
		case GRAPHIC_SELECT_GROUP_INFO_FRAME:			sAssetPrefix = "Graphics\\select group info frame";			break;
		case GRAPHIC_SELECT_GROUP_TOP_EDGE:				sAssetPrefix = "Graphics\\select group top edge";			break;
		case GRAPHIC_SELECT_MUSIC_BACKGROUND:			sAssetPrefix = "Graphics\\select music background";			break;
		case GRAPHIC_SELECT_MUSIC_DIFFICULTY_FRAME:		sAssetPrefix = "Graphics\\select music difficulty frame";	break;
		case GRAPHIC_SELECT_MUSIC_INFO_FRAME:			sAssetPrefix = "Graphics\\select music info frame";			break;
		case GRAPHIC_SELECT_MUSIC_METER_FRAME:			sAssetPrefix = "Graphics\\select music meter frame";		break;
		case GRAPHIC_SELECT_MUSIC_OPTION_ICONS:			sAssetPrefix = "Graphics\\select music option icons";		break;
		case GRAPHIC_SELECT_MUSIC_RADAR_BASE:			sAssetPrefix = "Graphics\\select music radar base";			break;
		case GRAPHIC_SELECT_MUSIC_RADAR_WORDS:			sAssetPrefix = "Graphics\\select music radar words 1x5";	break;
		case GRAPHIC_SELECT_MUSIC_ROULETTE_BANNER:		sAssetPrefix = "Graphics\\select music roulette banner";	break;
		case GRAPHIC_SELECT_MUSIC_SCROLLBAR:			sAssetPrefix = "Graphics\\select music scrollbar";			break;
		case GRAPHIC_SELECT_MUSIC_SCORE_FRAME:			sAssetPrefix = "Graphics\\select music score frame";		break;
		case GRAPHIC_SELECT_MUSIC_SECTION_BANNER:		sAssetPrefix = "Graphics\\select music section banner";		break;
		case GRAPHIC_SELECT_MUSIC_SECTION_BAR:			sAssetPrefix = "Graphics\\select music section bar";		break;
		case GRAPHIC_SELECT_MUSIC_SMALL_GRADES:			sAssetPrefix = "Graphics\\select music small grades 1x7";	break;
		case GRAPHIC_SELECT_MUSIC_SONG_BAR:				sAssetPrefix = "Graphics\\select music song bar";			break;
		case GRAPHIC_SELECT_MUSIC_SONG_HIGHLIGHT:		sAssetPrefix = "Graphics\\select music song highlight";		break;
		case GRAPHIC_SELECT_MUSIC_TOP_EDGE:				sAssetPrefix = "Graphics\\select music top edge";			break;
		case GRAPHIC_SELECT_STYLE_BACKGROUND:			sAssetPrefix = "Graphics\\select Style background";			break;
		case GRAPHIC_SELECT_STYLE_EXPLANATION:			sAssetPrefix = "Graphics\\select Style explanation";		break;
		case GRAPHIC_SELECT_STYLE_TOP_EDGE:				sAssetPrefix = "Graphics\\select Style top edge";			break;
		case GRAPHIC_SELECT_STYLE_ICONS:				sAssetPrefix = "Graphics\\select Style icons";				break;
		case GRAPHIC_SELECT_STYLE_INFO_0:				sAssetPrefix = "Graphics\\select style info 0";				break;
		case GRAPHIC_SELECT_STYLE_INFO_1:				sAssetPrefix = "Graphics\\select style info 1";				break;
		case GRAPHIC_SELECT_STYLE_INFO_2:				sAssetPrefix = "Graphics\\select style info 2";				break;
		case GRAPHIC_SELECT_STYLE_INFO_3:				sAssetPrefix = "Graphics\\select style info 3";				break;
		case GRAPHIC_SELECT_STYLE_INFO_4:				sAssetPrefix = "Graphics\\select style info 4";				break;
		case GRAPHIC_SELECT_STYLE_INFO_5:				sAssetPrefix = "Graphics\\select style info 5";				break;
		case GRAPHIC_SELECT_STYLE_INFO_6:				sAssetPrefix = "Graphics\\select style info 6";				break;
		case GRAPHIC_SELECT_STYLE_INFO_7:				sAssetPrefix = "Graphics\\select style info 7";				break;
		case GRAPHIC_SELECT_STYLE_PREVIEW_0:			sAssetPrefix = "Graphics\\select style preview 0";			break;
		case GRAPHIC_SELECT_STYLE_PREVIEW_1:			sAssetPrefix = "Graphics\\select style preview 1";			break;
		case GRAPHIC_SELECT_STYLE_PREVIEW_2:			sAssetPrefix = "Graphics\\select style preview 2";			break;
		case GRAPHIC_SELECT_STYLE_PREVIEW_3:			sAssetPrefix = "Graphics\\select style preview 3";			break;
		case GRAPHIC_SELECT_STYLE_PREVIEW_4:			sAssetPrefix = "Graphics\\select style preview 4";			break;
		case GRAPHIC_SELECT_STYLE_PREVIEW_5:			sAssetPrefix = "Graphics\\select style preview 5";			break;
		case GRAPHIC_SELECT_STYLE_PREVIEW_6:			sAssetPrefix = "Graphics\\select style preview 6";			break;
		case GRAPHIC_SELECT_STYLE_PREVIEW_7:			sAssetPrefix = "Graphics\\select style preview 7";			break;
		case GRAPHIC_SONG_OPTIONS_BACKGROUND:			sAssetPrefix = "Graphics\\song options background";			break;	
		case GRAPHIC_SONG_OPTIONS_TOP_EDGE:				sAssetPrefix = "Graphics\\song options top edge";			break;
		case GRAPHIC_STAGE_UNDERSCORE:					sAssetPrefix = "Graphics\\stage underscore";				break;
		case GRAPHIC_STEPS_DESCRIPTION:					sAssetPrefix = "Graphics\\Steps description 1x8";			break;
		case GRAPHIC_SYNCHRONIZE_BACKGROUND:			sAssetPrefix = "Graphics\\synchronize background";			break;
		case GRAPHIC_SYNCHRONIZE_TOP_EDGE:				sAssetPrefix = "Graphics\\synchronize top edge";			break;
		case GRAPHIC_TITLE_MENU_BACKGROUND:				sAssetPrefix = "Graphics\\title menu background";			break;
		case GRAPHIC_TITLE_MENU_LOGO:					sAssetPrefix = "Graphics\\title menu logo";					break;

		case SOUND_EDIT_CHANGE_LINE:			sAssetPrefix = "Sounds\\edit change line";				break;
		case SOUND_EDIT_CHANGE_SNAP:			sAssetPrefix = "Sounds\\edit change snap";				break;
		case SOUND_GAMEPLAY_ASSIST_TICK:		sAssetPrefix = "Sounds\\gameplay assist tick";			break;
		case SOUND_GAMEPLAY_FAILED:				sAssetPrefix = "Sounds\\gameplay failed";				break;
		case SOUND_INTRO_BREAK:					sAssetPrefix = "Sounds\\intro break";					break;
		case SOUND_MENU_BACK:					sAssetPrefix = "Sounds\\menu back";						break;
		case SOUND_MENU_INVALID:				sAssetPrefix = "Sounds\\menu invalid";					break;
		case SOUND_MENU_MUSIC:					sAssetPrefix = "Sounds\\menu music";					break;
		case SOUND_MENU_PROMPT:					sAssetPrefix = "Sounds\\menu prompt";					break;
		case SOUND_MENU_START:					sAssetPrefix = "Sounds\\menu start";					break;
		case SOUND_MENU_SWOOSH:					sAssetPrefix = "Sounds\\menu swoosh";					break;
		case SOUND_MUSIC_SCROLL_MUSIC:			sAssetPrefix = "Sounds\\music scroll music";			break;
		case SOUND_OPTION_CHANGE_COL:			sAssetPrefix = "Sounds\\option change col";				break;
		case SOUND_OPTION_CHANGE_ROW:			sAssetPrefix = "Sounds\\option change row";				break;
		case SOUND_RESULTS_FLY_OFF:				sAssetPrefix = "Sounds\\results fly off";				break;
		case SOUND_RESULTS_SCORE_TICK:			sAssetPrefix = "Sounds\\results score tick";			break;
		case SOUND_SELECT_DIFFICULTY_CHANGE:	sAssetPrefix = "Sounds\\select difficulty change";		break;
		case SOUND_SELECT_GROUP_CHANGE:			sAssetPrefix = "Sounds\\select group change";			break;
		case SOUND_SELECT_MUSIC_SECTION_EXPAND:	sAssetPrefix = "Sounds\\select music section expand";	break;
		case SOUND_SELECT_MUSIC_CHANGE_MUSIC:	sAssetPrefix = "Sounds\\select music change music";		break;
		case SOUND_SELECT_MUSIC_CHANGE_SORT:	sAssetPrefix = "Sounds\\select music change sort";		break;
		case SOUND_SELECT_MUSIC_CHANGE_NOTES:	sAssetPrefix = "Sounds\\select music change notes";		break;
		case SOUND_SELECT_STYLE_CHANGE:			sAssetPrefix = "Sounds\\select style change";			break;
		case SOUND_TITLE_MENU_CHANGE:			sAssetPrefix = "Sounds\\title menu change";				break;

		case FONT_HEADER1:						sAssetPrefix = "Fonts\\Header1";						break;
		case FONT_HEADER2:						sAssetPrefix = "Fonts\\Header2";						break;
		case FONT_NORMAL:						sAssetPrefix = "Fonts\\Normal";							break;
		case FONT_COMBO_NUMBERS:				sAssetPrefix = "Fonts\\Combo Numbers";					break;
		case FONT_METER:						sAssetPrefix = "Fonts\\Meter";							break;
		case FONT_SCORE_NUMBERS:				sAssetPrefix = "Fonts\\Score Numbers";					break;
		case FONT_TIMER_NUMBERS:				sAssetPrefix = "Fonts\\Timer Numbers";					break;
		case FONT_TEXT_BANNER:					sAssetPrefix = "Fonts\\Text Banner";					break;
		case FONT_STAGE:						sAssetPrefix = "Fonts\\stage";							break;

		default:	ASSERT(0);  // Unhandled theme element
	}
	

	CStringArray asPossibleElementFilePaths;
	CString sCurrentThemeDir = GetThemeDirFromName( m_sCurThemeName );
	CString sDefaultThemeDir = GetThemeDirFromName( DEFAULT_THEME_NAME );

	///////////////////////////////////////
	// Search both the current theme and the default theme dirs for this element
	///////////////////////////////////////
	if( -1 != sAssetPrefix.Find("Graphics\\") )
	{
		GetDirListing( sCurrentThemeDir + sAssetPrefix + "*.sprite", asPossibleElementFilePaths, false, true );
		GetDirListing( sCurrentThemeDir + sAssetPrefix + "*.png", asPossibleElementFilePaths, false, true );
		GetDirListing( sCurrentThemeDir + sAssetPrefix + "*.jpg", asPossibleElementFilePaths, false, true );
		GetDirListing( sCurrentThemeDir + sAssetPrefix + "*.bmp", asPossibleElementFilePaths, false, true );

		GetDirListing( sDefaultThemeDir + sAssetPrefix + "*.sprite", asPossibleElementFilePaths, false, true );
		GetDirListing( sDefaultThemeDir + sAssetPrefix + "*.png", asPossibleElementFilePaths, false, true );
		GetDirListing( sDefaultThemeDir + sAssetPrefix + "*.jpg", asPossibleElementFilePaths, false, true );
		GetDirListing( sDefaultThemeDir + sAssetPrefix + "*.bmp", asPossibleElementFilePaths, false, true );
	}
	else if( -1 != sAssetPrefix.Find("Sounds\\") )
	{
		GetDirListing( sCurrentThemeDir + sAssetPrefix + ".set", asPossibleElementFilePaths, false, true );
		GetDirListing( sCurrentThemeDir + sAssetPrefix + ".mp3", asPossibleElementFilePaths, false, true );
		GetDirListing( sCurrentThemeDir + sAssetPrefix + ".ogg", asPossibleElementFilePaths, false, true );
		GetDirListing( sCurrentThemeDir + sAssetPrefix + ".wav", asPossibleElementFilePaths, false, true );

		GetDirListing( sDefaultThemeDir + sAssetPrefix + ".set", asPossibleElementFilePaths, false, true );
		GetDirListing( sDefaultThemeDir + sAssetPrefix + ".mp3", asPossibleElementFilePaths, false, true );
		GetDirListing( sDefaultThemeDir + sAssetPrefix + ".ogg", asPossibleElementFilePaths, false, true );
		GetDirListing( sDefaultThemeDir + sAssetPrefix + ".wav", asPossibleElementFilePaths, false, true );
	}
	else if( -1 != sAssetPrefix.Find("Fonts\\") )
	{
		GetDirListing( sCurrentThemeDir + sAssetPrefix + ".font", asPossibleElementFilePaths, false, true );

		GetDirListing( sDefaultThemeDir + sAssetPrefix + ".font", asPossibleElementFilePaths, false, true );
	}
	else
	{
		ASSERT(0); // Unknown theme asset dir;
	}

	if( asPossibleElementFilePaths.GetSize() > 0 )
		return asPossibleElementFilePaths[0];
	else
		FatalError( "Theme element '%s' could not be found in '%s' or '%s'.", 
			sDefaultThemeDir + sAssetPrefix, 
			GetThemeDirFromName(m_sCurThemeName), 
			GetThemeDirFromName(DEFAULT_THEME_NAME) );

	return "";
}

