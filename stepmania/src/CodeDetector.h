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
		CODE_DARK,
		CODE_CANCEL_ALL,
		CODE_NEXT_THEME,
		CODE_NEXT_ANNOUNCER,
		NUM_CODES	// leave this at the end
	};

	static void RefreshCacheItems();	// call this before checking codes, but call infrequently
	static bool EnteredEasierDifficulty( GameController controller );
	static bool EnteredHarderDifficulty( GameController controller );
	static bool EnteredNextSort( GameController controller );
	static bool DetectAndAdjustMusicOptions( GameController controller );
	static bool EnteredCode( GameController controller, Code code );
};




#endif
