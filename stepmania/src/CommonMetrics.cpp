#include "global.h"
#include "CommonMetrics.h"
#include "RageUtil.h"
#include "Foreach.h"


CString PLAYER_COLOR_NAME( size_t p ) { return ssprintf("ColorP%d",p+1); }

ThemeMetric<CString>	INITIAL_SCREEN					("Common","InitialScreen");
ThemeMetric<CString>	FIRST_RUN_INITIAL_SCREEN		("Common","FirstRunInitialScreen");
ThemeMetric<CString>	DEFAULT_MODIFIERS				("Common","DefaultModifiers" );
ThemeMetric<CString>	DEFAULT_CPU_MODIFIERS			("Common","DefaultCpuModifiers" );
ThemeMetric1D<RageColor> PLAYER_COLOR					("Common",PLAYER_COLOR_NAME,NUM_PLAYERS);
ThemeMetric<float>		JOIN_PAUSE_SECONDS				("Common","JoinPauseSeconds");
ThemeMetric<CString>	WINDOW_TITLE					("Common","WindowTitle");
ThemeMetric<bool>		HOME_EDIT_MODE					("Common","HomeEditMode");
ThemeMetric<int>		MAX_STEPS_LOADED_FROM_PROFILE	("Common","MaxStepsLoadedFromProfile");


class ThemeMetricDifficultiesToShow : ThemeMetric<CString>
{
public:
	set<Difficulty> m_v;

	ThemeMetricDifficultiesToShow() : ThemeMetric<CString>("Common","DifficultiesToShow") {}
	void Read()
	{
		ThemeMetric<CString>::Read();

		m_v.clear();

		CStringArray v;
		split( GetValue(), ",", v );
		ASSERT( v.size() > 0 );

		FOREACH_CONST( CString, v, i )
		{
			Difficulty d = StringToDifficulty( *i );
			if( d == DIFFICULTY_INVALID )
				RageException::Throw( "Unknown difficulty \"%s\" in CourseDifficultiesToShow", i->c_str() );
			m_v.insert( d );
		}
	}
};
ThemeMetricDifficultiesToShow	DIFFICULTIES_TO_SHOW;
const set<Difficulty>& CommonMetrics::GetDifficultiesToShow() { return DIFFICULTIES_TO_SHOW.m_v; }


class ThemeMetricCourseDifficultiesToShow : ThemeMetric<CString>
{
public:
	set<CourseDifficulty> m_v;

	ThemeMetricCourseDifficultiesToShow() : ThemeMetric<CString>("Common","CourseDifficultiesToShow") {}
	void Read()
	{
		ThemeMetric<CString>::Read();

		m_v.clear();

		CStringArray v;
		split( GetValue(), ",", v );
		ASSERT( v.size() > 0 );

		FOREACH_CONST( CString, v, i )
		{
			CourseDifficulty d = StringToCourseDifficulty( *i );
			if( d == DIFFICULTY_INVALID )
				RageException::Throw( "Unknown CourseDifficulty \"%s\" in CourseDifficultiesToShow", i->c_str() );
			m_v.insert( d );
		}
	}
};
ThemeMetricCourseDifficultiesToShow	COURSE_DIFFICULTIES_TO_SHOW;
const set<CourseDifficulty>& CommonMetrics::GetCourseDifficultiesToShow() { return COURSE_DIFFICULTIES_TO_SHOW.m_v; }



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
