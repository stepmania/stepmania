#include "global.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"
#include "RageUtil.h"
#include "ThemeMetric.h"
#include "EnumHelper.h"
#include "Foreach.h"
#include "PrefsManager.h"
#include "LuaManager.h"
#include "GameManager.h"
#include <float.h>

#include "LuaFunctions.h"

const CString RANKING_TO_FILL_IN_MARKER[NUM_PLAYERS] = {"#P1#","#P2#"};

extern const CString GROUP_ALL = "---Group All---";

static const CString RadarCategoryNames[] = {
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
	"Rolls"
};
XToString( RadarCategory, NUM_RADAR_CATEGORIES );
XToThemedString( RadarCategory, NUM_RADAR_CATEGORIES );


static void LuaStepsType(lua_State* L)
{
	FOREACH_StepsType( st )
	{
		CString s = GAMEMAN->StepsTypeToString( st );
		s.MakeUpper();
		s.Replace('-','_');
		LUA->SetGlobal( "STEPS_TYPE_"+s, st );
	}
	LUA->SetGlobal( "STEPS_TYPE_INVALID", STEPS_TYPE_INVALID );
}
REGISTER_WITH_LUA_FUNCTION( LuaStepsType );


static const CString PlayModeNames[] = {
	"Regular",
	"Nonstop",
	"Oni",
	"Endless",
	"Battle",
	"Rave",
};
XToString( PlayMode, NUM_PLAY_MODES );
XToThemedString( PlayMode, NUM_PLAY_MODES );
StringToX( PlayMode );
LuaXToString( PlayMode );
LuaXType( PlayMode, NUM_PLAY_MODES, "PLAY_MODE_" )

RankingCategory AverageMeterToRankingCategory( int iAverageMeter )
{
	if(      iAverageMeter <= 3 )	return RANKING_A;
	else if( iAverageMeter <= 6 )	return RANKING_B;
	else if( iAverageMeter <= 9 )	return RANKING_C;
	else							return RANKING_D;
}


static const CString RankingCategoryNames[] = {
	"a",
	"b",
	"c",
	"d",
};
XToString( RankingCategory, NUM_RANKING_CATEGORIES );
StringToX( RankingCategory );


static const CString PlayerControllerNames[] = {
	"Human",
	"Autoplay",
	"Cpu",
};
XToString( PlayerController, NUM_PLAYER_CONTROLLERS );


static const CString CoinModeNames[] = {
	"home",
	"pay",
	"free",
};
XToString( CoinMode, NUM_COIN_MODES );
static void LuaCoinMode(lua_State* L)
{
	FOREACH_CoinMode( i )
	{
		CString s = CoinModeNames[i];
		s.MakeUpper();
		LUA->SetGlobal( "COIN_MODE_"+s, i );
	}
}
REGISTER_WITH_LUA_FUNCTION( LuaCoinMode );


static const CString PremiumNames[] = {
	"none",
	"double",
	"joint",
};
XToString( Premium, NUM_PREMIUMS );
static void LuaPremium(lua_State* L)
{
	FOREACH_Premium( i )
	{
		CString s = PremiumNames[i];
		s.MakeUpper();
		LUA->SetGlobal( "PREMIUM_"+s, i );
	}
}
REGISTER_WITH_LUA_FUNCTION( LuaPremium );


static const CString SortOrderNames[] = {
	"Preferred",
	"Group",
	"Title",
	"BPM",
	"Popularity",
	"TopGrades",
	"Artist",
	"Genre",
	"EasyMeter",
	"MediumMeter",
	"HardMeter",
	"ChallengeMeter",
	"ModeMenu",
	"AllCourses",
	"Nonstop",
	"Oni",
	"Endless",
	"Roulette",
};
XToString( SortOrder, NUM_SORT_ORDERS );
StringToX( SortOrder );

