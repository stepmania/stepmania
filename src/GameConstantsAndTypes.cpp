#include "global.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"
#include "RageUtil.h"
#include "ThemeMetric.h"
#include "EnumHelper.h"

#include "LuaManager.h"
#include "GameManager.h"
#include "LocalizedString.h"
#include "PlayerNumber.h"
#include <float.h>

RString StepsTypeToString( StepsType st );

static vector<RString> GenerateRankingToFillInMarker()
{
	vector<RString> vRankings;
	FOREACH_ENUM( PlayerNumber, pn )
		vRankings.push_back( ssprintf("#P%d#", pn+1) );
	return vRankings;
}
extern const vector<RString> RANKING_TO_FILL_IN_MARKER( GenerateRankingToFillInMarker() );

extern const RString GROUP_ALL = "---Group All---";

static const char *RadarCategoryNames[] = {
	"Stream",
	"Voltage",
	"Air",
	"Freeze",
	"Chaos",
	"Notes",
	"TapsAndHolds",
	"Jumps",
	"Holds",
	"Mines",
	"Hands",
	"Rolls",
	"Lifts",
	"Fakes",
};
XToString( RadarCategory );
XToLocalizedString( RadarCategory );
LuaFunction( RadarCategoryToLocalizedString, RadarCategoryToLocalizedString(Enum::Check<RadarCategory>(L, 1)) );
LuaXType( RadarCategory );

RString StepsTypeToString( StepsType st )
{
	RString s = GAMEMAN->GetStepsTypeInfo( st ).szName; // "dance-single"
	/* foo-bar -> Foo_Bar */
	s.Replace('-','_');

	bool bCapitalizeNextLetter = true;
	for( int i=0; i<(int)s.length(); i++ )
	{
		if( bCapitalizeNextLetter )
		{
			s[i] = toupper(s[i]);
			bCapitalizeNextLetter = false;
		}

		if( s[i] == '_' )
			bCapitalizeNextLetter = true;
	}

	return s;
}
namespace StringConversion { template<> RString ToString<StepsType>( const StepsType &value ) { return StepsTypeToString(value); } }

LuaXType( StepsType );


static const char *PlayModeNames[] = {
	"Regular",
	"Nonstop",
	"Oni",
	"Endless",
	"Battle",
	"Rave",
};
XToString( PlayMode );
XToLocalizedString( PlayMode );
StringToX( PlayMode );
LuaFunction( PlayModeToLocalizedString, PlayModeToLocalizedString(Enum::Check<PlayMode>(L, 1)) );
LuaXType( PlayMode );

RankingCategory AverageMeterToRankingCategory( int iAverageMeter )
{
	if(      iAverageMeter <= 3 )	return RANKING_A;
	else if( iAverageMeter <= 6 )	return RANKING_B;
	else if( iAverageMeter <= 9 )	return RANKING_C;
	else							return RANKING_D;
}


static const char *RankingCategoryNames[] = {
	"a",
	"b",
	"c",
	"d",
};
XToString( RankingCategory );
StringToX( RankingCategory );
LuaXType( RankingCategory );


static const char *PlayerControllerNames[] = {
	"Human",
	"Autoplay",
	"Cpu",
	//"Replay",
};
XToString( PlayerController );
StringToX( PlayerController );
XToLocalizedString( PlayerController );
LuaXType( PlayerController );


static const char *HealthStateNames[] = {
	"Hot",
	"Alive",
	"Danger",
	"Dead",
};
XToString( HealthState );
LuaXType( HealthState );

static const char *StageResultNames[] = {
	"Win",
	"Lose",
	"Draw",
};
XToString( StageResult );
LuaXType( StageResult );

static const char *CoinModeNames[] = {
	"Home",
	"Pay",
	"Free",
};
XToString( CoinMode );
StringToX( CoinMode );
LuaXType( CoinMode );


static const char *PremiumNames[] = {
	"Off",
	"DoubleFor1Credit",
	"2PlayersFor1Credit",
};
XToString( Premium );
StringToX( Premium );
XToLocalizedString( Premium );
LuaXType( Premium );


