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
#include "ThemeManager.h"
#include "EnumHelper.h"


static const CString RadarCategoryNames[NUM_RADAR_CATEGORIES] = {
	"Stream",
	"Voltage",
	"Air",
	"Freeze",
	"Chaos",
	"Taps",
	"Jumps",
	"Holds",
	"Mines",
	"Hands",
};
XToString( RadarCategory );


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


static const CString CourseDifficultyNames[NUM_COURSE_DIFFICULTIES] = {
	"Regular",
	"Difficult",
};
XToString( CourseDifficulty );
XToThemedString( CourseDifficulty );
StringToX( CourseDifficulty );


static const CString PlayModeNames[NUM_PLAY_MODES] = {
	"Arcade",
	"Nonstop",
	"Oni",
	"Endless",
	"Battle",
	"Rave",
};
XToString( PlayMode );
StringToX( PlayMode );


RankingCategory AverageMeterToRankingCategory( float fAverageMeter )
{
	if(      fAverageMeter <= 3 )	return RANKING_A;
	else if( fAverageMeter <= 6 )	return RANKING_B;
	else if( fAverageMeter <= 9 )	return RANKING_C;
	else							return RANKING_D;
}


static const CString RankingCategoryNames[NUM_RANKING_CATEGORIES] = {
	"a",
	"b",
	"c",
	"d",
};
XToString( RankingCategory );
StringToX( RankingCategory );


static const CString CoinModeNames[NUM_COIN_MODES] = {
	"home",
	"pay",
	"free",
};
XToString( CoinMode );


static const CString SortOrderNames[NUM_SORT_ORDERS] = {
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
XToString( SortOrder );
StringToX( SortOrder );


static const CString TapNoteScoreNames[NUM_TAP_NOTE_SCORES] = {
	"None",
	"HitMine",
	"Miss",
	"Boo",
	"Good",
	"Great",
	"Perfect",
	"Marvelous",
};
XToString( TapNoteScore );


static const CString MemoryCardStateNames[NUM_MEMORY_CARD_STATES] = {
	"ready",
	"late",
	"error",
	"none",
};
XToString( MemoryCardState );


static const CString PerDifficultyAwardNames[NUM_PER_DIFFICULTY_AWARDS] = {
	"FullComboGreats",
	"FullComboPerfects",
	"FullComboMarvelouses",
	"SingleDigitGreats",
	"SingleDigitPerfects",
	"OneGreat",
	"OnePerfect",
	"Greats80Percent",
	"Greats90Percent",
	"Greats100Percent",
};
XToString( PerDifficultyAward );
XToThemedString( PerDifficultyAward );
StringToX( PerDifficultyAward );


// Numbers are intentially not at the front of these strings so that the 
// strings can be used as XML entity names.
// Numbers are intentially not at the back so that "1000" and "10000" don't 
// conflict when searching for theme elements.
static const CString PeakComboAwardNames[NUM_PEAK_COMBO_AWARDS] = {
	"Peak1000Combo",
	"Peak2000Combo",
	"Peak3000Combo",
	"Peak4000Combo",
	"Peak5000Combo",
	"Peak6000Combo",
	"Peak7000Combo",
	"Peak8000Combo",
	"Peak9000Combo",
	"Peak10000Combo",
};
XToString( PeakComboAward );
XToThemedString( PeakComboAward );
StringToX( PeakComboAward );
