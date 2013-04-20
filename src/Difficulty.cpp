#include "global.h"
#include "Difficulty.h"
#include "GameState.h"
#include "ThemeMetric.h"
#include "LuaManager.h"
#include "LocalizedString.h"
#include "GameConstantsAndTypes.h"
#include "GameManager.h"
#include "Steps.h"
#include "Trail.h"

static const char *DifficultyNames[] = {
	"Beginner",
	"Easy",
	"Medium",
	"Hard",
	"Challenge",
	"Edit",
};
XToString( Difficulty );
StringToX( Difficulty );
LuaXType( Difficulty );

const RString &CourseDifficultyToLocalizedString( CourseDifficulty x )
{
	static auto_ptr<LocalizedString> g_CourseDifficultyName[NUM_Difficulty];
	if( g_CourseDifficultyName[0].get() == NULL )
	{
		FOREACH_ENUM( Difficulty,i)
		{
			auto_ptr<LocalizedString> ap( new LocalizedString("CourseDifficulty", DifficultyToString(i)) );
			g_CourseDifficultyName[i] = ap;
		}
	}
	return g_CourseDifficultyName[x]->GetValue();
}

LuaFunction( CourseDifficultyToLocalizedString, CourseDifficultyToLocalizedString(Enum::Check<Difficulty>(L, 1)) );

CourseDifficulty GetNextShownCourseDifficulty( CourseDifficulty cd )
{
	for( CourseDifficulty d=(CourseDifficulty)(cd+1); d<NUM_Difficulty; enum_add(d, 1) )
	{
		if( GAMESTATE->IsCourseDifficultyShown(d) )
			return d;
	}
	return Difficulty_Invalid;
}

Difficulty OldStyleStringToDifficulty( const RString& sDC )
{
	RString s2 = sDC;
	s2.MakeLower();
	if( s2 == "beginner" )			return Difficulty_Beginner;
	else if( s2 == "easy" )		return Difficulty_Easy;
	else if( s2 == "basic" )		return Difficulty_Easy;
	else if( s2 == "light" )		return Difficulty_Easy;
	else if( s2 == "medium" )		return Difficulty_Medium;
	else if( s2 == "another" )		return Difficulty_Medium;
	else if( s2 == "trick" )		return Difficulty_Medium;
	else if( s2 == "standard" )	return Difficulty_Medium;
	else if( s2 == "difficult")	return Difficulty_Medium;
	else if( s2 == "hard" )		return Difficulty_Hard;
	else if( s2 == "ssr" )			return Difficulty_Hard;
	else if( s2 == "maniac" )		return Difficulty_Hard;
	else if( s2 == "heavy" )		return Difficulty_Hard;
	else if( s2 == "smaniac" )		return Difficulty_Challenge;
	else if( s2 == "challenge" )	return Difficulty_Challenge;
	else if( s2 == "expert" )		return Difficulty_Challenge;
	else if( s2 == "oni" )			return Difficulty_Challenge;
	else if( s2 == "edit" )		return Difficulty_Edit;
	else							return Difficulty_Invalid;
}

static ThemeMetric<RString> NAMES("CustomDifficulty","Names");

RString GetCustomDifficulty( StepsType st, Difficulty dc, CourseType ct )
{
	/* XXX GAMEMAN->GetStepsTypeInfo( StepsType_Invalid ) will crash. I'm not
	 * sure what the correct behavior in this case should be. Should we still
	 * allow custom difficulties? Why do we not allow custom difficulties for
	 * Edit? - Steve */
	// CustomDifficulty for Edit defeats the purpose of the edit's name. -aj
	if( st == StepsType_Invalid )
	{
		/* This is not totally necessary since DifficultyToString() will
		 * return "", but the comment there says that the caller should
		 * really be checking for invalid values. */
		if( dc == Difficulty_Invalid )
			return RString();
		return DifficultyToString( dc );
	}

	if( dc == Difficulty_Edit )
	{
		return "Edit";
	}
	// OPTIMIZATION OPPORTUNITY: cache these metrics and cache the splitting
	vector<RString> vsNames;
	split( NAMES, ",", vsNames );
	FOREACH( RString, vsNames, sName )
	{
		ThemeMetric<StepsType> STEPS_TYPE("CustomDifficulty",(*sName)+"StepsType");
		if( STEPS_TYPE == StepsType_Invalid  ||  st == STEPS_TYPE )	// match
		{
			ThemeMetric<Difficulty> DIFFICULTY("CustomDifficulty",(*sName)+"Difficulty");
			if( DIFFICULTY == Difficulty_Invalid  ||  dc == DIFFICULTY )	// match
			{
				ThemeMetric<CourseType> COURSE_TYPE("CustomDifficulty",(*sName)+"CourseType");
				if( COURSE_TYPE == CourseType_Invalid  ||  ct == COURSE_TYPE )	// match
				{
					ThemeMetric<RString> STRING("CustomDifficulty",(*sName)+"String");
					return STRING.GetValue();
				}
			}
		}
	}
	// no matching CustomDifficulty, so use a regular difficulty name
	if( dc == Difficulty_Invalid )
		return RString();
	return DifficultyToString( dc );
}

LuaFunction( GetCustomDifficulty, GetCustomDifficulty(Enum::Check<StepsType>(L,1), Enum::Check<Difficulty>(L, 2), Enum::Check<CourseType>(L, 3, true)) );

RString CustomDifficultyToLocalizedString( const RString &sCustomDifficulty )
{
	return THEME->GetString( "CustomDifficulty", sCustomDifficulty );
}

LuaFunction( CustomDifficultyToLocalizedString, CustomDifficultyToLocalizedString(SArg(1)) );


RString StepsToCustomDifficulty( const Steps *pSteps )
{
	return GetCustomDifficulty( pSteps->m_StepsType, pSteps->GetDifficulty(), CourseType_Invalid );
}

RString TrailToCustomDifficulty( const Trail *pTrail )
{
	return GetCustomDifficulty( pTrail->m_StepsType, pTrail->m_CourseDifficulty, pTrail->m_CourseType );
}

#include "LuaBinding.h"

LuaFunction( StepsToCustomDifficulty, StepsToCustomDifficulty(Luna<Steps>::check(L, 1)) );
LuaFunction( TrailToCustomDifficulty, TrailToCustomDifficulty(Luna<Trail>::check(L, 1)) );

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
