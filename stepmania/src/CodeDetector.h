/* CodeDetector - Uses InputQueue to detect input of codes. */

#ifndef CODE_DETECTOR_H
#define CODE_DETECTOR_H

#include "GameInput.h"

enum Code {
	CODE_EASIER1,
	CODE_EASIER2,
	CODE_HARDER1,
	CODE_HARDER2,
	CODE_NEXT_SORT1,
	CODE_NEXT_SORT2,
	CODE_NEXT_SORT3,
	CODE_NEXT_SORT4,
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
	CODE_BW_NEXT_GROUP,
	CODE_BW_NEXT_GROUP2,
	CODE_SAVE_SCREENSHOT1,
	CODE_SAVE_SCREENSHOT2,
	CODE_CANCEL_ALL_PLAYER_OPTIONS,
	CODE_BACK_IN_EVENT_MODE,
	NUM_CODES	// leave this at the end
};
#define FOREACH_Code( c ) FOREACH_ENUM( Code, NUM_CODES, c )

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
	static void RefreshCacheItems( CString sClass="" );	// call this before checking codes, but call infrequently
	static bool EnteredEasierDifficulty( GameController controller );
	static bool EnteredHarderDifficulty( GameController controller );
	static bool EnteredNextSort( GameController controller );
	static bool EnteredModeMenu( GameController controller );
	static bool DetectAndAdjustMusicOptions( GameController controller );
	static bool EnteredCode( GameController controller, Code code );
	static bool EnteredNextBannerGroup( GameController controller );
};

#endif

/*
 * (c) 2001-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
