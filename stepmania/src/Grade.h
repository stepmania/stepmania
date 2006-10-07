/* Grade - Mark the player receives after clearing a song. */

#ifndef GRADE_H
#define GRADE_H

#include "RageUtil.h"
#include "EnumHelper.h"

#define NUM_Grade_TierS 20
enum Grade 
{ 
	Grade_Tier01,	// = AAAA
	Grade_Tier02,	// = AAA
	Grade_Tier03,	// = AA
	Grade_Tier04,	// = A
	Grade_Tier05,	// = B
	Grade_Tier06,	// = C
	Grade_Tier07,	// = D
	Grade_Tier08,
	Grade_Tier09,
	Grade_Tier10,
	Grade_Tier11,
	Grade_Tier12,
	Grade_Tier13,
	Grade_Tier14,
	Grade_Tier15,
	Grade_Tier16,
	Grade_Tier17,
	Grade_Tier18,
	Grade_Tier19,
	Grade_Tier20,
	Grade_Failed,	// = E
	NUM_Grade, 
	GRADE_Invalid,
};
#define Grade_NoData GRADE_Invalid

/* This is in the header so the test sets don't require Grade.cpp (through PrefsManager),
 * since that pulls in ThemeManager. */
static inline RString GradeToString( Grade g )
{
	ASSERT_M( (g >= 0 && g<NUM_Grade) || g == Grade_NoData, ssprintf("grade = %d",g) );

	switch( g )
	{
	case Grade_NoData:	return "NoData";
	case Grade_Failed:	return "Failed";
	default:		return ssprintf("Tier%02d",g+1);
	}
}

RString GradeToOldString( Grade g );	// "AAA", "B", etc for backward compatibility.  Used in announcer
RString GradeToLocalizedString( Grade g );
Grade StringToGrade( const RString &s );
LuaDeclareType( Grade );
#define FOREACH_Grade( g ) FOREACH_ENUM2( Grade, g )
#define FOREACH_UsedGrade( g ) FOREACH_ENUM_N( Grade, THEME->GetMetricI("PlayerStageStats","NumGradeTiersUsed"), g )

#endif

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
