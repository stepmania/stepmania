#include "global.h"
#include "CommonMetrics.h"
#include "RageUtil.h"

#include "GameManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "ProductInfo.h"
#include "LuaManager.h"

#include <vector>


ThemeMetric<RString>		CommonMetrics::OPERATOR_MENU_SCREEN		("Common","OperatorMenuScreen");
ThemeMetric<RString>		CommonMetrics::FIRST_ATTRACT_SCREEN		("Common","FirstAttractScreen");
ThemeMetric<RString>		CommonMetrics::DEFAULT_MODIFIERS		("Common","DefaultModifiers" );
LocalizedString				CommonMetrics::WINDOW_TITLE				("Common","WindowTitle");
ThemeMetric<int>			CommonMetrics::MAX_COURSE_ENTRIES_BEFORE_VARIOUS("Common","MaxCourseEntriesBeforeShowVarious");
ThemeMetric<float>			CommonMetrics::TICK_EARLY_SECONDS		("ScreenGameplay","TickEarlySeconds");
ThemeMetric<RString>		CommonMetrics::DEFAULT_NOTESKIN_NAME	("Common","DefaultNoteSkinName");
ThemeMetricDifficultiesToShow	CommonMetrics::DIFFICULTIES_TO_SHOW		("Common","DifficultiesToShow");
ThemeMetricCourseDifficultiesToShow	CommonMetrics::COURSE_DIFFICULTIES_TO_SHOW	("Common","CourseDifficultiesToShow");
ThemeMetricStepsTypesToShow	CommonMetrics::STEPS_TYPES_TO_SHOW		("Common","StepsTypesToHide");
ThemeMetric<bool>			CommonMetrics::AUTO_SET_STYLE			("Common","AutoSetStyle");
ThemeMetric<int>			CommonMetrics::PERCENT_SCORE_DECIMAL_PLACES	("Common","PercentScoreDecimalPlaces");
ThemeMetric<RString>		CommonMetrics::IMAGES_TO_CACHE	("Common","ImageCache");

ThemeMetricDifficultiesToShow::ThemeMetricDifficultiesToShow( const RString& sGroup, const RString& sName ) :
	ThemeMetric<RString>(sGroup,sName)
{
	// re-read because ThemeMetric::ThemeMetric calls ThemeMetric::Read, not the derived one
	if( IsLoaded() )
		Read();
}
void ThemeMetricDifficultiesToShow::Read()
{
	ASSERT( GetName().Right(6) == "ToShow" );

	ThemeMetric<RString>::Read();

	m_v.clear();

	std::vector<RString> v;
	split( ThemeMetric<RString>::GetValue(), ",", v );
	if(v.empty())
	{
		LuaHelpers::ReportScriptError("DifficultiesToShow must have at least one entry.");
		return;
	}

	for (RString const &i : v)
	{
		Difficulty d = StringToDifficulty( i );
		if( d == Difficulty_Invalid )
		{
			LuaHelpers::ReportScriptErrorFmt("Unknown difficulty \"%s\" in CourseDifficultiesToShow.", i.c_str());
		}
		else
		{
			m_v.push_back( d );
		}
	}
}
const std::vector<Difficulty>& ThemeMetricDifficultiesToShow::GetValue() const { return m_v; }


ThemeMetricCourseDifficultiesToShow::ThemeMetricCourseDifficultiesToShow( const RString& sGroup, const RString& sName ) :
	ThemeMetric<RString>(sGroup,sName)
{
	// re-read because ThemeMetric::ThemeMetric calls ThemeMetric::Read, not the derived one
	if( IsLoaded() )
		Read();
}
void ThemeMetricCourseDifficultiesToShow::Read()
{
	ASSERT( GetName().Right(6) == "ToShow" );

	ThemeMetric<RString>::Read();

	m_v.clear();

	std::vector<RString> v;
	split( ThemeMetric<RString>::GetValue(), ",", v );
	if(v.empty())
	{
		LuaHelpers::ReportScriptError("CourseDifficultiesToShow must have at least one entry.");
		return;
	}

	for (RString const &i : v)
	{
		CourseDifficulty d = StringToDifficulty( i );
		if( d == Difficulty_Invalid )
		{
			LuaHelpers::ReportScriptErrorFmt("Unknown CourseDifficulty \"%s\" in CourseDifficultiesToShow.", i.c_str());
		}
		else
		{
			m_v.push_back( d );
		}
	}
}
const std::vector<CourseDifficulty>& ThemeMetricCourseDifficultiesToShow::GetValue() const { return m_v; }

static void RemoveStepsTypes( std::vector<StepsType>& inout, RString sStepsTypesToRemove )
{
	std::vector<RString> v;
	split( sStepsTypesToRemove, ",", v );
	if( v.size() == 0 ) return; // Nothing to do!

	// subtract StepsTypes
	for (RString const &i : v)
	{
		StepsType st = GAMEMAN->StringToStepsType(i);
		if( st == StepsType_Invalid )
		{
			LuaHelpers::ReportScriptErrorFmt( "Invalid StepsType value '%s' in '%s'", i.c_str(), sStepsTypesToRemove.c_str() );
			continue;
		}

		const std::vector<StepsType>::iterator iter = find( inout.begin(), inout.end(), st );
		if( iter != inout.end() )
			inout.erase( iter );
	}
}
ThemeMetricStepsTypesToShow::ThemeMetricStepsTypesToShow( const RString& sGroup, const RString& sName ) :
	ThemeMetric<RString>(sGroup,sName)
{
	// re-read because ThemeMetric::ThemeMetric calls ThemeMetric::Read, not the derived one
	if( IsLoaded() )
		Read();
}
void ThemeMetricStepsTypesToShow::Read()
{
	ASSERT( GetName().Right(6) == "ToHide" );

	ThemeMetric<RString>::Read();

	m_v.clear();
	GAMEMAN->GetStepsTypesForGame( GAMESTATE->m_pCurGame, m_v );

	RemoveStepsTypes( m_v, ThemeMetric<RString>::GetValue() );
}
const std::vector<StepsType>& ThemeMetricStepsTypesToShow::GetValue() const { return m_v; }


RString CommonMetrics::LocalizeOptionItem( const RString &s, bool bOptional )
{
	if( bOptional && !THEME->HasString("OptionNames",s) )
		return s;
	return THEME->GetString( "OptionNames", s );
}

LuaFunction( LocalizeOptionItem, CommonMetrics::LocalizeOptionItem(SArg(1),true) );

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
