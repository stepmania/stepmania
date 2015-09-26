#ifndef DIFFICULTY_H
#define DIFFICULTY_H

#include "EnumHelper.h"
#include "GameConstantsAndTypes.h"
class Steps;
class Trail;

// Player number stuff
enum Difficulty 
{
	Difficulty_Beginner,
	Difficulty_Easy,
	Difficulty_Medium,
	Difficulty_Hard,
	Difficulty_Challenge,
	Difficulty_Edit,
	NUM_Difficulty,
	Difficulty_Invalid
};
const RString& DifficultyToString( Difficulty dc );
Difficulty StringToDifficulty( const RString& sDC );
LuaDeclareType( Difficulty );

Difficulty OldStyleStringToDifficulty( const RString& sDC ); // compatibility

typedef Difficulty CourseDifficulty;
const int NUM_CourseDifficulty = NUM_Difficulty;
/** @brief Loop through the shown course difficulties. */
#define FOREACH_ShownCourseDifficulty( cd ) \
for( Difficulty cd=GetNextShownCourseDifficulty((CourseDifficulty)-1); \
	cd!=Difficulty_Invalid; cd=GetNextShownCourseDifficulty(cd) )

const RString& CourseDifficultyToLocalizedString( Difficulty dc );

Difficulty GetNextShownCourseDifficulty( Difficulty pn );


// CustomDifficulty is a themeable difficulty name based on Difficulty, string matching on StepsType, and CourseType.
// It is used to look up localized strings and look up colors.
RString GetCustomDifficulty( StepsType st, Difficulty dc, CourseType ct );
RString CustomDifficultyToLocalizedString( const RString &sCustomDifficulty );
RString StepsToCustomDifficulty( const Steps *pSteps );
RString TrailToCustomDifficulty( const Trail *pTrail );


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
