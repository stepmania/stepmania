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
	static void RefreshCacheItems();	// call this before checking codes, but call infrequently
	static bool EnteredEasierDifficulty( GameController controller );
	static bool EnteredHarderDifficulty( GameController controller );
	static bool EnteredNextSort( GameController controller );
	static bool DetectAndAdjustOptions( GameController controller );
};




#endif
