#include "global.h"
/*
-----------------------------------------------------------------------------
 File: GameConstantsAndTypes.cpp

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"


int TapNoteScoreToDancePoints( TapNoteScore tns, bool bOni )
{
	switch( tns )
	{
	case TNS_MARVELOUS:	return bOni ? +3 : +2;
	case TNS_PERFECT:	return +2;
	case TNS_GREAT:		return +1;
	case TNS_GOOD:		return +0;
	case TNS_BOO:		return bOni ? 0 : -4;
	case TNS_MISS:		return bOni ? 0 : -8;
	case TNS_NONE:		return 0;
	default:	ASSERT(0);	return 0;
	}
}

CString DifficultyToString( Difficulty dc )
{
	switch( dc )
	{
	case DIFFICULTY_BEGINNER:	return "beginner";
	case DIFFICULTY_EASY:		return "easy";
	case DIFFICULTY_MEDIUM:		return "medium";
	case DIFFICULTY_HARD:		return "hard";
	case DIFFICULTY_CHALLENGE:	return "challenge";
	default:	ASSERT(0);		return "";	// invalid Difficulty
	}
}

/* We prefer the above names; recognize a number of others, too.  (They'l
 * get normalized when written to SMs, etc.) */
Difficulty StringToDifficulty( CString sDC )
{
	sDC.MakeLower();
	if( sDC == "beginner" )			return DIFFICULTY_BEGINNER;
	else if( sDC == "easy" )		return DIFFICULTY_EASY;
	else if( sDC == "basic" )		return DIFFICULTY_EASY;
	else if( sDC == "light" )		return DIFFICULTY_EASY;
	else if( sDC == "medium" )		return DIFFICULTY_MEDIUM;
	else if( sDC == "another" )		return DIFFICULTY_MEDIUM;
	else if( sDC == "trick" )		return DIFFICULTY_MEDIUM;
	else if( sDC == "standard" )	return DIFFICULTY_MEDIUM;
	else if( sDC == "hard" )		return DIFFICULTY_HARD;
	else if( sDC == "ssr" )			return DIFFICULTY_HARD;
	else if( sDC == "maniac" )		return DIFFICULTY_HARD;
	else if( sDC == "heavy" )		return DIFFICULTY_HARD;
	else if( sDC == "smaniac" )		return DIFFICULTY_CHALLENGE;
	else if( sDC == "challenge" )	return DIFFICULTY_CHALLENGE;
	else							return DIFFICULTY_INVALID;
}


CString PlayModeToString( PlayMode pm )
{
	switch( pm )
	{
	case PLAY_MODE_ARCADE:		return "arcade";
	case PLAY_MODE_ONI:			return "oni";
	case PLAY_MODE_NONSTOP:		return "nonstop";
	case PLAY_MODE_ENDLESS:		return "endless";
	case PLAY_MODE_BATTLE:		return "battle";
	default:	ASSERT(0);		return "";
	}
}

PlayMode StringToPlayMode( CString s )
{
	for( int i=0; i<NUM_PLAY_MODES; i++ )
		if( PlayModeToString((PlayMode)i).CompareNoCase(s) == 0 )
			return (PlayMode)i;
	return PLAY_MODE_INVALID;
}



RankingCategory AverageMeterToRankingCategory( float fAverageMeter )
{
	if(      fAverageMeter <= 3 )	return RANKING_A;
	else if( fAverageMeter <= 6 )	return RANKING_B;
	else if( fAverageMeter <= 9 )	return RANKING_C;
	else							return RANKING_D;

}

