#ifndef CODEDETECTOR_H
#define CODEDETECTOR_H
/*
-----------------------------------------------------------------------------
 Class: CodeDetector

 Desc: Uses InputQueue to detect input of codes that would affect player and song options.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameInput.h"

struct CodeItem
{
public:
	bool Load( CString sButtonsNames );
	bool EnteredCode( GameController controller ) const;

private:
	vector<GameButton> buttons;
	enum Type
	{ 
		sequence,		// press the buttons in sequence
		hold_and_press,	// hold the first iNumButtons-1 buttons, then press the last
		tap				// press all buttons simultaneously
	};
	Type m_Type;
	float fMaxSecondsBack;
};	

class CodeDetector
{
public:
	enum Code {
		CODE_EASIER1,
		CODE_EASIER2,
		CODE_HARDER1,
		CODE_HARDER2,
		CODE_NEXT_SORT1,
		CODE_NEXT_SORT2,
		CODE_NEXT_SORT3,
		CODE_NEXT_SORT4,
		CODE_SORT_MENU1,
		CODE_SORT_MENU2,
		CODE_MODE_MENU1,
		CODE_MODE_MENU2,
		CODE_MIRROR,
		CODE_LEFT,
		CODE_RIGHT,
		CODE_SHUFFLE,
		CODE_SUPER_SHUFFLE,
		CODE_NEXT_TRANSFORM,
		CODE_NEXT_SCROLL_SPEED,
		CODE_PREVIOUS_SCROLL_SPEED,
		CODE_NEXT_ACCEL,
		CODE_NEXT_EFFECT,
		CODE_NEXT_APPEARANCE,
		CODE_NEXT_TURN,
		CODE_REVERSE,
		CODE_HOLDS,
		CODE_MINES,
		CODE_DARK,
		CODE_HIDDEN,
		CODE_RANDOMVANISH,
		CODE_CANCEL_ALL,
		CODE_NEXT_THEME,
		CODE_NEXT_THEME2,
		CODE_NEXT_ANNOUNCER,
		CODE_NEXT_ANNOUNCER2,
		CODE_NEXT_GAME,
		CODE_NEXT_GAME2,
		CODE_BW_NEXT_GROUP,
		CODE_BW_NEXT_GROUP2,
		NUM_CODES	// leave this at the end
	};

	static void RefreshCacheItems();	// call this before checking codes, but call infrequently
	static bool EnteredEasierDifficulty( GameController controller );
	static bool EnteredHarderDifficulty( GameController controller );
	static bool EnteredNextSort( GameController controller );
	static bool EnteredSortMenu( GameController controller );
	static bool EnteredModeMenu( GameController controller );
	static bool DetectAndAdjustMusicOptions( GameController controller );
	static bool EnteredCode( GameController controller, Code code );
	static bool EnteredNextBannerGroup( GameController controller );
};




#endif
