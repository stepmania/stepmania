#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: FontManager

 Desc: See header.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ThemeManager.h"
#include "RageLog.h"
#include "ErrorCatcher/ErrorCatcher.h"


ThemeManager*	THEME = NULL;	// global object accessable from anywhere in the program


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
	GetDirListing( "Themes\\*", AddTo, true );
	
	// strip out the folder called "CVS"
	for( int i=0; i<AddTo.GetSize(); i++ )
	{
		if( 0 == stricmp( AddTo[i], "cvs" ) )
		{
			AddTo.RemoveAt(i);
			i--;
		}
	}
}

bool ThemeManager::SwitchTheme( CString sThemeName )		// return false if theme doesn't exist
{
	sThemeName.MakeLower();
	m_sCurThemeName = sThemeName;
	CString sThemeDir = ThemeNameToThemeDir( m_sCurThemeName );

	if( !DoesFileExist( sThemeDir ) )
	{
		FatalError( ssprintf( "The theme in diretory '%' could not be loaded.", sThemeDir ) );
		return false;
	}

	return true;
}

void ThemeManager::AssertThemeIsComplete( CString sThemeName )		// return false if theme doesn't exist
{
	for( int i=0; i<NUM_THEME_ELEMENTS; i++ )
	{
		if( GetPathTo( (ThemeElement)i, sThemeName ) == "" )
			FatalError( ssprintf( "The theme element for theme '%s' called '%s' could not be found.", sThemeName, ElementToAssetPath((ThemeElement)i) ) );
	}
}


CString ThemeManager::GetPathTo( ThemeElement te )
{
	return GetPathTo( te, m_sCurThemeName );
}


CString ThemeManager::ThemeNameToThemeDir( CString sThemeName )
{
	return ssprintf( "Themes\\%s\\", sThemeName );
}


