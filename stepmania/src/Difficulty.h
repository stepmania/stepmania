#ifndef DIFFICULTY_H
#define DIFFICULTY_H

#include "EnumHelper.h"

//
// Player number stuff
//
enum Difficulty 
{
	DIFFICULTY_BEGINNER,	// corresponds to DDREX Beginner
	DIFFICULTY_EASY,		// corresponds to Basic, Easy
	DIFFICULTY_MEDIUM,		// corresponds to Trick, Another, Standard, Normal
	DIFFICULTY_HARD,		// corresponds to Maniac, SSR, Heavy, Crazy
	DIFFICULTY_CHALLENGE,	// corresponds to 5th SMANIAC, MAX2 Challenge, EX Challenge
	DIFFICULTY_EDIT,
	NUM_DIFFICULTIES,
	DIFFICULTY_INVALID
};
#define FOREACH_Difficulty( dc ) FOREACH_ENUM( Difficulty, NUM_DIFFICULTIES, dc )
const CString& DifficultyToString( Difficulty dc );
CString DifficultyToThemedString( Difficulty dc );
Difficulty StringToDifficulty( const CString& sDC );


enum CourseDifficulty 
{
	COURSE_DIFFICULTY_EASY,
	COURSE_DIFFICULTY_REGULAR,
	COURSE_DIFFICULTY_DIFFICULT,
	NUM_COURSE_DIFFICULTIES,
	COURSE_DIFFICULTY_INVALID
};
#define FOREACH_CourseDifficulty( cd ) FOREACH_ENUM( CourseDifficulty, NUM_COURSE_DIFFICULTIES, cd )
#define FOREACH_ShownCourseDifficulty( cd ) for( CourseDifficulty cd=GetNextShownCourseDifficulty((CourseDifficulty)-1); cd!=COURSE_DIFFICULTY_INVALID; cd=GetNextShownCourseDifficulty(cd) )

const CString& CourseDifficultyToString( CourseDifficulty dc );
CString CourseDifficultyToThemedString( CourseDifficulty dc );
CourseDifficulty StringToCourseDifficulty( const CString& sDC );

CourseDifficulty GetNextShownCourseDifficulty( CourseDifficulty pn );

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
