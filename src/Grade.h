#ifndef GRADE_H
#define GRADE_H

#include "RageUtil.h"
#include "EnumHelper.h"
#include "ThemeMetric.h"

/** @brief The list of grading tiers available.
 *
 * TODO: Look into a more flexible system without a fixed number of grades. -Wolfman2000
 */
enum Grade
{ 
	Grade_Tier01,	/**< Usually an AAAA */
	Grade_Tier02,	/**< Usually an AAA */
	Grade_Tier03,	/**< Usually an AA */
	Grade_Tier04,	/**< Usually an A */
	Grade_Tier05,	/**< Usually a B */
	Grade_Tier06,	/**< Usually a C */
	Grade_Tier07,	/**< Usually a D */
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
	Grade_Failed,	/**< Usually an E */
	NUM_Grade, 
	Grade_Invalid,
};
/** @brief Have an alternative for if there is no data for grading. */
#define Grade_NoData Grade_Invalid

/**
 * @brief Convert the grade supplied to a string representation.
 *
 * This is in the header so the test sets don't require Grade.cpp (through PrefsManager),
 * since that pulls in ThemeManager.
 * @param g the grade to convert.
 * @return the string reprsentation.
 */
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

/**
 * @brief Convert to the old version styled grade strings.
 *
 * This is mainly for backward compatibility purposes, but the announcer
 * also uses it. Think "AAA", "B", etc.
 * This is only referenced in ScreenEvaluation at the moment.
 * @param g the current Grade.
 * @return the old styled grade string. */
RString GradeToOldString( Grade g );
Grade GradeToOldGrade( Grade g );
RString GradeToLocalizedString( Grade g );
/**
 * @brief Convert the given RString into a proper Grade.
 * @param s the string to convert.
 * @return the expected Grade.
 */
Grade StringToGrade( const RString &s );
LuaDeclareType( Grade );
extern ThemeMetric<int> NUM_GRADE_TIERS_USED;
#define NUM_POSSIBLE_GRADES	(NUM_GRADE_TIERS_USED+1)
/**
 * @brief Step through the enumerator one at a time to get the next Grade.
 * @param g the current Grade.
 * @return the next Grade. */
Grade GetNextPossibleGrade( Grade g );
/** @brief Loop through each possible Grade. */
#define FOREACH_PossibleGrade( g ) \
for( Grade g = (Grade)(0); g != Grade_Invalid; g = GetNextPossibleGrade(g) )

#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2004
 * @section LICENSE
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