CString ThemeManager::ElementToAssetPath( ThemeElement te ) 
{
	CString sAssetPath;		// fill this in below

	switch( te )
	{
		case GRAPHIC_TITLE_MENU_BACKGROUND:			sAssetPath = "Graphics\\title menu background";		break;
		case GRAPHIC_SELECT_STYLE_BACKGROUND:		sAssetPath = "Graphics\\select Style background";	break;
		case GRAPHIC_SELECT_MUSIC_BACKGROUND:		sAssetPath = "Graphics\\select music background";	break;
		case GRAPHIC_SELECT_STYLE_TOP_EDGE:			sAssetPath = "Graphics\\select Style top edge";		break;
		case GRAPHIC_SELECT_MUSIC_TOP_EDGE:			sAssetPath = "Graphics\\select music top edge";		break;
		case GRAPHIC_GAME_OPTIONS_TOP_EDGE:			sAssetPath = "Graphics\\game options top edge";		break;
		case GRAPHIC_GRAPHIC_OPTIONS_TOP_EDGE:		sAssetPath = "Graphics\\graphic options top edge";	break;
		case GRAPHIC_PLAYER_OPTIONS_TOP_EDGE:		sAssetPath = "Graphics\\player options top edge";	break;
		case GRAPHIC_SONG_OPTIONS_TOP_EDGE:			sAssetPath = "Graphics\\song options top edge";		break;
		case GRAPHIC_RESULTS_TOP_EDGE:				sAssetPath = "Graphics\\results top edge";			break;
		case GRAPHIC_RESULTS_BACKGROUND:			sAssetPath = "Graphics\\results background";		break;
		case GRAPHIC_RESULTS_BANNER_FRAME:			sAssetPath = "Graphics\\results banner frame";		break;
		case GRAPHIC_RESULTS_BONUS_FRAME_P1:		sAssetPath = "Graphics\\results bonus frame p1";	break;
		case GRAPHIC_RESULTS_BONUS_FRAME_P2:		sAssetPath = "Graphics\\results bonus frame p2";	break;
		case GRAPHIC_RESULTS_GRADE_FRAME:			sAssetPath = "Graphics\\results grade frame 1x2";	break;
		case GRAPHIC_RESULTS_JUDGE_LABELS:			sAssetPath = "Graphics\\results judge labels 1x6";	break;
		case GRAPHIC_RESULTS_SCORE_LABELS:			sAssetPath = "Graphics\\results score labels 1x2";	break;
		case GRAPHIC_RESULTS_GRADES:				sAssetPath = "Graphics\\results grades 1x7";		break;
		case GRAPHIC_FALLBACK_BANNER:				sAssetPath = "Graphics\\Fallback Banner";			break;
		case GRAPHIC_FALLBACK_BACKGROUND:			sAssetPath = "Graphics\\Fallback Background";		break;
		case GRAPHIC_FALLBACK_CD_TITLE:				sAssetPath = "Graphics\\Fallback CD Title";			break;
		case GRAPHIC_JUDGEMENT:						sAssetPath = "Graphics\\judgement 1x9";				break;
		case GRAPHIC_MENU_BOTTOM_EDGE:				sAssetPath = "Graphics\\menu bottom edge";			break;
		case GRAPHIC_SCORE_FRAME:					sAssetPath = "Graphics\\score frame";				break;
		case GRAPHIC_LIFEMETER_FRAME:				sAssetPath = "Graphics\\Life Meter Frame";			break;
		case GRAPHIC_LIFEMETER_PILLS:				sAssetPath = "Graphics\\life meter pills 17x1";		break;
		case GRAPHIC_COMBO:							sAssetPath = "Graphics\\combo";						break;
		case GRAPHIC_SCROLLBAR_PARTS:				sAssetPath = "Graphics\\scrollbar parts";			break;
		case GRAPHIC_CLOSING_STAR:					sAssetPath = "Graphics\\closing star";				break;
		case GRAPHIC_OPENING_STAR:					sAssetPath = "Graphics\\opening star";				break;
		case GRAPHIC_CAUTION:						sAssetPath = "Graphics\\Caution";					break;
		case GRAPHIC_READY:							sAssetPath = "Graphics\\Ready";						break;
		case GRAPHIC_HERE_WE_GO:					sAssetPath = "Graphics\\here we go";				break;
		case GRAPHIC_CLEARED:						sAssetPath = "Graphics\\cleared";					break;
		case GRAPHIC_FAILED:						sAssetPath = "Graphics\\failed";					break;
		case GRAPHIC_KEEP_ALIVE:					sAssetPath = "Graphics\\keep alive";				break;
		case GRAPHIC_DANCER_P1:						sAssetPath = "Graphics\\dancer p1";					break;
		case GRAPHIC_DANCER_P2:						sAssetPath = "Graphics\\dancer p2";					break;
		case GRAPHIC_PAD_SINGLE:					sAssetPath = "Graphics\\Pad single";				break;
		case GRAPHIC_PAD_DOUBLE:					sAssetPath = "Graphics\\Pad double";				break;
		case GRAPHIC_PAD_SOLO:						sAssetPath = "Graphics\\Pad solo";					break;
		case GRAPHIC_STYLE_ICONS:					sAssetPath = "Graphics\\Style icons 1x5";			break;
		case GRAPHIC_SELECT_MUSIC_SONG_BAR:			sAssetPath = "Graphics\\select music song bar";			break;
		case GRAPHIC_SELECT_MUSIC_SONG_HIGHLIGHT:	sAssetPath = "Graphics\\select music song highlight";	break;
		case GRAPHIC_STEPS_DESCRIPTION:					sAssetPath = "Graphics\\Steps description 1x8";				break;
		case GRAPHIC_SELECT_MUSIC_SECTION_BAR:			sAssetPath = "Graphics\\select music section bar";			break;
		case GRAPHIC_MUSIC_SORT_ICONS:					sAssetPath = "Graphics\\music sort icons 1x4";				break;
		case GRAPHIC_MUSIC_STATUS_ICONS:				sAssetPath = "Graphics\\music status icons 1x4";			break;
		case GRAPHIC_DANGER_TEXT:						sAssetPath = "Graphics\\danger text";						break;
		case GRAPHIC_DANGER_BACKGROUND:					sAssetPath = "Graphics\\danger background";					break;
		case GRAPHIC_ARROWS_LEFT:						sAssetPath = "Graphics\\arrows left 1x4";					break;
		case GRAPHIC_ARROWS_RIGHT:						sAssetPath = "Graphics\\arrows right 1x4";					break;
		case GRAPHIC_EDIT_BACKGROUND:					sAssetPath = "Graphics\\edit background";					break;
		case GRAPHIC_EDIT_SNAP_INDICATOR:				sAssetPath = "Graphics\\edit snap indicator";				break;
		case GRAPHIC_GAME_OPTIONS_BACKGROUND:			sAssetPath = "Graphics\\game options background";			break;
		case GRAPHIC_GRAPHIC_OPTIONS_BACKGROUND:		sAssetPath = "Graphics\\graphic options background";		break;
		case GRAPHIC_PLAYER_OPTIONS_BACKGROUND:			sAssetPath = "Graphics\\player options background";			break;
		case GRAPHIC_SONG_OPTIONS_BACKGROUND:			sAssetPath = "Graphics\\song options background";			break;	
		case GRAPHIC_SYNCHRONIZE_BACKGROUND:			sAssetPath = "Graphics\\synchronize background";			break;
		case GRAPHIC_SYNCHRONIZE_TOP_EDGE:				sAssetPath = "Graphics\\synchronize top edge";				break;
		case GRAPHIC_SELECT_GAME_BACKGROUND:			sAssetPath = "Graphics\\select game background";			break;
		case GRAPHIC_SELECT_GAME_TOP_EDGE:				sAssetPath = "Graphics\\select game top edge";				break;
		case GRAPHIC_TITLE_MENU_LOGO:					sAssetPath = "Graphics\\title menu logo";					break;
		case GRAPHIC_SELECT_DIFFICULTY_BACKGROUND:		sAssetPath = "Graphics\\select difficulty background";		break;
		case GRAPHIC_SELECT_DIFFICULTY_TOP_EDGE:		sAssetPath = "Graphics\\select difficulty top edge";		break;
		case GRAPHIC_SELECT_DIFFICULTY_EXPLANATION:		sAssetPath = "Graphics\\select difficulty explanation";		break;
		case GRAPHIC_SELECT_DIFFICULTY_EASY_HEADER:		sAssetPath = "Graphics\\select difficulty easy header";		break;
		case GRAPHIC_SELECT_DIFFICULTY_MEDIUM_HEADER:	sAssetPath = "Graphics\\select difficulty medium header";	break;
		case GRAPHIC_SELECT_DIFFICULTY_HARD_HEADER:		sAssetPath = "Graphics\\select difficulty hard header";		break;
		case GRAPHIC_SELECT_DIFFICULTY_EASY_PICTURE:	sAssetPath = "Graphics\\select difficulty easy picture";	break;
		case GRAPHIC_SELECT_DIFFICULTY_MEDIUM_PICTURE:	sAssetPath = "Graphics\\select difficulty medium picture";	break;
		case GRAPHIC_SELECT_DIFFICULTY_HARD_PICTURE:	sAssetPath = "Graphics\\select difficulty hard picture";	break;
		case GRAPHIC_SELECT_DIFFICULTY_ARROW_P1:		sAssetPath = "Graphics\\select difficulty arrow p1";		break;
		case GRAPHIC_SELECT_DIFFICULTY_ARROW_P2:		sAssetPath = "Graphics\\select difficulty arrow p2";		break;
		case GRAPHIC_SELECT_DIFFICULTY_OK:				sAssetPath = "Graphics\\select difficulty ok";				break;
		case GRAPHIC_SELECT_MODE_BACKGROUND:			sAssetPath = "Graphics\\select mode background";			break;
		case GRAPHIC_SELECT_MODE_TOP_EDGE:				sAssetPath = "Graphics\\select mode top edge";				break;
		case GRAPHIC_SELECT_MODE_EXPLANATION:			sAssetPath = "Graphics\\select mode explanation";			break;
		case GRAPHIC_SELECT_MODE_ARROW:					sAssetPath = "Graphics\\select mode arrow";					break;
		case GRAPHIC_SELECT_MODE_OK:					sAssetPath = "Graphics\\select mode ok";					break;
		case GRAPHIC_SELECT_MODE_ARCADE_HEADER:			sAssetPath = "Graphics\\select mode arcade header";			break;
		case GRAPHIC_SELECT_MODE_ARCADE_PICTURE:		sAssetPath = "Graphics\\select mode arcade picture";		break;
		case GRAPHIC_SELECT_MODE_FREE_PLAY_HEADER:		sAssetPath = "Graphics\\select mode free play header";		break;
		case GRAPHIC_SELECT_MODE_FREE_PLAY_PICTURE:		sAssetPath = "Graphics\\select mode free play picture";		break;
		case GRAPHIC_SELECT_MODE_NONSTOP_HEADER:		sAssetPath = "Graphics\\select mode nonstop header";		break;
		case GRAPHIC_SELECT_MODE_NONSTOP_PICTURE:		sAssetPath = "Graphics\\select mode nonstop picture";		break;
		case GRAPHIC_SELECT_MUSIC_INFO_FRAME:			sAssetPath = "Graphics\\select music info frame";			break;
		case GRAPHIC_SELECT_MUSIC_RADAR_BASE:			sAssetPath = "Graphics\\select music radar base";			break;
		case GRAPHIC_SELECT_MUSIC_RADAR_WORDS:			sAssetPath = "Graphics\\select music radar words 1x5";		break;
		case GRAPHIC_SELECT_MUSIC_SCORE_FRAME:			sAssetPath = "Graphics\\select music score frame";			break;
		case GRAPHIC_SELECT_MUSIC_SMALL_GRADES:			sAssetPath = "Graphics\\select music small grades 1x7";		break;
		case GRAPHIC_SELECT_GROUP_EXPLANATION:			sAssetPath = "Graphics\\select group explanation";			break;
		case GRAPHIC_SELECT_GROUP_INFO_FRAME:			sAssetPath = "Graphics\\select group info frame";			break;
		case GRAPHIC_SELECT_GROUP_BUTTON:				sAssetPath = "Graphics\\select group button";				break;
		case GRAPHIC_SELECT_GROUP_CONTENTS_HEADER:		sAssetPath = "Graphics\\select group contents header";		break;
		case GRAPHIC_ENDING_BACKGROUND:					sAssetPath = "Graphics\\ending background";					break;
		case GRAPHIC_DIFFICULTY_ICONS:					sAssetPath = "Graphics\\difficulty icons";					break;
		case GRAPHIC_DANCING_DIFFICULTY_FRAME:			sAssetPath = "Graphics\\dancing difficulty frame";			break;
		case GRAPHIC_DANCING_TOP_FRAME:					sAssetPath = "Graphics\\dancing top frame";					break;
		case GRAPHIC_DANCING_BOTTOM_FRAME:				sAssetPath = "Graphics\\dancing bottom frame";				break;
		case GRAPHIC_SELECT_MUSIC_OPTION_ICONS:			sAssetPath = "Graphics\\select music option icons";			break;
		case GRAPHIC_SELECT_MUSIC_DIFFICULTY_FRAME:		sAssetPath = "Graphics\\select music difficulty frame";		break;
		case GRAPHIC_SELECT_MUSIC_METER_FRAME:			sAssetPath = "Graphics\\select music meter frame";			break;
		case GRAPHIC_ALL_MUSIC_BANNER:					sAssetPath = "Graphics\\all music banner";					break;

		case SOUND_FAILED:						sAssetPath = "Sounds\\failed";						break;
		case SOUND_ASSIST:						sAssetPath = "Sounds\\Assist";						break;
		case SOUND_SELECT:						sAssetPath = "Sounds\\select";						break;
		case SOUND_SWITCH_STYLE:				sAssetPath = "Sounds\\switch Style";				break;
		case SOUND_SWITCH_MUSIC:				sAssetPath = "Sounds\\switch music";				break;
		case SOUND_SWITCH_SORT:					sAssetPath = "Sounds\\switch sort";					break;
		case SOUND_EXPAND:						sAssetPath = "Sounds\\expand";						break;
		case SOUND_SWITCH_STEPS:				sAssetPath = "Sounds\\switch Steps";				break;
		case SOUND_TITLE_CHANGE:				sAssetPath = "Sounds\\title change";				break;
		case SOUND_MENU_SWOOSH:					sAssetPath = "Sounds\\menu swoosh";					break;
		case SOUND_MENU_BACK:					sAssetPath = "Sounds\\menu back";					break;
		case SOUND_INVALID:						sAssetPath = "Sounds\\invalid";						break;
		case SOUND_EDIT_CHANGE_LINE:			sAssetPath = "Sounds\\edit change line";			break;
		case SOUND_EDIT_CHANGE_SNAP:			sAssetPath = "Sounds\\edit change snap";			break;
		case SOUND_SELECT_DIFFICULTY_CHANGE:	sAssetPath = "Sounds\\select difficulty change";	break;
		case SOUND_SELECT_MODE_CHANGE:			sAssetPath = "Sounds\\select mode change";			break;
		case SOUND_MENU_MUSIC:					sAssetPath = "Sounds\\menu music";					break;
		case SOUND_ENDING_MUSIC:				sAssetPath = "Sounds\\ending music";				break;

		case FONT_BOLD:							sAssetPath = "Fonts\\Bold";							break;
		case FONT_COMBO_NUMBERS:				sAssetPath = "Fonts\\Combo Numbers";				break;
		case FONT_FEET:							sAssetPath = "Fonts\\Feet";							break;
		case FONT_NORMAL:						sAssetPath = "Fonts\\Normal";						break;
		case FONT_SCORE_NUMBERS:				sAssetPath = "Fonts\\Score Numbers";				break;
		case FONT_TEXT_BANNER:					sAssetPath = "Fonts\\Text Banner";					break;
		case FONT_BIG_MESSAGE:					sAssetPath = "Fonts\\big message";					break;

		case ANNOUNCER_ATTRACT:					sAssetPath = "Announcer\\attract";					break;
		case ANNOUNCER_BAD_COMMENT:				sAssetPath = "Announcer\\bad comment";				break;
		case ANNOUNCER_CAUTION:					sAssetPath = "Announcer\\caution";					break;
		case ANNOUNCER_CLEARED:					sAssetPath = "Announcer\\cleared";					break;
		case ANNOUNCER_FAIL_COMMENT:			sAssetPath = "Announcer\\fail comment";				break;
		case ANNOUNCER_GAME_OVER:				sAssetPath = "Announcer\\game over";				break;
		case ANNOUNCER_GOOD_COMMENT:			sAssetPath = "Announcer\\good comment";				break;
		case ANNOUNCER_HERE_WE_GO:				sAssetPath = "Announcer\\here we go";				break;
		case ANNOUNCER_MUSIC_COMMENT:			sAssetPath = "Announcer\\music comment";			break;
		case ANNOUNCER_READY:					sAssetPath = "Announcer\\ready";					break;
		case ANNOUNCER_READY_LAST:				sAssetPath = "Announcer\\ready last";				break;
		case ANNOUNCER_RESULT_AAA:				sAssetPath = "Announcer\\result aaa";				break;
		case ANNOUNCER_RESULT_AA:				sAssetPath = "Announcer\\result aa";				break;
		case ANNOUNCER_RESULT_A:				sAssetPath = "Announcer\\result a";					break;
		case ANNOUNCER_RESULT_B:				sAssetPath = "Announcer\\result b";					break;
		case ANNOUNCER_RESULT_C:				sAssetPath = "Announcer\\result c";					break;
		case ANNOUNCER_RESULT_D:				sAssetPath = "Announcer\\result d";					break;
		case ANNOUNCER_RESULT_E:				sAssetPath = "Announcer\\result e";					break;
		case ANNOUNCER_TITLE:					sAssetPath = "Announcer\\title";					break;
		case ANNOUNCER_DIFFICULTY_COMMENT:		sAssetPath = "Announcer\\difficulty comment";		break;

		default:
			FatalError( ssprintf("Unhandled theme element %d", te) );
	}
	
	return sAssetPath;
}


