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


CString CategoryToString( RadarCategory cat )
{
	switch( cat )
	{
	case RADAR_STREAM:	return "stream";
	case RADAR_VOLTAGE:	return "voltage";
	case RADAR_AIR:		return "air";
	case RADAR_FREEZE:	return "freeze";
	case RADAR_CHAOS:	return "chaos";
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
	case PLAY_MODE_ARCADE:			return "arcade";
	case PLAY_MODE_ONI:				return "oni";
	case PLAY_MODE_NONSTOP:			return "nonstop";
	case PLAY_MODE_ENDLESS:			return "endless";
	case PLAY_MODE_HUMAN_BATTLE:	return "humanbattle";
	case PLAY_MODE_CPU_BATTLE:		return "cpubattle";
	case PLAY_MODE_RAVE:			return "rave";
	default:	ASSERT(0);			return "";
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
	"SORT",
	"COURSES",
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

	ASSERT(0);
	return SORT_PREFERRED;
}
