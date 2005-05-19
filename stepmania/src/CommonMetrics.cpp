#include "global.h"
#include "CommonMetrics.h"
#include "RageUtil.h"
#include "Foreach.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameState.h"


CString PLAYER_COLOR_NAME( size_t p ) { return ssprintf("ColorP%d",int(p+1)); }

ThemeMetric<CString>	INITIAL_SCREEN						("Common","InitialScreen");
ThemeMetric<CString>	DEFAULT_MODIFIERS					("Common","DefaultModifiers" );
ThemeMetric<CString>	DEFAULT_CPU_MODIFIERS				("Common","DefaultCpuModifiers" );
ThemeMetric1D<RageColor> PLAYER_COLOR						("Common",PLAYER_COLOR_NAME,NUM_PLAYERS);
ThemeMetric<CString>	WINDOW_TITLE						("Common","WindowTitle");
ThemeMetricEnum<EditMode>	EDIT_MODE						("Common","EditMode");
ThemeMetric<int>		MAX_COURSE_ENTRIES_BEFORE_VARIOUS	("Common","MaxCourseEntriesBeforeShowVarious");
ThemeMetric<float>		TICK_EARLY_SECONDS					("ScreenGameplay","TickEarlySeconds");
ThemeMetricDifficultiesToShow		DIFFICULTIES_TO_SHOW		("Common","DifficultiesToShow");
ThemeMetricCourseDifficultiesToShow	COURSE_DIFFICULTIES_TO_SHOW	("Common","CourseDifficultiesToShow");
ThemeMetricStepsTypesToShow			STEPS_TYPES_TO_SHOW			("Common","StepsTypesToHide");


ThemeMetricDifficultiesToShow::ThemeMetricDifficultiesToShow( const CString& sGroup, const CString& sName ) : 
	ThemeMetric<CString>(sGroup,sName)
{
	ASSERT( sName.Right(6) == "ToShow" );

	// re-read because ThemeMetric::ThemeMetric calls ThemeMetric::Read, not the derived one
	if( IsLoaded() )
		Read();
}
void ThemeMetricDifficultiesToShow::Read()
{
	ThemeMetric<CString>::Read();

	m_v.clear();

	CStringArray v;
	split( ThemeMetric<CString>::GetValue(), ",", v );
	ASSERT( v.size() > 0 );

	FOREACH_CONST( CString, v, i )
	{
		Difficulty d = StringToDifficulty( *i );
		if( d == DIFFICULTY_INVALID )
			RageException::Throw( "Unknown difficulty \"%s\" in CourseDifficultiesToShow", i->c_str() );
		m_v.push_back( d );
	}
}
const vector<Difficulty>& ThemeMetricDifficultiesToShow::GetValue() { return m_v; }


ThemeMetricCourseDifficultiesToShow::ThemeMetricCourseDifficultiesToShow( const CString& sGroup, const CString& sName ) : 
	ThemeMetric<CString>(sGroup,sName)
{
	ASSERT( sName.Right(6) == "ToShow" );

	// re-read because ThemeMetric::ThemeMetric calls ThemeMetric::Read, not the derived one
	if( IsLoaded() )
		Read();
}
void ThemeMetricCourseDifficultiesToShow::Read()
{
	ThemeMetric<CString>::Read();

	m_v.clear();

	CStringArray v;
	split( ThemeMetric<CString>::GetValue(), ",", v );
	ASSERT( v.size() > 0 );

	FOREACH_CONST( CString, v, i )
	{
		CourseDifficulty d = StringToCourseDifficulty( *i );
		if( d == DIFFICULTY_INVALID )
			RageException::Throw( "Unknown CourseDifficulty \"%s\" in CourseDifficultiesToShow", i->c_str() );
		m_v.push_back( d );
	}
}
const vector<CourseDifficulty>& ThemeMetricCourseDifficultiesToShow::GetValue() { return m_v; }


static void RemoveStepsTypes( vector<StepsType>& inout, CString sStepsTypesToRemove )
{
	CStringArray v;
	split( sStepsTypesToRemove, ",", v );
	ASSERT( v.size() > 0 );

	// subtract StepsTypes
	FOREACH_CONST( CString, v, i )
	{
		StepsType st = GameManager::StringToStepsType(*i);
		if( st == STEPS_TYPE_INVALID )
			LOG->Warn( "Invalid StepsType value '%s' in '%s'", i->c_str(), sStepsTypesToRemove.c_str() );

		const vector<StepsType>::iterator iter = find( inout.begin(), inout.end(), st );
		if( iter != inout.end() )
			inout.erase( iter );
	}
}
ThemeMetricStepsTypesToShow::ThemeMetricStepsTypesToShow( const CString& sGroup, const CString& sName ) : 
	ThemeMetric<CString>(sGroup,sName)
{
	ASSERT( sName.Right(6) == "ToHide" );

	// re-read because ThemeMetric::ThemeMetric calls ThemeMetric::Read, not the derived one
	if( IsLoaded() )
		Read();
}
void ThemeMetricStepsTypesToShow::Read()
{
	ThemeMetric<CString>::Read();

	m_v.clear();
	GAMEMAN->GetStepsTypesForGame( GAMESTATE->m_pCurGame, m_v );

	RemoveStepsTypes( m_v, ThemeMetric<CString>::GetValue() );
}
const vector<StepsType>& ThemeMetricStepsTypesToShow::GetValue() { return m_v; }


CString THEME_OPTION_ITEM( CString s, bool bOptional )
{
	if( bOptional && !THEME->HasMetric("OptionNames",s) )
		return s;
	return THEME->GetMetric( "OptionNames", s );
}


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
