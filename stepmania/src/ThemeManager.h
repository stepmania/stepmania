/*
-----------------------------------------------------------------------------
 File: ThemeManager.h

 Desc: .

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _ThemeManager_H_
#define _ThemeManager_H_


#include "RageUtil.h"


enum ThemeElement { 
	GRAPHIC_TITLE_MENU_BACKGROUND,
	GRAPHIC_SELECT_STYLE_BACKGROUND,
	GRAPHIC_SELECT_MUSIC_BACKGROUND,
	GRAPHIC_OPTIONS_BACKGROUND,
	GRAPHIC_RESULT_BACKGROUND,
	GRAPHIC_SELECT_STYLE_TOP_EDGE,
	GRAPHIC_SELECT_MUSIC_TOP_EDGE,
	GRAPHIC_GAME_OPTIONS_TOP_EDGE,
	GRAPHIC_PLAYER_OPTIONS_TOP_EDGE,
	GRAPHIC_MUSIC_OPTIONS_TOP_EDGE,
	GRAPHIC_RESULT_TOP_EDGE,
	GRAPHIC_FALLBACK_BANNER,
	GRAPHIC_FALLBACK_BACKGROUND,
	GRAPHIC_COLOR_ARROW_GRAY_PART,
	GRAPHIC_COLOR_ARROW_COLOR_PART,
	GRAPHIC_GHOST_ARROW,			
	GRAPHIC_BRIGHT_GHOST_ARROW,			
	GRAPHIC_HOLD_GHOST_ARROW,			
	GRAPHIC_GRAY_ARROW,		
	GRAPHIC_JUDGEMENT,	
	GRAPHIC_MENU_BOTTOM_EDGE,
	GRAPHIC_SCORE_FRAME,	
	GRAPHIC_LIFEMETER_FRAME,
	GRAPHIC_LIFEMETER_PILLS,
	GRAPHIC_COMBO,		
	GRAPHIC_CLOSING_STAR,
	GRAPHIC_OPENING_STAR,
	GRAPHIC_CAUTION,
	GRAPHIC_READY,	
	GRAPHIC_HERE_WE_GO,	
	GRAPHIC_CLEARED,
	GRAPHIC_FAILED,	
	GRAPHIC_GRADES,	
	GRAPHIC_KEEP_ALIVE,	
	GRAPHIC_DANCER_P1,
	GRAPHIC_DANCER_P2,
	GRAPHIC_PAD_SINGLE,	
	GRAPHIC_PAD_DOUBLE,
	GRAPHIC_PAD_SOLO,
	GRAPHIC_STYLE_ICONS,
	GRAPHIC_STYLE_EXPLANATIONS,
	GRAPHIC_MUSIC_SELECTION_HIGHLIGHT,
	GRAPHIC_STEPS_DESCRIPTION,
	GRAPHIC_SECTION_BACKGROUND,
	GRAPHIC_MUSIC_SORT_ICONS,
	GRAPHIC_MUSIC_STATUS_ICONS,
	GRAPHIC_DANGER_TEXT,
	GRAPHIC_DANGER_BACKGROUND,
	GRAPHIC_ARROWS_LEFT,
	GRAPHIC_ARROWS_RIGHT,
	
	SOUND_FAILED,	
	SOUND_ASSIST,	
	SOUND_SELECT,	
	SOUND_SWITCH_STYLE,	
	SOUND_SWITCH_MUSIC,	
	SOUND_SWITCH_SORT,	
	SOUND_EXPAND,	
	SOUND_SWITCH_STEPS,	
	SOUND_TITLE_CHANGE,	
	SOUND_MENU_SWOOSH,	
	SOUND_MENU_BACK,	
	SOUND_TRAINING_MUSIC,	
	SOUND_INVALID,	

	FONT_OUTLINE,	
	FONT_NORMAL,
	FONT_FUTURISTIC,
	FONT_BOLD_NUMBERS,	
	FONT_LCD_NUMBERS,	
	FONT_FEET,	
	FONT_COMBO_NUMBERS,	
	FONT_SCORE_NUMBERS,	

	ANNOUNCER_ATTRACT,
	ANNOUNCER_BAD_COMMENT,	
	ANNOUNCER_CAUTION,
	ANNOUNCER_CLEARED,
	ANNOUNCER_FAIL_COMMENT,
	ANNOUNCER_GAME_OVER,
	ANNOUNCER_GOOD_COMMENT,
	ANNOUNCER_HERE_WE_GO,
	ANNOUNCER_MUSIC_COMMENT,
	ANNOUNCER_READY,
	ANNOUNCER_READY_LAST,
	ANNOUNCER_RESULT_AAA,
	ANNOUNCER_RESULT_AA,
	ANNOUNCER_RESULT_A,
	ANNOUNCER_RESULT_B,
	ANNOUNCER_RESULT_C,
	ANNOUNCER_RESULT_D,
	ANNOUNCER_RESULT_E,
	ANNOUNCER_TITLE,


	NUM_THEME_ELEMENTS	// leave this at the end
};



const CString DEFAULT_THEME_NAME = "default";
const CString DEFAULT_THEME_DIR  = "Themes\\default\\";



class ThemeManager
{
public:
	ThemeManager()
	{
		CStringArray arrayThemeNames;
		GetThemeNames( arrayThemeNames );
		for( int i=0; i<arrayThemeNames.GetSize(); i++ )
			AssertThemeIsComplete( arrayThemeNames[i] );

		SetTheme( DEFAULT_THEME_NAME );
	};

	void GetThemeNames( CStringArray& AddTo )
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
	};

	bool SetTheme( CString sThemeName )		// return false if theme doesn't exist
	{
		sThemeName.MakeLower();
		m_sCurThemeName = sThemeName;
		CString sThemeDir = ThemeNameToThemeDir( m_sCurThemeName );

		if( !DoesFileExist( sThemeDir ) )
		{
			RageError( ssprintf( "The theme in diretory '%' could not be loaded.", sThemeDir ) );
			return false;
		}

		return true;
	};

	void AssertThemeIsComplete( CString sThemeName )		// return false if theme doesn't exist
	{
		for( int i=0; i<NUM_THEME_ELEMENTS; i++ )
		{
			if( GetPathTo( (ThemeElement)i, sThemeName ) == "" )
				RageError( ssprintf( "The theme element for theme '%s' called '%s' could not be found.", sThemeName, ElementToAssetPath((ThemeElement)i) ) );
		}
	};
	
	CString ElementToAssetPath( ThemeElement te );

	CString GetPathTo( ThemeElement te )
	{
		return GetPathTo( te, m_sCurThemeName );
	};
	CString GetPathTo( ThemeElement te, CString sThemeName );



private:

	CString ThemeNameToThemeDir( CString sThemeName )
	{
		return ssprintf( "Themes\\%s\\", sThemeName );
	}

	CString m_sCurThemeName;
};




extern ThemeManager*	THEME;	// global and accessable from anywhere in our program


#endif