static void LuaSortOrder(lua_State* L)
{
	FOREACH_SortOrder( so )
	{
		CString s = SortOrderToString( so );
		
		// [uppercase] -> _[uppercase]
		for( unsigned i=0; i<s.size(); i++ )
		{
			if( isupper(s[i]) )
			{
				s.insert( s.begin()+i, '_' );
				i++;
			}
		}

		s.MakeUpper();
		LUA->SetGlobal( "SORT"+s, so );
	}
}
REGISTER_WITH_LUA_FUNCTION( LuaSortOrder );


static const CString TapNoteScoreNames[] = {
	"None",
	"HitMine",
	"AvoidMine",
	"Miss",
	"W5",
	"W4",
	"W3",
	"W2",
	"W1",
};
XToString( TapNoteScore, NUM_TapNoteScore );
TapNoteScore StringToTapNoteScore( const CString &s )
{
	// new style
	if	   ( s == "None" )		return TNS_None;
	else if( s == "HitMine" )	return TNS_HitMine;
	else if( s == "AvoidMine" )	return TNS_AvoidMine;
	else if( s == "Miss" )		return TNS_Miss;
	else if( s == "W5" )		return TNS_W5;
	else if( s == "W4" )		return TNS_W4;
	else if( s == "W3" )		return TNS_W3;
	else if( s == "W2" )		return TNS_W2;
	else if( s == "W1" )		return TNS_W1;

	// for backward compatibility
	else if( s == "Boo" )		return TNS_W5;
	else if( s == "Good" )		return TNS_W4;
	else if( s == "Great" )		return TNS_W3;
	else if( s == "Perfect" )	return TNS_W2;
	else if( s == "Marvelous" )	return TNS_W1;

	return TNS_INVALID;
}
XToThemedString( TapNoteScore, NUM_TapNoteScore );
static void LuaTapNoteScores( lua_State* L )
{
	FOREACH_TapNoteScore( i )
	{
		CString s = TapNoteScoreNames[i];
		s.MakeUpper();
		LUA->SetGlobal( "TNS_"+s, i );
	}
}
REGISTER_WITH_LUA_FUNCTION( LuaTapNoteScores );


static const CString HoldNoteScoreNames[] = {
	"None",
	"LetGo",
	"Held",
};
XToString( HoldNoteScore, NUM_HoldNoteScore );
HoldNoteScore StringToHoldNoteScore( const CString &s )
{
	// for backward compatibility
	if	   ( s == "NG" )	return HNS_LetGo;
	else if( s == "OK" )	return HNS_Held;

	// new style
	else if( s == "None" )	return HNS_None;
	else if( s == "LetGo" )	return HNS_LetGo;
	else if( s == "Held" )	return HNS_Held;

	return HNS_INVALID;
}
XToThemedString( HoldNoteScore, NUM_HoldNoteScore );
static void LuaHoldNoteScores( lua_State* L )
{
	FOREACH_HoldNoteScore( i )
	{
		CString s = HoldNoteScoreNames[i];
		s.MakeUpper();
		LUA->SetGlobal( "HNS_"+s, i );
	}
}
REGISTER_WITH_LUA_FUNCTION( LuaHoldNoteScores );


static const CString TimingWindowNames[] = {
	"W1",
	"W2",
	"W3",
	"W4",
	"W5",
	"Mine",
	"Attack",
	"Hold",
	"Roll",
};
XToString( TimingWindow, NUM_TimingWindow );


static const CString ScoreEventNames[] = {
	"W1",
	"W2",
	"W3",
	"W4",
	"W5",
	"Miss",
	"HitMine",
	"Held",
	"LetGo",
};
XToString( ScoreEvent, NUM_ScoreEvent );


static const CString MemoryCardStateNames[] = {
	"ready",
	"checking",
	"late",
	"error",
	"removed",
	"none",
};
XToString( MemoryCardState, NUM_MEMORY_CARD_STATES );