static const char *SortOrderNames[] = {
	"Preferred",
	"Group",
	"Title",
	"BPM",
	"Popularity",
	"TopGrades",
	"Artist",
	"Genre",
	"BeginnerMeter",
	"EasyMeter",
	"MediumMeter",
	"HardMeter",
	"ChallengeMeter",
	"DoubleEasyMeter",
	"DoubleMediumMeter",
	"DoubleHardMeter",
	"DoubleChallengeMeter",
	"ModeMenu",
	"AllCourses",
	"Nonstop",
	"Oni",
	"Endless",
	"Length",
	"Roulette",
	"Recent",
};
XToString( SortOrder );
StringToX( SortOrder );
LuaXType( SortOrder );
XToLocalizedString( SortOrder );
LuaFunction( SortOrderToLocalizedString, SortOrderToLocalizedString(Enum::Check<SortOrder>(L, 1)) );


static const char *TapNoteScoreNames[] = {
	"None",
	"HitMine",
	"AvoidMine",
	"CheckpointMiss",
	"Miss",
	"W5",
	"W4",
	"W3",
	"W2",
	"W1",
	"CheckpointHit",
};
struct tns_conversion_helper
{
	std::map<RString, TapNoteScore> conversion_map;
	tns_conversion_helper()
	{
		FOREACH_ENUM(TapNoteScore, tns)
		{
			conversion_map[TapNoteScoreNames[tns]]= tns;
		}
		// for backward compatibility
		conversion_map["Boo"]= TNS_W5;
		conversion_map["Good"]= TNS_W4;
		conversion_map["Great"]= TNS_W3;
		conversion_map["Perfect"]= TNS_W2;
		conversion_map["Marvelous"]= TNS_W1;
	}
};
tns_conversion_helper tns_converter;
XToString( TapNoteScore );
LuaXType( TapNoteScore );
TapNoteScore StringToTapNoteScore( const RString &s )
{
	std::map<RString, TapNoteScore>::iterator tns=
		tns_converter.conversion_map.find(s);
	if(tns != tns_converter.conversion_map.end())
	{
		return tns->second;
	}
	return TapNoteScore_Invalid;
}
// This is necessary because the StringToX macro wasn't used, and Preference
// relies on there being a StringConversion entry for enums used in prefs. -Kyz
namespace StringConversion
{
	template<> bool FromString<TapNoteScore>(const RString& value, TapNoteScore& out)
	{
		out= StringToTapNoteScore(value);
		return out != TapNoteScore_Invalid;
	}
}
XToLocalizedString( TapNoteScore );
LuaFunction( TapNoteScoreToLocalizedString, TapNoteScoreToLocalizedString(Enum::Check<TapNoteScore>(L, 1)) );


static const char *HoldNoteScoreNames[] = {
	"None",
	"LetGo",
	"Held",
	"MissedHold",
};
XToString( HoldNoteScore );
LuaXType( HoldNoteScore );
HoldNoteScore StringToHoldNoteScore( const RString &s )
{
	// for backward compatibility
	if     ( s == "NG" )		return HNS_LetGo;
	else if( s == "OK" )		return HNS_Held;

	// new style
	else if( s == "None" )		return HNS_None;
	else if( s == "LetGo" )		return HNS_LetGo;
	else if( s == "Held" )		return HNS_Held;
	else if( s == "MissedHold" )	return HNS_Missed;

	return HoldNoteScore_Invalid;
}
XToLocalizedString( HoldNoteScore );

static const char *TimingWindowNames[] = {
	"W1",
	"W2",
	"W3",
	"W4",
	"W5",
	"Mine",
	"Attack",
	"Hold",
	"Roll",
	"Checkpoint"
};
XToString( TimingWindow );

static const char *ScoreEventNames[] = {
	"CheckpointHit",
	"W1",
	"W2",
	"W3",
	"W4",
	"W5",
	"Miss",
	"HitMine",
	"CheckpointMiss",
	"Held",
	"LetGo",
	"MissedHold",
};
XToString( ScoreEvent );

static const char *TapNoteScoreJudgeTypeNames[] = {
	"MinimumScore",
	"LastScore",
};
XToString( TapNoteScoreJudgeType );
LuaXType( TapNoteScoreJudgeType );

static const char *ProfileSlotNames[] = {
	"Player1",
	"Player2",
	"Machine",
};
XToString( ProfileSlot );
LuaXType( ProfileSlot );

static const char *MemoryCardStateNames[] = {
	"ready",
	"checking",
	"late",
	"error",
	"removed",
	"none",
};
XToString( MemoryCardState );
LuaXType( MemoryCardState );

