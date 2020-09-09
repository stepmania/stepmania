#include "global.h"
#include "Grade.h"
#include "RageUtil.h"
#include "ThemeManager.h"
#include "RageLog.h"
#include "EnumHelper.h"
#include "LuaManager.h"

LuaXType( Grade );

/** @brief The current number of grade tiers being used. */
ThemeMetric<int> NUM_GRADE_TIERS_USED("PlayerStageStats","NumGradeTiersUsed");

Grade GetNextPossibleGrade( Grade g )
{
	if( g < NUM_GRADE_TIERS_USED - 1 )
		return (Grade)(g+1);
	else if( g == NUM_GRADE_TIERS_USED - 1 )
		return Grade_Failed;
	else
		return Grade_Invalid;
}


RString GradeToLocalizedString( Grade g )
{
	RString s = GradeToString(g);
	if( !THEME->HasString("Grade",s) )
		return "???";
	return THEME->GetString( "Grade",s );
}

RString GradeToOldString( Grade g )
{
	// string is meant to be human readable
	switch( GradeToOldGrade(g) )
	{
	case Grade_Tier01:	return "AAAA";
	case Grade_Tier02:	return "AAA";
	case Grade_Tier03:	return "AA";
	case Grade_Tier04:	return "A";
	case Grade_Tier05:	return "B";
	case Grade_Tier06:	return "C";
	case Grade_Tier07:	return "D";
	case Grade_Failed:	return "E";
	case Grade_NoData:	return "N";
	default:		return "N";
	}
};

Grade GradeToOldGrade( Grade g )
{
	// There used to be 7 grades (plus fail) but grades can now be defined by themes.
	// So we need to re-scale the grade bands based on how many actual grades the theme defines.
	if( g < NUM_GRADE_TIERS_USED )
		g = (Grade)std::lround((double)g * Grade_Tier07 / (NUM_GRADE_TIERS_USED - 1));

	return g;
}

Grade StringToGrade( const RString &sGrade )
{
	RString s = sGrade;
	s.MakeUpper();

	// new style
	int iTier;
	if( sscanf(sGrade.c_str(),"Tier%02d",&iTier) == 1 && iTier >= 0 && iTier < NUM_Grade)
		return (Grade)(iTier-1);
	else if( s == "FAILED" )
		return Grade_Failed;
	else if( s == "NODATA" )
		return Grade_NoData;

	LOG->Warn( "Invalid grade: %s", sGrade.c_str() );
	return Grade_NoData;
};

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
