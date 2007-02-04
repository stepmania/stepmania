/* Definitions of metrics that are in the "Common" group */

#ifndef COMMON_METRICS_H
#define COMMON_METRICS_H

#include "ThemeMetric.h"
#include "PlayerNumber.h"
#include "Difficulty.h"
#include "GameConstantsAndTypes.h"
#include "LocalizedString.h"


//
// Types
//
class ThemeMetricDifficultiesToShow : public ThemeMetric<RString>
{
public:
	ThemeMetricDifficultiesToShow() { }
	ThemeMetricDifficultiesToShow( const RString& sGroup, const RString& sName );
	void Read();
	const vector<Difficulty> &GetValue() const;
private:
	vector<Difficulty> m_v;
};
class ThemeMetricCourseDifficultiesToShow : public ThemeMetric<RString>
{
public:
	ThemeMetricCourseDifficultiesToShow() { }
	ThemeMetricCourseDifficultiesToShow( const RString& sGroup, const RString& sName );
	void Read();
	const vector<CourseDifficulty> &GetValue() const;
private:
	vector<CourseDifficulty> m_v;
};
class ThemeMetricStepsTypesToShow : public ThemeMetric<RString>
{
public:
	ThemeMetricStepsTypesToShow() { }
	ThemeMetricStepsTypesToShow( const RString& sGroup, const RString& sName );
	void Read();
	const vector<StepsType> &GetValue() const;
private:
	vector<StepsType> m_v;
};


//
// Metrics
//
namespace CommonMetrics
{
	extern DynamicThemeMetric<RString>		INITIAL_SCREEN;
	extern ThemeMetric<RString>			FIRST_ATTRACT_SCREEN;
	extern ThemeMetric<RString>			DEFAULT_MODIFIERS;
	extern ThemeMetric<RString>			DEFAULT_CPU_MODIFIERS;
	extern ThemeMetric1D<apActorCommands>		PLAYER_COLOR;
	extern LocalizedString				WINDOW_TITLE;
	extern ThemeMetric<int>				MAX_COURSE_ENTRIES_BEFORE_VARIOUS;
	extern ThemeMetric<float>			TICK_EARLY_SECONDS;
	extern ThemeMetric<RString>			DEFAULT_NOTESKIN_NAME;
	extern ThemeMetricDifficultiesToShow		DIFFICULTIES_TO_SHOW;
	extern ThemeMetricCourseDifficultiesToShow	COURSE_DIFFICULTIES_TO_SHOW;
	extern ThemeMetricStepsTypesToShow		STEPS_TYPES_TO_SHOW;

	RString LocalizeOptionItem( const RString &s, bool bOptional );
};

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
