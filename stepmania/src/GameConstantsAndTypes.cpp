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
#include "GameState.h"
#include "RageUtil.h"


CString RadarCategoryToString( RadarCategory cat )
{
	switch( cat )
	{
	case RADAR_STREAM:	return "stream";
	case RADAR_VOLTAGE:	return "voltage";
	case RADAR_AIR:		return "air";
	case RADAR_FREEZE:	return "freeze";
	case RADAR_CHAOS:	return "chaos";
	case RADAR_NUM_TAPS_AND_HOLDS: return "taps";
	case RADAR_NUM_JUMPS: return "jumps";
	case RADAR_NUM_HOLDS: return "holds";
	case RADAR_NUM_MINES: return "mines";
	case RADAR_NUM_HANDS: return "hands";
	default:	ASSERT(0);		return "";	// invalid
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
	case DIFFICULTY_EDIT:		return "edit";
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
	else if( sDC == "difficult")	return DIFFICULTY_MEDIUM;
	else if( sDC == "hard" )		return DIFFICULTY_HARD;
	else if( sDC == "ssr" )			return DIFFICULTY_HARD;
	else if( sDC == "maniac" )		return DIFFICULTY_HARD;
	else if( sDC == "heavy" )		return DIFFICULTY_HARD;
	else if( sDC == "smaniac" )		return DIFFICULTY_CHALLENGE;
	else if( sDC == "challenge" )	return DIFFICULTY_CHALLENGE;
	else if( sDC == "expert" )		return DIFFICULTY_CHALLENGE;
	else if( sDC == "oni" )			return DIFFICULTY_CHALLENGE;
	else if( sDC == "edit" )		return DIFFICULTY_EDIT;
	else							return DIFFICULTY_INVALID;
}


CString CourseDifficultyToString( CourseDifficulty dc )
{
	switch( dc )
	{
	case COURSE_DIFFICULTY_REGULAR:		return "regular";
	case COURSE_DIFFICULTY_DIFFICULT:	return "difficult";
	default:	ASSERT(0);		return "";	// invalid Difficulty
	}
}

/* We prefer the above names; recognize a number of others, too.  (They'l
 * get normalized when written to SMs, etc.) */
CourseDifficulty StringToCourseDifficulty( CString sDC )
{
	sDC.MakeLower();
	if( sDC == "regular" )			return COURSE_DIFFICULTY_REGULAR;
	else if( sDC == "difficult" )	return COURSE_DIFFICULTY_DIFFICULT;
	else							return COURSE_DIFFICULTY_INVALID;
}


CString PlayModeToString( PlayMode pm )
{
	switch( pm )
	{
	case PLAY_MODE_ARCADE:	return "arcade";
	case PLAY_MODE_ONI:		return "oni";
	case PLAY_MODE_NONSTOP:	return "nonstop";
	case PLAY_MODE_ENDLESS:	return "endless";
	case PLAY_MODE_BATTLE:	return "battle";
	case PLAY_MODE_RAVE:	return "rave";
	default:	ASSERT(0);	return "";
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

CString RankingCategoryToString( RankingCategory rc )
{
	switch( rc )
	{
	case RANKING_A: return "A";
	case RANKING_B: return "B";
	case RANKING_C: return "C";
	case RANKING_D: return "D";
	default: FAIL_M( ssprintf("%i", rc) );
	}
}

RankingCategory StringToRankingCategory( CString rc )
{
	if( rc == "A" ) return RANKING_A;
	if( rc == "B" ) return RANKING_B;
	if( rc == "C" ) return RANKING_C;
	if( rc == "D" ) return RANKING_D;
	return RANKING_INVALID;
}

CString CoinModeToString( CoinMode cm )
{
	switch( cm )
	{
		case COIN_HOME:		return "home";
		case COIN_PAY:		return "pay";
		case COIN_FREE:		return "free";
		default:	ASSERT(0);	return "";
	}
}

static const char *SongSortOrderNames[NUM_SORT_ORDERS] = {
	"PREFERRED",
	"GROUP",
	"TITLE",
	"BPM",
	"PLAYERS BEST",
	"TOP GRADE",
	"ARTIST",
	"EASY METER",
	"MEDIUM METER",
	"HARD METER",
	"CHALLENGE METER",
	"SORT",
	"MODE",
	"COURSES",
	"NONSTOP",
	"ONI",
	"ENDLESS",
	"ROULETTE"
};

CString SongSortOrderToString( SongSortOrder so )
{
	ASSERT(so < NUM_SORT_ORDERS);
	return SongSortOrderNames[so];
}

SongSortOrder StringToSongSortOrder( CString str )
{
	for( unsigned i = 0; i < NUM_SORT_ORDERS; ++i)
		if( !str.CompareNoCase(SongSortOrderNames[i]) )
			return (SongSortOrder)i;

	return SORT_INVALID;
}


CString TapNoteScoreToString( TapNoteScore tns )
{
	switch( tns )
	{
	case TNS_NONE:			return "None";
	case TNS_HIT_MINE:		return "HitMine";
	case TNS_MISS:			return "Miss";
	case TNS_BOO:			return "Boo";
	case TNS_GOOD:			return "Good";
	case TNS_GREAT:			return "Great";
	case TNS_PERFECT:		return "Perfect";
	case TNS_MARVELOUS:		return "Marvelous";
	default:	ASSERT(0);	return "";	// invalid
	}
}

CString MemoryCardStateToString( MemoryCardState mcs )
{
	switch( mcs )
	{
	case MEMORY_CARD_STATE_READY:		return "ready";
	case MEMORY_CARD_STATE_TOO_LATE:	return "late";
	case MEMORY_CARD_STATE_WRITE_ERROR:	return "error";
	case MEMORY_CARD_STATE_NO_CARD:		return "none";
	default:		ASSERT(0);			return "";
	}
}

