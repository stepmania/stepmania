#include "global.h"
#include "Difficulty.h"
#include "GameState.h"
#include "ThemeManager.h"


static const CString DifficultyNames[NUM_RADAR_CATEGORIES] = {
	"Beginner",
	"Easy",
	"Medium",
	"Hard",
	"Challenge",
	"Edit",
};
XToString( Difficulty );
XToThemedString( Difficulty );

/* We prefer the above names; recognize a number of others, too.  (They'l
 * get normalized when written to SMs, etc.) */
Difficulty StringToDifficulty( const CString& sDC )
{
	CString s2 = sDC;
	s2.MakeLower();
	if( s2 == "beginner" )		return DIFFICULTY_BEGINNER;
	else if( s2 == "easy" )		return DIFFICULTY_EASY;
	else if( s2 == "basic" )	return DIFFICULTY_EASY;
	else if( s2 == "light" )	return DIFFICULTY_EASY;
	else if( s2 == "medium" )	return DIFFICULTY_MEDIUM;
	else if( s2 == "another" )	return DIFFICULTY_MEDIUM;
	else if( s2 == "trick" )	return DIFFICULTY_MEDIUM;
	else if( s2 == "standard" )	return DIFFICULTY_MEDIUM;
	else if( s2 == "difficult")	return DIFFICULTY_MEDIUM;
	else if( s2 == "hard" )		return DIFFICULTY_HARD;
	else if( s2 == "ssr" )		return DIFFICULTY_HARD;
	else if( s2 == "maniac" )	return DIFFICULTY_HARD;
	else if( s2 == "heavy" )	return DIFFICULTY_HARD;
	else if( s2 == "smaniac" )	return DIFFICULTY_CHALLENGE;
	else if( s2 == "challenge" )return DIFFICULTY_CHALLENGE;
	else if( s2 == "expert" )	return DIFFICULTY_CHALLENGE;
	else if( s2 == "oni" )		return DIFFICULTY_CHALLENGE;
	else if( s2 == "edit" )		return DIFFICULTY_EDIT;
	else						return DIFFICULTY_INVALID;
}


static const CString CourseDifficultyNames[NUM_COURSE_DIFFICULTIES] =
{
	"Regular",
	"Difficult",
};
XToString( CourseDifficulty );
XToThemedString( CourseDifficulty );
StringToX( CourseDifficulty );

CourseDifficulty GetNextShownCourseDifficulty( CourseDifficulty cd )
{
	for( CourseDifficulty d=(CourseDifficulty)(cd+1); d<NUM_COURSE_DIFFICULTIES; ((int&)d)++ )
	{
		if( GAMESTATE->IsCourseDifficultyShown(d) )
			return d;
	}
	return COURSE_DIFFICULTY_INVALID;
}
