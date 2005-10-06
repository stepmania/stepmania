#ifndef DIFFICULTY_H
#define DIFFICULTY_H

#include "EnumHelper.h"

//
// Player number stuff
//
enum Difficulty 
{
	DIFFICULTY_BEGINNER,
	DIFFICULTY_EASY,
	DIFFICULTY_MEDIUM,
	DIFFICULTY_HARD,
	DIFFICULTY_CHALLENGE,
	DIFFICULTY_EDIT,
	NUM_DIFFICULTIES,
	DIFFICULTY_INVALID
};
#define FOREACH_Difficulty( dc ) FOREACH_ENUM( Difficulty, NUM_DIFFICULTIES, dc )
const CString& DifficultyToString( Difficulty dc );
const CString& DifficultyToThemedString( Difficulty dc );
Difficulty StringToDifficulty( const CString& sDC );


typedef Difficulty CourseDifficulty;
#define NUM_COURSE_DIFFICULTIES NUM_DIFFICULTIES
#define FOREACH_CourseDifficulty FOREACH_Difficulty
#define FOREACH_ShownCourseDifficulty( cd ) for( Difficulty cd=GetNextShownCourseDifficulty((CourseDifficulty)-1); cd!=DIFFICULTY_INVALID; cd=GetNextShownCourseDifficulty(cd) )

const CString& CourseDifficultyToString( Difficulty dc );
const CString& CourseDifficultyToThemedString( Difficulty dc );
Difficulty StringToCourseDifficulty( const CString& sDC );

Difficulty GetNextShownCourseDifficulty( Difficulty pn );

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
