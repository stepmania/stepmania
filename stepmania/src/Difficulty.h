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