CString ThemeManager::GetPathTo( ThemeElement te, CString sThemeName ) 
{
	CString sThemeDir = ThemeNameToThemeDir( sThemeName );

	CString sAssetPath = ElementToAssetPath( te );

	CString sAssetDir, sAssetFileName, sThrowAway;
	splitrelpath( sAssetPath, sAssetDir, sAssetFileName, sThrowAway );


	CStringArray arrayPossibleElementFileNames;		// fill this with the possible files

	///////////////////////////////////////
	// see if the current theme implements this element
	///////////////////////////////////////
	if( sAssetDir == "Graphics\\" )
	{
		GetDirListing( sThemeDir + sAssetDir + sAssetFileName + "*.sprite", arrayPossibleElementFileNames );
		GetDirListing( sThemeDir + sAssetDir + sAssetFileName + "*.png", arrayPossibleElementFileNames );
		GetDirListing( sThemeDir + sAssetDir + sAssetFileName + "*.jpg", arrayPossibleElementFileNames );
		GetDirListing( sThemeDir + sAssetDir + sAssetFileName + "*.bmp", arrayPossibleElementFileNames );
	}
	else if( sAssetDir == "Sounds\\" )
	{
		GetDirListing( sThemeDir + sAssetDir + sAssetFileName + ".set", arrayPossibleElementFileNames );
		GetDirListing( sThemeDir + sAssetDir + sAssetFileName + ".mp3", arrayPossibleElementFileNames );
		GetDirListing( sThemeDir + sAssetDir + sAssetFileName + ".wav", arrayPossibleElementFileNames );
	}
	else if( sAssetDir == "Fonts\\" )
	{
		GetDirListing( sThemeDir + sAssetDir + sAssetFileName + ".font", arrayPossibleElementFileNames );
	}
	else if( sAssetDir == "Announcer\\" )
	{
		// return the directory, and let the SoundSet discover all the samples in that directory
		if( DoesFileExist( sThemeDir + sAssetDir + sAssetFileName ) )
			return sThemeDir + sAssetDir + sAssetFileName;
	}
	else
	{
		FatalError( ssprintf("Unknown theme asset dir '%s'.", sAssetDir) );
	}

	if( arrayPossibleElementFileNames.GetSize() > 0 )
		return sThemeDir + sAssetDir + arrayPossibleElementFileNames[0];


	///////////////////////////////////////
	// the current theme does not implement this element.  Fall back to the default path.
	///////////////////////////////////////
	if( sAssetDir == "Graphics\\" )
	{
		GetDirListing( DEFAULT_THEME_DIR + sAssetDir + sAssetFileName + "*.sprite", arrayPossibleElementFileNames );
		GetDirListing( DEFAULT_THEME_DIR + sAssetDir + sAssetFileName + "*.png", arrayPossibleElementFileNames );
		GetDirListing( DEFAULT_THEME_DIR + sAssetDir + sAssetFileName + "*.jpg", arrayPossibleElementFileNames );
		GetDirListing( DEFAULT_THEME_DIR + sAssetDir + sAssetFileName + "*.bmp", arrayPossibleElementFileNames );
	}
	else if( sAssetDir == "Sounds\\" )
	{
		GetDirListing( DEFAULT_THEME_DIR + sAssetDir + sAssetFileName + ".set", arrayPossibleElementFileNames );
		GetDirListing( DEFAULT_THEME_DIR + sAssetDir + sAssetFileName + ".mp3", arrayPossibleElementFileNames );
		GetDirListing( DEFAULT_THEME_DIR + sAssetDir + sAssetFileName + ".wav", arrayPossibleElementFileNames );
	}
	else if( sAssetDir == "Fonts\\" )
	{
		GetDirListing( DEFAULT_THEME_DIR + sAssetDir + sAssetFileName + ".font", arrayPossibleElementFileNames );
	}
	else if( sAssetDir == "Announcer\\" )
	{
		// return the directory, and let the SoundSet discover all the samples in that directory
		if( DoesFileExist( DEFAULT_THEME_DIR + sAssetDir + sAssetFileName ) )
			return DEFAULT_THEME_DIR + sAssetDir + sAssetFileName;
	}
	else
	{
		FatalError( ssprintf("Unknown theme asset dir '%s'.", sAssetDir) );
	}

	if( arrayPossibleElementFileNames.GetSize() > 0 )
		return DEFAULT_THEME_DIR + sAssetDir + arrayPossibleElementFileNames[0];



	FatalError( ssprintf("The theme element '%s' does not exist in the current theme directory or the default theme directory.", sAssetDir + "\\" + sAssetFileName) );
	return "";
}
