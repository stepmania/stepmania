#include "global.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"
#include "RageUtil.h"
#include "ThemeManager.h"
#include "EnumHelper.h"
#include "Foreach.h"


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
XToThemedString( RadarCategory );


static const CString PlayModeNames[NUM_PLAY_MODES] = {
	"Regular",
	"Nonstop",
	"Oni",
	"Endless",
	"Battle",
	"Rave",
};
XToString( PlayMode );
XToThemedString( PlayMode );
StringToX( PlayMode );


RankingCategory AverageMeterToRankingCategory( int iAverageMeter )
{
	if(      iAverageMeter <= 3 )	return RANKING_A;
	else if( iAverageMeter <= 6 )	return RANKING_B;
	else if( iAverageMeter <= 9 )	return RANKING_C;
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
StringToX( TapNoteScore );
XToThemedString( TapNoteScore );


static const CString HoldNoteScoreNames[NUM_HOLD_NOTE_SCORES] = {
	"None",
	"NG",
	"OK",
};
XToString( HoldNoteScore );
StringToX( HoldNoteScore );
XToThemedString( HoldNoteScore );


static const CString MemoryCardStateNames[NUM_MEMORY_CARD_STATES] = {
	"ready",
	"late",
	"error",
	"none",
};
XToString( MemoryCardState );


static const CString PerDifficultyAwardNames[NUM_PER_DIFFICULTY_AWARDS] = {
	"FullComboGreats",
	"SingleDigitGreats",
	"OneGreat",
	"FullComboPerfects",
	"SingleDigitPerfects",
	"OnePerfect",
	"FullComboMarvelouses",
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


#include "LuaFunctions.h"
#define LuaXToString(X)	\
CString Lua##X##ToString( int n ) \
{ return X##ToString( (X) n ); } \
LuaFunction_Int( X##ToString, Lua##X##ToString(a1) ); /* register it */

#define LuaStringToX(X)	\
X LuaStringTo##X( CString s ) \
{ return (X) StringTo##X( s ); } \
LuaFunction_Str( StringTo##X, LuaStringTo##X(str) ); /* register it */

LuaXToString( Difficulty );
LuaStringToX( Difficulty );


void DisplayBpms::Add( float f )
{
	vfBpms.push_back( f );
}

float DisplayBpms::GetMin()
{
	float fMin = 99999;
	FOREACH_CONST( float, vfBpms, f )
	{
		if( *f != -1 )
			fMin = min( fMin, *f );
	}
	if( fMin == 99999 )
		return 0;
	else
		return fMin;
}

float DisplayBpms::GetMax()
{
	float fMax = 0;
	FOREACH_CONST( float, vfBpms, f )
	{
		if( *f != -1 )
			fMax = max( fMax, *f );
	}
	return fMax;
}

bool DisplayBpms::BpmIsConstant()
{
	return GetMin() == GetMax();
}

bool DisplayBpms::IsMystery()
{
	FOREACH_CONST( float, vfBpms, f )
	{
		if( *f == -1 )
			return true;
	}
	return false;
}

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
