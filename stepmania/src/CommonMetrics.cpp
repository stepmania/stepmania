#include "global.h"
#include "CommonMetrics.h"
#include "RageUtil.h"


CString PLAYER_COLOR_NAME( size_t p ) { return ssprintf("ColorP%d",p+1); }

ThemeMetric<CString>	DIFFICULTIES_TO_SHOW		("Common","DifficultiesToShow");
ThemeMetric<CString>	INITIAL_SCREEN				("Common","InitialScreen");
ThemeMetric<CString>	FIRST_RUN_INITIAL_SCREEN	("Common","FirstRunInitialScreen");
ThemeMetric<CString>	DEFAULT_MODIFIERS			("Common","DefaultModifiers" );
ThemeMetric<CString>	DEFAULT_CPU_MODIFIERS		("Common","DefaultCpuModifiers" );
ThemeMetric<CString>	COURSE_DIFFICULTIES_TO_SHOW	("Common","CourseDifficultiesToShow");
ThemeMetric1D<RageColor,NUM_PLAYERS> PLAYER_COLOR	("Common",PLAYER_COLOR_NAME,NUM_PLAYERS);
ThemeMetric<float>		JOIN_PAUSE_SECONDS			("Common","JoinPauseSeconds");
ThemeMetric<CString>	WINDOW_TITLE				("Common","WindowTitle");


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