static const char *StageAwardNames[] = {
	"FullComboW3",
	"SingleDigitW3",
	"OneW3",
	"FullComboW2",
	"SingleDigitW2",
	"OneW2",
	"FullComboW1",
	"80PercentW3",
	"90PercentW3",
	"100PercentW3",
};
XToString( StageAward );
XToLocalizedString( StageAward );
StringToX( StageAward );
LuaFunction( StageAwardToLocalizedString, StageAwardToLocalizedString(Enum::Check<StageAward>(L, 1)) );
LuaXType( StageAward );

// Numbers are intentionally not at the front of these strings so that the 
// strings can be used as XML entity names.
// Numbers are intentionally not at the back so that "1000" and "10000" don't 
// conflict when searching for theme elements.
static const char *PeakComboAwardNames[] = {
	"1000",
	"2000",
	"3000",
	"4000",
	"5000",
	"6000",
	"7000",
	"8000",
	"9000",
	"10000",
};
XToString( PeakComboAward );
XToLocalizedString( PeakComboAward );
StringToX( PeakComboAward );
LuaFunction( PeakComboAwardToLocalizedString, PeakComboAwardToLocalizedString(Enum::Check<PeakComboAward>(L, 1)) );
LuaXType( PeakComboAward );


void DisplayBpms::Add( float f )
{
	vfBpms.push_back( f );
}

float DisplayBpms::GetMin() const
{
	float fMin = FLT_MAX;
	for (float const &f : vfBpms)
	{
		if( f != -1 )
			fMin = min( fMin, f );
	}
	if( fMin == FLT_MAX )
		return 0;
	else
		return fMin;
}

float DisplayBpms::GetMax() const
{
	return this->GetMaxWithin();
}

float DisplayBpms::GetMaxWithin(float highest) const
{
	float fMax = 0;
	for (float const &f : vfBpms)
	{
		if( f != -1 )
			fMax = clamp(max( fMax, f ), 0, highest);
	}
	return fMax;
}

bool DisplayBpms::BpmIsConstant() const
{
	return fabsf( GetMin() - GetMax() ) < 0.001f;
}

bool DisplayBpms::IsSecret() const
{
	return std::any_of(vfBpms.begin(), vfBpms.end(), [](float const &f) { return f == -1; });
}

static const char *StyleTypeNames[] = {
	"OnePlayerOneSide",
	"TwoPlayersTwoSides",
	"OnePlayerTwoSides",
	"TwoPlayersSharedSides",
};
XToString( StyleType );
StringToX( StyleType );
LuaXType( StyleType );

static const char *GoalTypeNames[] = {
	"Calories",
	"Time",
	"None",
};
XToString( GoalType );
StringToX( GoalType );
LuaXType( GoalType );

static const char *EditModeNames[] = {
	"Practice",
	"CourseMods",
	"Home",
	"Full"
};
XToString( EditMode );
StringToX( EditMode );
LuaXType( EditMode );

static const char *SampleMusicPreviewModeNames[] = {
	"Normal",
	"StartToPreview",
	"ScreenMusic",
	"LastSong"
};
XToString( SampleMusicPreviewMode );
StringToX( SampleMusicPreviewMode );
LuaXType( SampleMusicPreviewMode );

static const char *StageNames[] = {
	"1st",
	"2nd",
	"3rd",
	"4th",
	"5th",
	"6th",
	"Next",
	"Final",
	"Extra1",
	"Extra2",
	"Nonstop",
	"Oni",
	"Endless",
	"Event",
	"Demo",
};
XToString( Stage );
LuaXType( Stage );
XToLocalizedString( Stage );
LuaFunction( StageToLocalizedString, StageToLocalizedString(Enum::Check<Stage>(L, 1)) );

static const char *EarnedExtraStageNames[] = {
	"No",
	"Extra1",
	"Extra2",
};
XToString( EarnedExtraStage );
LuaXType( EarnedExtraStage );


static const char *MultiPlayerStatusNames[] = {
	"Joined",
	"NotJoined",
	"Unplugged",
	"MissingMultitap",
};
XToString( MultiPlayerStatus );


static const char *CourseTypeNames[] = {
	"Nonstop",
	"Oni",
	"Endless",
	"Survival",
};
XToString( CourseType );
XToLocalizedString( CourseType );
LuaXType( CourseType );
LuaFunction( CourseTypeToLocalizedString, CourseTypeToLocalizedString( Enum::Check<CourseType>( L, 1 ) ) );

static const char *FailTypeNames[] = {
	"Immediate",
	"ImmediateContinue",
	"EndOfSong",
	"Off",
};
XToString( FailType );
XToLocalizedString( FailType );
StringToX( FailType );
LuaXType( FailType );

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
