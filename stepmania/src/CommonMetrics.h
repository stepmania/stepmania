/* Definitions of metrics that are in the "Common" group */

#ifndef COMMON_METRICS_H
#define COMMON_METRICS_H

#include "ThemeMetric.h"
#include "PlayerNumber.h"
#include "Difficulty.h"

extern ThemeMetric<CString>		INITIAL_SCREEN;
extern ThemeMetric<CString>		FIRST_RUN_INITIAL_SCREEN;
extern ThemeMetric<CString>		DEFAULT_MODIFIERS;
extern ThemeMetric<CString>		DEFAULT_CPU_MODIFIERS;
extern ThemeMetric1D<RageColor> PLAYER_COLOR;
extern ThemeMetric<float>		JOIN_PAUSE_SECONDS;
extern ThemeMetric<CString>		WINDOW_TITLE;
extern ThemeMetric<bool>		HOME_EDIT_MODE;
extern ThemeMetric<int>			MAX_STEPS_LOADED_FROM_PROFILE;
extern ThemeMetric<int>			MAX_COURSE_ENTRIES_BEFORE_VARIOUS;
extern ThemeMetric<float>		TICK_EARLY_SECONDS;

CString THEME_OPTION_ITEM( CString s, bool bOptional );

namespace CommonMetrics
{
	const set<Difficulty>& GetDifficultiesToShow();
	const set<CourseDifficulty>& GetCourseDifficultiesToShow();
}

#endif

/*
 * (c) 2003-2004 Chris Danford
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
