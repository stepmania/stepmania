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
const CString THEME_BASE_DIR  = "Themes\\default\\";

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
	CString sThemeDir = GetThemeDirFromName( m_sCurThemeName );

	if( !DoesFileExist( sThemeDir ) )
		FatalError( "Error loading the announcer in diretory '%s'.", m_sCurThemeName );
}


void ThemeManager::AssertThemeIsComplete( CString sAnnouncerName )
{
	for( int i=0; i<NUM_THEME_ELEMENTS; i++ )
	{
		CString sPath = GetPathTo( (ThemeElement)i, sAnnouncerName );
		if( !DoesFileExist(sPath) )
			FatalError( "The theme element '%s' is missing.", sPath );
	}
}

CString ThemeManager::GetPathTo( ThemeElement te )
{
	return GetPathTo( te, m_sCurThemeName );
}


CString ThemeManager::GetThemeDirFromName( CString sThemeName )
{
	return ssprintf( "%s\\%s\\", THEME_BASE_DIR, sThemeName );
}


CString ThemeManager::GetPathTo( ThemeElement te, CString sThemeName ) 
{
	CString sAssetDir;		// fill this in below

	switch( te )
	{
		case GRAPHIC_TITLE_MENU_BACKGROUND:			sAssetDir = "Graphics\\title menu background";		break;
		case GRAPHIC_SELECT_STYLE_BACKGROUND:		sAssetDir = "Graphics\\select Style background";	break;
		case GRAPHIC_SELECT_MUSIC_BACKGROUND:		sAssetDir = "Graphics\\select music background";	break;
		case GRAPHIC_SELECT_STYLE_TOP_EDGE:			sAssetDir = "Graphics\\select Style top edge";		break;
		case GRAPHIC_SELECT_MUSIC_TOP_EDGE:			sAssetDir = "Graphics\\select music top edge";		break;
		case GRAPHIC_GAME_OPTIONS_TOP_EDGE:			sAssetDir = "Graphics\\game options top edge";		break;
		case GRAPHIC_GRAPHIC_OPTIONS_TOP_EDGE:		sAssetDir = "Graphics\\graphic options top edge";	break;
		case GRAPHIC_PLAYER_OPTIONS_TOP_EDGE:		sAssetDir = "Graphics\\player options top edge";	break;
		case GRAPHIC_SONG_OPTIONS_TOP_EDGE:			sAssetDir = "Graphics\\song options top edge";		break;
		case GRAPHIC_RESULTS_TOP_EDGE:				sAssetDir = "Graphics\\results top edge";			break;
		case GRAPHIC_RESULTS_BACKGROUND:			sAssetDir = "Graphics\\results background";		break;
		case GRAPHIC_RESULTS_BANNER_FRAME:			sAssetDir = "Graphics\\results banner frame";		break;
		case GRAPHIC_RESULTS_BONUS_FRAME_P1:		sAssetDir = "Graphics\\results bonus frame p1";	break;
		case GRAPHIC_RESULTS_BONUS_FRAME_P2:		sAssetDir = "Graphics\\results bonus frame p2";	break;
		case GRAPHIC_RESULTS_GRADE_FRAME:			sAssetDir = "Graphics\\results grade frame 1x2";	break;
		case GRAPHIC_RESULTS_JUDGE_LABELS:			sAssetDir = "Graphics\\results judge labels 1x6";	break;
		case GRAPHIC_RESULTS_SCORE_LABELS:			sAssetDir = "Graphics\\results score labels 1x2";	break;
		case GRAPHIC_RESULTS_GRADES:				sAssetDir = "Graphics\\results grades 1x7";		break;
		case GRAPHIC_FALLBACK_BANNER:				sAssetDir = "Graphics\\Fallback Banner";			break;
		case GRAPHIC_FALLBACK_BACKGROUND:			sAssetDir = "Graphics\\Fallback Background";		break;
		case GRAPHIC_FALLBACK_CD_TITLE:				sAssetDir = "Graphics\\Fallback CD Title";			break;
		case GRAPHIC_GAMEPLAY_JUDGEMENT:			sAssetDir = "Graphics\\gameplay judgement 1x9";	break;
		case GRAPHIC_MENU_BOTTOM_EDGE:				sAssetDir = "Graphics\\menu bottom edge";			break;
		case GRAPHIC_LIFEMETER_FRAME:				sAssetDir = "Graphics\\Life Meter Frame";			break;
		case GRAPHIC_LIFEMETER_PILLS:				sAssetDir = "Graphics\\life meter pills 17x1";		break;
		case GRAPHIC_GAMEPLAY_COMBO:				sAssetDir = "Graphics\\gameplay combo";			break;
		case GRAPHIC_SCROLLBAR_PARTS:				sAssetDir = "Graphics\\scrollbar parts";			break;
		case GRAPHIC_GAMEPLAY_CLOSING_STAR:			sAssetDir = "Graphics\\gameplay closing star";		break;
		case GRAPHIC_GAMEPLAY_OPENING_STAR:			sAssetDir = "Graphics\\gameplay opening star";		break;
		case GRAPHIC_CAUTION:						sAssetDir = "Graphics\\Caution";					break;
		case GRAPHIC_GAMEPLAY_READY:				sAssetDir = "Graphics\\gameplay Ready";			break;
		case GRAPHIC_GAMEPLAY_HERE_WE_GO:			sAssetDir = "Graphics\\gameplay here we go";		break;
		case GRAPHIC_GAMEPLAY_CLEARED:				sAssetDir = "Graphics\\gameplay cleared";			break;
		case GRAPHIC_GAMEPLAY_FAILED:				sAssetDir = "Graphics\\gameplay failed";			break;
		case GRAPHIC_KEEP_ALIVE:					sAssetDir = "Graphics\\keep alive";				break;
		case GRAPHIC_DANCER_P1:						sAssetDir = "Graphics\\dancer p1";					break;
		case GRAPHIC_DANCER_P2:						sAssetDir = "Graphics\\dancer p2";					break;
		case GRAPHIC_PAD_SINGLE:					sAssetDir = "Graphics\\Pad single";				break;
		case GRAPHIC_PAD_DOUBLE:					sAssetDir = "Graphics\\Pad double";				break;
		case GRAPHIC_PAD_SOLO:						sAssetDir = "Graphics\\Pad solo";					break;
		case GRAPHIC_STYLE_ICONS:					sAssetDir = "Graphics\\Style icons 1x5";			break;
		case GRAPHIC_SELECT_MUSIC_SONG_BAR:			sAssetDir = "Graphics\\select music song bar";			break;
		case GRAPHIC_SELECT_MUSIC_SONG_HIGHLIGHT:	sAssetDir = "Graphics\\select music song highlight";	break;
		case GRAPHIC_STEPS_DESCRIPTION:					sAssetDir = "Graphics\\Steps description 1x8";				break;
		case GRAPHIC_SELECT_MUSIC_SECTION_BAR:			sAssetDir = "Graphics\\select music section bar";			break;
		case GRAPHIC_MUSIC_SORT_ICONS:					sAssetDir = "Graphics\\music sort icons 1x4";				break;
		case GRAPHIC_MUSIC_STATUS_ICONS:				sAssetDir = "Graphics\\music status icons 1x4";			break;
		case GRAPHIC_GAMEPLAY_DANGER_TEXT:				sAssetDir = "Graphics\\gameplay danger text";				break;
		case GRAPHIC_GAMEPLAY_DANGER_BACKGROUND:		sAssetDir = "Graphics\\gameplay danger background";		break;
		case GRAPHIC_ARROWS_LEFT:						sAssetDir = "Graphics\\arrows left 1x4";					break;
		case GRAPHIC_ARROWS_RIGHT:						sAssetDir = "Graphics\\arrows right 1x4";					break;
		case GRAPHIC_EDIT_BACKGROUND:					sAssetDir = "Graphics\\edit background";					break;
		case GRAPHIC_EDIT_SNAP_INDICATOR:				sAssetDir = "Graphics\\edit snap indicator";				break;
		case GRAPHIC_GAME_OPTIONS_BACKGROUND:			sAssetDir = "Graphics\\game options background";			break;
		case GRAPHIC_GRAPHIC_OPTIONS_BACKGROUND:		sAssetDir = "Graphics\\graphic options background";		break;
		case GRAPHIC_PLAYER_OPTIONS_BACKGROUND:			sAssetDir = "Graphics\\player options background";			break;
		case GRAPHIC_SONG_OPTIONS_BACKGROUND:			sAssetDir = "Graphics\\song options background";			break;	
		case GRAPHIC_SYNCHRONIZE_BACKGROUND:			sAssetDir = "Graphics\\synchronize background";			break;
		case GRAPHIC_SYNCHRONIZE_TOP_EDGE:				sAssetDir = "Graphics\\synchronize top edge";				break;
		case GRAPHIC_SELECT_GAME_BACKGROUND:			sAssetDir = "Graphics\\select game background";			break;
		case GRAPHIC_SELECT_GAME_TOP_EDGE:				sAssetDir = "Graphics\\select game top edge";				break;
		case GRAPHIC_TITLE_MENU_LOGO:					sAssetDir = "Graphics\\title menu logo";					break;
		case GRAPHIC_SELECT_DIFFICULTY_BACKGROUND:		sAssetDir = "Graphics\\select difficulty background";		break;
		case GRAPHIC_SELECT_DIFFICULTY_TOP_EDGE:		sAssetDir = "Graphics\\select difficulty top edge";		break;
		case GRAPHIC_SELECT_DIFFICULTY_EXPLANATION:		sAssetDir = "Graphics\\select difficulty explanation";		break;
		case GRAPHIC_SELECT_DIFFICULTY_EASY_HEADER:		sAssetDir = "Graphics\\select difficulty easy header";		break;
		case GRAPHIC_SELECT_DIFFICULTY_MEDIUM_HEADER:	sAssetDir = "Graphics\\select difficulty medium header";	break;
		case GRAPHIC_SELECT_DIFFICULTY_HARD_HEADER:		sAssetDir = "Graphics\\select difficulty hard header";		break;
		case GRAPHIC_SELECT_DIFFICULTY_EASY_PICTURE:	sAssetDir = "Graphics\\select difficulty easy picture";	break;
		case GRAPHIC_SELECT_DIFFICULTY_MEDIUM_PICTURE:	sAssetDir = "Graphics\\select difficulty medium picture";	break;
		case GRAPHIC_SELECT_DIFFICULTY_HARD_PICTURE:	sAssetDir = "Graphics\\select difficulty hard picture";	break;
		case GRAPHIC_SELECT_DIFFICULTY_ARROW_P1:		sAssetDir = "Graphics\\select difficulty arrow p1";		break;
		case GRAPHIC_SELECT_DIFFICULTY_ARROW_P2:		sAssetDir = "Graphics\\select difficulty arrow p2";		break;
		case GRAPHIC_SELECT_DIFFICULTY_OK:				sAssetDir = "Graphics\\select difficulty ok";				break;
		case GRAPHIC_SELECT_MODE_BACKGROUND:			sAssetDir = "Graphics\\select mode background";			break;
		case GRAPHIC_SELECT_MODE_TOP_EDGE:				sAssetDir = "Graphics\\select mode top edge";				break;
		case GRAPHIC_SELECT_MODE_EXPLANATION:			sAssetDir = "Graphics\\select mode explanation";			break;
		case GRAPHIC_SELECT_MODE_ARROW:					sAssetDir = "Graphics\\select mode arrow";					break;
		case GRAPHIC_SELECT_MODE_OK:					sAssetDir = "Graphics\\select mode ok";					break;
		case GRAPHIC_SELECT_MODE_ARCADE_HEADER:			sAssetDir = "Graphics\\select mode arcade header";			break;
		case GRAPHIC_SELECT_MODE_ARCADE_PICTURE:		sAssetDir = "Graphics\\select mode arcade picture";		break;
		case GRAPHIC_SELECT_MODE_FREE_PLAY_HEADER:		sAssetDir = "Graphics\\select mode free play header";		break;
		case GRAPHIC_SELECT_MODE_FREE_PLAY_PICTURE:		sAssetDir = "Graphics\\select mode free play picture";		break;
		case GRAPHIC_SELECT_MODE_NONSTOP_HEADER:		sAssetDir = "Graphics\\select mode nonstop header";		break;
		case GRAPHIC_SELECT_MODE_NONSTOP_PICTURE:		sAssetDir = "Graphics\\select mode nonstop picture";		break;
		case GRAPHIC_SELECT_MUSIC_INFO_FRAME:			sAssetDir = "Graphics\\select music info frame";			break;
		case GRAPHIC_SELECT_MUSIC_RADAR_BASE:			sAssetDir = "Graphics\\select music radar base";			break;
		case GRAPHIC_SELECT_MUSIC_RADAR_WORDS:			sAssetDir = "Graphics\\select music radar words 1x5";		break;
		case GRAPHIC_SELECT_MUSIC_SCORE_FRAME:			sAssetDir = "Graphics\\select music score frame";			break;
		case GRAPHIC_SELECT_MUSIC_SMALL_GRADES:			sAssetDir = "Graphics\\select music small grades 1x7";		break;
		case GRAPHIC_SELECT_MUSIC_ROULETTE_BANNER:		sAssetDir = "Graphics\\select music roulette banner";		break;
		case GRAPHIC_SELECT_GROUP_EXPLANATION:			sAssetDir = "Graphics\\select group explanation";			break;
		case GRAPHIC_SELECT_GROUP_INFO_FRAME:			sAssetDir = "Graphics\\select group info frame";			break;
		case GRAPHIC_SELECT_GROUP_BUTTON:				sAssetDir = "Graphics\\select group button";				break;
		case GRAPHIC_SELECT_GROUP_CONTENTS_HEADER:		sAssetDir = "Graphics\\select group contents header";		break;
		case GRAPHIC_SELECT_GROUP_TOP_EDGE:				sAssetDir = "Graphics\\select group top edge";				break;
		case GRAPHIC_SELECT_GROUP_BACKGROUND:			sAssetDir = "Graphics\\select group background";			break;
		case GRAPHIC_MUSIC_SCROLL_BACKGROUND:			sAssetDir = "Graphics\\music scroll background";			break;
		case GRAPHIC_DIFFICULTY_ICONS:					sAssetDir = "Graphics\\difficulty icons";					break;
		case GRAPHIC_GAMEPLAY_DIFFICULTY_FRAME:			sAssetDir = "Graphics\\gameplay difficulty frame";			break;
		case GRAPHIC_GAMEPLAY_TOP_FRAME:				sAssetDir = "Graphics\\gameplay top frame";				break;
		case GRAPHIC_GAMEPLAY_BOTTOM_FRAME:				sAssetDir = "Graphics\\gameplay bottom frame";				break;
		case GRAPHIC_SELECT_MUSIC_OPTION_ICONS:			sAssetDir = "Graphics\\select music option icons";			break;
		case GRAPHIC_SELECT_MUSIC_DIFFICULTY_FRAME:		sAssetDir = "Graphics\\select music difficulty frame";		break;
		case GRAPHIC_SELECT_MUSIC_METER_FRAME:			sAssetDir = "Graphics\\select music meter frame";			break;
		case GRAPHIC_STAGE_UNDERSCORE:					sAssetDir = "Graphics\\stage underscore";					break;
		case GRAPHIC_ALL_MUSIC_BANNER:					sAssetDir = "Graphics\\all music banner";					break;

		case SOUND_FAILED:						sAssetDir = "Sounds\\failed";						break;
		case SOUND_ASSIST:						sAssetDir = "Sounds\\Assist";						break;
		case SOUND_SELECT:						sAssetDir = "Sounds\\select";						break;
		case SOUND_SWITCH_STYLE:				sAssetDir = "Sounds\\switch Style";				break;
		case SOUND_SWITCH_MUSIC:				sAssetDir = "Sounds\\switch music";				break;
		case SOUND_SWITCH_SORT:					sAssetDir = "Sounds\\switch sort";					break;
		case SOUND_EXPAND:						sAssetDir = "Sounds\\expand";						break;
		case SOUND_SWITCH_STEPS:				sAssetDir = "Sounds\\switch Steps";				break;
		case SOUND_TITLE_CHANGE:				sAssetDir = "Sounds\\title change";				break;
		case SOUND_MENU_SWOOSH:					sAssetDir = "Sounds\\menu swoosh";					break;
		case SOUND_MENU_BACK:					sAssetDir = "Sounds\\menu back";					break;
		case SOUND_INVALID:						sAssetDir = "Sounds\\invalid";						break;
		case SOUND_EDIT_CHANGE_LINE:			sAssetDir = "Sounds\\edit change line";			break;
		case SOUND_EDIT_CHANGE_SNAP:			sAssetDir = "Sounds\\edit change snap";			break;
		case SOUND_SELECT_DIFFICULTY_CHANGE:	sAssetDir = "Sounds\\select difficulty change";	break;
		case SOUND_SELECT_MODE_CHANGE:			sAssetDir = "Sounds\\select mode change";			break;
		case SOUND_MENU_MUSIC:					sAssetDir = "Sounds\\menu music";					break;
		case SOUND_ENDING_MUSIC:				sAssetDir = "Sounds\\ending music";				break;

		case FONT_HEADER1:						sAssetDir = "Fonts\\Header1";						break;
		case FONT_HEADER2:						sAssetDir = "Fonts\\Header2";						break;
		case FONT_NORMAL:						sAssetDir = "Fonts\\Normal";						break;
		case FONT_COMBO_NUMBERS:				sAssetDir = "Fonts\\Combo Numbers";				break;
		case FONT_METER:						sAssetDir = "Fonts\\Meter";						break;
		case FONT_SCORE_NUMBERS:				sAssetDir = "Fonts\\Score Numbers";				break;
		case FONT_TEXT_BANNER:					sAssetDir = "Fonts\\Text Banner";					break;
		case FONT_STAGE:						sAssetDir = "Fonts\\stage";						break;

		default:	ASSERT(0);  // Unhandled theme element
	}
	
	return GetThemeDirFromName( sThemeName ) + sAssetDir;
}

