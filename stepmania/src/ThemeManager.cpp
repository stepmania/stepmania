#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ThemeManager.h

 Desc: .

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "ThemeManager.h"



ThemeManager*	THEME = NULL;	// global object accessable from anywhere in the program




CString ThemeManager::GetPathTo( ThemeElement te, CString sThemeName ) 
{
	CString sThemeDir = ThemeNameToThemeDir( sThemeName );

	CString sAssetPath;		// fill this in below

	switch( te )
	{
		case GRAPHIC_TITLE_MENU_BACKGROUND:		sAssetPath = "Graphics\\title menu background";		break;
		case GRAPHIC_SELECT_STYLE_BACKGROUND:	sAssetPath = "Graphics\\select style background";	break;
		case GRAPHIC_SELECT_MUSIC_BACKGROUND:	sAssetPath = "Graphics\\select music background";	break;
		case GRAPHIC_OPTIONS_BACKGROUND:		sAssetPath = "Graphics\\options background";		break;
		case GRAPHIC_RESULT_BACKGROUND:			sAssetPath = "Graphics\\result background";			break;
		case GRAPHIC_SELECT_STYLE_TOP_EDGE:		sAssetPath = "Graphics\\select style top edge";		break;
		case GRAPHIC_SELECT_MUSIC_TOP_EDGE:		sAssetPath = "Graphics\\select music top edge";		break;
		case GRAPHIC_GAME_OPTIONS_TOP_EDGE:		sAssetPath = "Graphics\\game options top edge";		break;
		case GRAPHIC_PLAYER_OPTIONS_TOP_EDGE:	sAssetPath = "Graphics\\player options top edge";	break;
		case GRAPHIC_MUSIC_OPTIONS_TOP_EDGE:	sAssetPath = "Graphics\\music options top edge";	break;
		case GRAPHIC_RESULT_TOP_EDGE:			sAssetPath = "Graphics\\result top edge";			break;
		case GRAPHIC_FALLBACK_BANNER:		sAssetPath = "Graphics\\Fallback Banner";				break;
		case GRAPHIC_FALLBACK_BACKGROUND:	sAssetPath = "Graphics\\Fallback Background";			break;
		case GRAPHIC_COLOR_ARROW_GRAY_PART:	sAssetPath = "Graphics\\Color Arrow gray part 2x2";		break;
		case GRAPHIC_COLOR_ARROW_COLOR_PART:sAssetPath = "Graphics\\Color Arrow color part";		break;
		case GRAPHIC_GHOST_ARROW:			sAssetPath = "Graphics\\ghost arrow";					break;
		case GRAPHIC_HOLD_GHOST_ARROW:		sAssetPath = "Graphics\\hold ghost arrow";				break;
		case GRAPHIC_GRAY_ARROW:			sAssetPath = "Graphics\\gray arrow";					break;
		case GRAPHIC_JUDGEMENT:				sAssetPath = "Graphics\\judgement 1x9";					break;
		case GRAPHIC_MENU_BOTTOM_EDGE:		sAssetPath = "Graphics\\menu bottom edge";				break;
		case GRAPHIC_SCORE_FRAME:			sAssetPath = "Graphics\\score frame";					break;
		case GRAPHIC_LIFEMETER_FRAME:		sAssetPath = "Graphics\\Life Meter Frame";				break;
		case GRAPHIC_LIFEMETER_PILLS:		sAssetPath = "Graphics\\life meter pills 17x1";			break;
		case GRAPHIC_COMBO:					sAssetPath = "Graphics\\combo";							break;
		case GRAPHIC_CLOSING_STAR:			sAssetPath = "Graphics\\closing star";					break;
		case GRAPHIC_OPENING_STAR:			sAssetPath = "Graphics\\opening star";					break;
		case GRAPHIC_CAUTION:				sAssetPath = "Graphics\\Caution";						break;
		case GRAPHIC_READY:					sAssetPath = "Graphics\\Ready";							break;
		case GRAPHIC_HERE_WE_GO:			sAssetPath = "Graphics\\here we go";					break;
		case GRAPHIC_CLEARED:				sAssetPath = "Graphics\\cleared";						break;
		case GRAPHIC_FAILED:				sAssetPath = "Graphics\\failed";						break;
		case GRAPHIC_GRADES:				sAssetPath = "Graphics\\grades 1x8";					break;
		case GRAPHIC_KEEP_ALIVE:			sAssetPath = "Graphics\\keep alive";					break;
		case GRAPHIC_DANCER_P1:				sAssetPath = "Graphics\\dancer p1";						break;
		case GRAPHIC_DANCER_P2:				sAssetPath = "Graphics\\dancer p2";						break;
		case GRAPHIC_PAD_SINGLE:			sAssetPath = "Graphics\\Pad single";					break;
		case GRAPHIC_PAD_DOUBLE:			sAssetPath = "Graphics\\Pad double";					break;
		case GRAPHIC_STYLE_ICONS:			sAssetPath = "Graphics\\style icons 1x4";				break;
		case GRAPHIC_STYLE_EXPLANATIONS:	sAssetPath = "Graphics\\style explanations 1x8";		break;
		case GRAPHIC_MUSIC_SELECTION_HIGHLIGHT:	sAssetPath = "Graphics\\music selection highlight";	break;
		case GRAPHIC_STEPS_DESCRIPTION:		sAssetPath = "Graphics\\steps description 1x8";			break;
		case GRAPHIC_SECTION_BACKGROUND:	sAssetPath = "Graphics\\section background";			break;
		case GRAPHIC_MUSIC_SORT_ICONS:		sAssetPath = "Graphics\\music sort icons 1x5";			break;
		case GRAPHIC_MUSIC_STATUS_ICONS:	sAssetPath = "Graphics\\music status icons 1x4";		break;
		case GRAPHIC_DANGER:				sAssetPath = "Graphics\\danger";						break;
		case GRAPHIC_DANGER_BACKGROUND:		sAssetPath = "Graphics\\danger background";				break;

		case SOUND_FAILED:					sAssetPath = "Sounds\\failed";							break;
		case SOUND_ASSIST:					sAssetPath = "Sounds\\Assist";							break;
		case SOUND_SELECT:					sAssetPath = "Sounds\\select";							break;
		case SOUND_SWITCH_STYLE:			sAssetPath = "Sounds\\switch style";					break;
		case SOUND_SWITCH_MUSIC:			sAssetPath = "Sounds\\switch music";					break;
		case SOUND_SWITCH_SORT:				sAssetPath = "Sounds\\switch sort";						break;
		case SOUND_EXPAND:					sAssetPath = "Sounds\\expand";							break;
		case SOUND_SWITCH_STEPS:			sAssetPath = "Sounds\\switch steps";					break;
		case SOUND_TITLE_CHANGE:			sAssetPath = "Sounds\\title change";					break;
		case SOUND_MENU_SWOOSH:				sAssetPath = "Sounds\\menu swoosh";						break;
		case SOUND_MENU_BACK:				sAssetPath = "Sounds\\menu back";						break;
		case SOUND_TRAINING_MUSIC:			sAssetPath = "Sounds\\training music";					break;

		case FONT_OUTLINE:					sAssetPath = "Fonts\\Outline";							break;
		case FONT_NORMAL:					sAssetPath = "Fonts\\Normal";							break;
		case FONT_FUTURISTIC:				sAssetPath = "Fonts\\Futuristic";						break;
		case FONT_BOLD_NUMBERS:				sAssetPath = "Fonts\\Bold Numbers";						break;
		case FONT_LCD_NUMBERS:				sAssetPath = "Fonts\\LCD Numbers";						break;
		case FONT_FEET:						sAssetPath = "Fonts\\Feet";								break;
		case FONT_COMBO_NUMBERS:			sAssetPath = "Fonts\\MAX Numbers";						break;
		case FONT_SCORE_NUMBERS:			sAssetPath = "Fonts\\Bold Numbers";						break;

		case ANNOUNCER_ATTRACT:				sAssetPath = "Announcer\\attract";						break;
		case ANNOUNCER_BAD_COMMENT:			sAssetPath = "Announcer\\bad comment";					break;
		case ANNOUNCER_CAUTION:				sAssetPath = "Announcer\\caution";						break;
		case ANNOUNCER_CLEARED:				sAssetPath = "Announcer\\cleared";						break;
		case ANNOUNCER_FAIL_COMMENT:		sAssetPath = "Announcer\\fail comment";					break;
		case ANNOUNCER_GAME_OVER:			sAssetPath = "Announcer\\game over";					break;
		case ANNOUNCER_GOOD_COMMENT:		sAssetPath = "Announcer\\good comment";					break;
		case ANNOUNCER_HERE_WE_GO:			sAssetPath = "Announcer\\here we go";					break;
		case ANNOUNCER_MUSIC_COMMENT:		sAssetPath = "Announcer\\music comment";				break;
		case ANNOUNCER_READY:				sAssetPath = "Announcer\\ready";						break;
		case ANNOUNCER_READY_LAST:			sAssetPath = "Announcer\\ready last";					break;
		case ANNOUNCER_RESULT_AAA:			sAssetPath = "Announcer\\result aaa";					break;
		case ANNOUNCER_RESULT_AA:			sAssetPath = "Announcer\\result aa";					break;
		case ANNOUNCER_RESULT_A:			sAssetPath = "Announcer\\result a";						break;
		case ANNOUNCER_RESULT_B:			sAssetPath = "Announcer\\result b";						break;
		case ANNOUNCER_RESULT_C:			sAssetPath = "Announcer\\result c";						break;
		case ANNOUNCER_RESULT_D:			sAssetPath = "Announcer\\result d";						break;
		case ANNOUNCER_RESULT_E:			sAssetPath = "Announcer\\result e";						break;
		case ANNOUNCER_TITLE:				sAssetPath = "Announcer\\title";						break;


		default:
			RageError( ssprintf("Unhandled theme element %d", te) );
	}

	CString sAssetDir, sAssetFileName, sThrowAway;
	splitrelpath( sAssetPath, sAssetDir, sAssetFileName, sThrowAway );


	CStringArray arrayPossibleElementFileNames;		// fill this with the possible files

	///////////////////////////////////////
	// see if the current theme implements this element
	///////////////////////////////////////
	if( sAssetDir == "Graphics\\" )
	{
		GetDirListing( sThemeDir + sAssetDir + sAssetFileName + ".sprite", arrayPossibleElementFileNames );
		GetDirListing( sThemeDir + sAssetDir + sAssetFileName + ".png", arrayPossibleElementFileNames );
		GetDirListing( sThemeDir + sAssetDir + sAssetFileName + ".jpg", arrayPossibleElementFileNames );
		GetDirListing( sThemeDir + sAssetDir + sAssetFileName + ".bmp", arrayPossibleElementFileNames );
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
		RageError( ssprintf("Unknown theme asset dir '%s'.", sAssetDir) );
	}

	if( arrayPossibleElementFileNames.GetSize() > 0 )
		return sThemeDir + sAssetDir + arrayPossibleElementFileNames[0];


	///////////////////////////////////////
	// the current theme does not implement this element.  Fall back to the default path.
	///////////////////////////////////////
	if( sAssetDir == "Graphics\\" )
	{
		GetDirListing( DEFAULT_THEME_DIR + sAssetDir + sAssetFileName + ".sprite", arrayPossibleElementFileNames );
		GetDirListing( DEFAULT_THEME_DIR + sAssetDir + sAssetFileName + ".png", arrayPossibleElementFileNames );
		GetDirListing( DEFAULT_THEME_DIR + sAssetDir + sAssetFileName + ".jpg", arrayPossibleElementFileNames );
		GetDirListing( DEFAULT_THEME_DIR + sAssetDir + sAssetFileName + ".bmp", arrayPossibleElementFileNames );
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
		RageError( ssprintf("Unknown theme asset dir '%s'.", sAssetDir) );
	}

	if( arrayPossibleElementFileNames.GetSize() > 0 )
		return DEFAULT_THEME_DIR + sAssetDir + arrayPossibleElementFileNames[0];



	RageError( ssprintf("The theme element '%s' does not exist in the current theme directory or the default theme directory.", sAssetDir + "\\" + sAssetFileName) );
	return "";
}