static const CString PerDifficultyAwardNames[] = {
	"FullComboW3",
	"SingleDigitW3",
	"OneW3",
	"FullComboW2",
	"SingleDigitW2",
	"OneW2",
	"FullComboW1",
	"Percent80W3",
	"Percent90W3",
	"Percent100W3",
};
XToString( PerDifficultyAward, NUM_PER_DIFFICULTY_AWARDS );
XToThemedString( PerDifficultyAward, NUM_PER_DIFFICULTY_AWARDS );
StringToX( PerDifficultyAward );
static void LuaPerDifficultyAward( lua_State* L )
{
	FOREACH_PerDifficultyAward( i )
		LUA->SetGlobal( PerDifficultyAwardNames[i], i );
}
REGISTER_WITH_LUA_FUNCTION( LuaPerDifficultyAward );

// Numbers are intentially not at the front of these strings so that the 
// strings can be used as XML entity names.
// Numbers are intentially not at the back so that "1000" and "10000" don't 
// conflict when searching for theme elements.
static const CString PeakComboAwardNames[] = {
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
XToString( PeakComboAward, NUM_PEAK_COMBO_AWARDS );
XToThemedString( PeakComboAward, NUM_PEAK_COMBO_AWARDS );
StringToX( PeakComboAward );
static void LuaPeakComboAward( lua_State* L )
{
	FOREACH_PeakComboAward( i )
		LUA->SetGlobal( PeakComboAwardNames[i], i );
}
REGISTER_WITH_LUA_FUNCTION( LuaPeakComboAward );

LuaXToString( Difficulty );
LuaStringToX( Difficulty );


void DisplayBpms::Add( float f )
{
	vfBpms.push_back( f );
}

float DisplayBpms::GetMin() const
{
	float fMin = FLT_MAX;
	FOREACH_CONST( float, vfBpms, f )
	{
		if( *f != -1 )
			fMin = min( fMin, *f );
	}
	if( fMin == FLT_MAX )
		return 0;
	else
		return fMin;
}

float DisplayBpms::GetMax() const
{
	float fMax = 0;
	FOREACH_CONST( float, vfBpms, f )
	{
		if( *f != -1 )
			fMax = max( fMax, *f );
	}
	return fMax;
}

bool DisplayBpms::BpmIsConstant() const
{
	return fabsf( GetMin() - GetMax() ) < 0.001f;
}

bool DisplayBpms::IsSecret() const
{
	FOREACH_CONST( float, vfBpms, f )
	{
		if( *f == -1 )
			return true;
	}
	return false;
}

static const CString StyleTypeNames[] = {
	"OnePlayerOneSide",
	"TwoPlayersTwoSides",
	"OnePlayerTwoSides",
};
XToString( StyleType, NUM_STYLE_TYPES );
StringToX( StyleType );


static const CString MenuDirNames[] = {
	"Up",
	"Down",
	"Left",
	"Right",
	"Auto",
};
XToString( MenuDir, NUM_MENU_DIRS );


static const CString GoalTypeNames[] = {
	"Calories",
	"Time",
	"None",
};
XToString( GoalType, NUM_GOAL_TYPES );
StringToX( GoalType );
static void LuaGoalType(lua_State* L)
{
	FOREACH_GoalType( gt )
	{
		CString s = GoalTypeNames[gt];
		s.MakeUpper();
		LUA->SetGlobal( "GOAL_"+s, gt );
	}
}
REGISTER_WITH_LUA_FUNCTION( LuaGoalType );


static const CString StageNames[] = {
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"Final",
	"Extra1",
	"Extra2",
	"Nonstop",
	"Oni",
	"Endless",
	"Event",
	"Demo",
};
XToString( Stage, NUM_STAGES );
static void LuaStage(lua_State* L)
{
	FOREACH_Stage( st )
	{
		CString s = StageNames[st];
		s.MakeUpper();
		LUA->SetGlobal( "STAGE_"+s, st );
	}
}
REGISTER_WITH_LUA_FUNCTION( LuaStage );
LuaXToString( Stage );


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
