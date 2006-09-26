/* GameConstantsAndTypes - Things used in many places that don't change often. */

#ifndef GAME_CONSTANTS_AND_TYPES_H
#define GAME_CONSTANTS_AND_TYPES_H

#include "PlayerNumber.h"	// TODO: Get rid of this dependency.  -Chris
#include "EnumHelper.h"


//
// Note definitions
//
const int MAX_METER = 13;
const int MIN_METER = 1;


/* This is just cached song data.  Not all of it may actually be displayed
 * in the radar. */
enum RadarCategory
{
	RadarCategory_Stream = 0,
	RadarCategory_Voltage,
	RadarCategory_Air,
	RadarCategory_Freeze,
	RadarCategory_Chaos,
	RadarCategory_TapsAndHolds,
	RadarCategory_Jumps,
	RadarCategory_Holds,
	RadarCategory_Mines,
	RadarCategory_Hands,
	RadarCategory_Rolls,
	NUM_RadarCategory	// leave this at the end
};
#define FOREACH_RadarCategory( rc ) FOREACH_ENUM( RadarCategory, NUM_RadarCategory, rc )
const RString& RadarCategoryToString( RadarCategory cat );
const RString& RadarCategoryToLocalizedString( RadarCategory cat );


enum StepsType
{
	STEPS_TYPE_DANCE_SINGLE = 0,
	STEPS_TYPE_DANCE_DOUBLE,
	STEPS_TYPE_DANCE_COUPLE,
	STEPS_TYPE_DANCE_SOLO,
	STEPS_TYPE_DANCE_ROUTINE,
	STEPS_TYPE_PUMP_SINGLE,
	STEPS_TYPE_PUMP_HALFDOUBLE,
	STEPS_TYPE_PUMP_DOUBLE,
	STEPS_TYPE_PUMP_COUPLE,
	STEPS_TYPE_EZ2_SINGLE,
	STEPS_TYPE_EZ2_DOUBLE,
	STEPS_TYPE_EZ2_REAL,
	STEPS_TYPE_PARA_SINGLE,
	STEPS_TYPE_PARA_VERSUS,
	STEPS_TYPE_DS3DDX_SINGLE,
	STEPS_TYPE_BEAT_SINGLE5,
	STEPS_TYPE_BEAT_DOUBLE5,
	STEPS_TYPE_BEAT_SINGLE7,
	STEPS_TYPE_BEAT_DOUBLE7,
	STEPS_TYPE_MANIAX_SINGLE,
	STEPS_TYPE_MANIAX_DOUBLE,
	STEPS_TYPE_TECHNO_SINGLE4,
	STEPS_TYPE_TECHNO_SINGLE5,
	STEPS_TYPE_TECHNO_SINGLE8,
	STEPS_TYPE_TECHNO_DOUBLE4,
	STEPS_TYPE_TECHNO_DOUBLE5,
	STEPS_TYPE_POPN_FIVE,
	STEPS_TYPE_POPN_NINE,
	STEPS_TYPE_LIGHTS_CABINET,
	NUM_STEPS_TYPES,		// leave this at the end
	STEPS_TYPE_INVALID,
};
#define FOREACH_StepsType( st ) FOREACH_ENUM( StepsType, NUM_STEPS_TYPES, st )

//
// Play mode stuff
//
enum PlayMode
{
	PLAY_MODE_REGULAR,
	PLAY_MODE_NONSTOP,
	PLAY_MODE_ONI,
	PLAY_MODE_ENDLESS,
	PLAY_MODE_BATTLE,	// manually launched attacks
	PLAY_MODE_RAVE,		// automatically launched attacks
	NUM_PlayMode,
	PLAY_MODE_INVALID
};
#define FOREACH_PlayMode( pm ) FOREACH_ENUM( PlayMode, NUM_PlayMode, pm )
const RString& PlayModeToString( PlayMode pm );
const RString& PlayModeToLocalizedString( PlayMode pm );
PlayMode StringToPlayMode( const RString& s );
LuaDeclareType( PlayMode );




enum SortOrder 
{
	SORT_PREFERRED,
	SORT_GROUP, 
	SORT_TITLE, 
	SORT_BPM, 
	SORT_POPULARITY, 
	SORT_TOP_GRADES,
	SORT_ARTIST,
	SORT_GENRE,
	SORT_EASY_METER,
	SORT_MEDIUM_METER,
	SORT_HARD_METER,
	SORT_CHALLENGE_METER,
	SORT_MODE_MENU,
	SORT_ALL_COURSES,
	SORT_NONSTOP_COURSES,
	SORT_ONI_COURSES,
	SORT_ENDLESS_COURSES,
	SORT_ROULETTE,
	NUM_SortOrder,
	SortOrder_INVALID
};
const SortOrder MAX_SELECTABLE_SORT = (SortOrder)(SORT_ROULETTE-1);
#define FOREACH_SortOrder( so ) FOREACH_ENUM( SortOrder, NUM_SortOrder, so )
const RString& SortOrderToString( SortOrder so );
SortOrder StringToSortOrder( const RString& str );

inline bool IsSongSort( SortOrder so ) { return so >= SORT_PREFERRED && so <= SORT_CHALLENGE_METER; }

//
// Scoring stuff
//

enum TapNoteScore { 
	TNS_None, 
	TNS_HitMine,
	TNS_AvoidMine,
	TNS_Miss,
	TNS_W5,
	TNS_W4,
	TNS_W3,
	TNS_W2,
	TNS_W1,
	NUM_TapNoteScore,
	TNS_INVALID,
};
#define FOREACH_TapNoteScore( tns ) FOREACH_ENUM( TapNoteScore, NUM_TapNoteScore, tns )
const RString& TapNoteScoreToString( TapNoteScore tns );
const RString& TapNoteScoreToLocalizedString( TapNoteScore tns );
TapNoteScore StringToTapNoteScore( const RString& str );
LuaDeclareType( TapNoteScore );


enum HoldNoteScore 
{ 
	HNS_None,	// this HoldNote has not been scored yet
	HNS_LetGo,		// the HoldNote has passed and they missed it
	HNS_Held,		// the HoldNote has passed and was successfully held all the way through
	NUM_HoldNoteScore,
	HNS_INVALID,
};
#define FOREACH_HoldNoteScore( hns ) FOREACH_ENUM( HoldNoteScore, NUM_HoldNoteScore, hns )
const RString& HoldNoteScoreToString( HoldNoteScore hns );
const RString& HoldNoteScoreToLocalizedString( HoldNoteScore hns );
HoldNoteScore StringToHoldNoteScore( const RString& str );


enum TimingWindow
{
	TW_W1,
	TW_W2,
	TW_W3,
	TW_W4,
	TW_W5,
	TW_Mine,
	TW_Attack,
	TW_Hold,
	TW_Roll,
	NUM_TimingWindow
};
const RString& TimingWindowToString( TimingWindow tw );


enum ScoreEvent
{
	SE_W1,
	SE_W2,
	SE_W3,
	SE_W4,
	SE_W5,
	SE_Miss,
	SE_HitMine,
	SE_Held,
	SE_LetGo,
	NUM_ScoreEvent
};
const RString& ScoreEventToString( ScoreEvent se );


//
// Profile and MemCard stuff
//
enum ProfileSlot
{
	ProfileSlot_Player1,
	ProfileSlot_Player2,
	ProfileSlot_Machine,
	NUM_ProfileSlot,
	ProfileSlot_INVALID
};
#define FOREACH_ProfileSlot( slot ) FOREACH_ENUM( ProfileSlot, NUM_ProfileSlot, slot )


enum MemoryCardState
{ 
	MemoryCardState_Ready, 
	MemoryCardState_Checking, 
	MemoryCardState_TooLate, 
	MemoryCardState_Error, 
	MemoryCardState_Removed,
	MemoryCardState_NoCard,
	NUM_MemoryCardState,
	MemoryCardState_INVALID,
};

const RString& MemoryCardStateToString( MemoryCardState mcs );


//
// Ranking stuff
//
enum RankingCategory
{
	RANKING_A,	// 1-3 meter per song avg.
	RANKING_B,	// 4-6 meter per song avg.
	RANKING_C,	// 7-9 meter per song avg.
	RANKING_D,	// 10+ meter per song avg.	// doesn't count extra stage!
	NUM_RANKING_CATEGORIES,
	RANKING_INVALID
};
#define FOREACH_RankingCategory( rc ) FOREACH_ENUM( RankingCategory, NUM_RANKING_CATEGORIES, rc )
const RString& RankingCategoryToString( RankingCategory rc );
RankingCategory StringToRankingCategory( const RString& rc );

extern const RString RANKING_TO_FILL_IN_MARKER[NUM_PLAYERS];
inline bool IsRankingToFillIn( const RString& sName ) { return !sName.empty() && sName[0]=='#'; }

RankingCategory AverageMeterToRankingCategory( int iAverageMeter );

//
// Group stuff
//
extern const RString GROUP_ALL;


//
//
//
enum PlayerController
{
	PC_HUMAN,
	PC_AUTOPLAY,
	PC_CPU,
	NUM_PlayerController
};
const RString& PlayerControllerToString( PlayerController pc );

const int MIN_SKILL = 0;
const int MAX_SKILL = 10;


enum StageResult
{
	RESULT_WIN,
	RESULT_LOSE,
	RESULT_DRAW
};


//
// Battle stuff
//
const int NUM_INVENTORY_SLOTS	= 3;
enum AttackLevel
{
	ATTACK_LEVEL_1,
	ATTACK_LEVEL_2,
	ATTACK_LEVEL_3,
	NUM_ATTACK_LEVELS
};
const int NUM_ATTACKS_PER_LEVEL	= 3;
const int ITEM_NONE = -1;



//
// Coin stuff
//

enum CoinMode
{
	COIN_MODE_HOME, 
	COIN_MODE_PAY, 
	COIN_MODE_FREE, 
	NUM_COIN_MODES 
};
#define FOREACH_CoinMode( i ) FOREACH_ENUM( CoinMode, NUM_COIN_MODES, i )
const RString& CoinModeToString( CoinMode cm );


//
// Premium
//
enum Premium
{
	PREMIUM_NONE,
	PREMIUM_DOUBLE,
	PREMIUM_JOINT,
	NUM_Premium,
	Premium_Invalidz
};
#define FOREACH_Premium( i ) FOREACH_ENUM( Premium, NUM_Premium, i )
const RString& PremiumToString( Premium p );
LuaDeclareType( Premium );


//
// Award stuff
//

enum PerDifficultyAward
{
	AWARD_FULL_COMBO_W3,
	AWARD_SINGLE_DIGIT_W3,
	AWARD_ONE_W3,
	AWARD_FULL_COMBO_W2,
	AWARD_SINGLE_DIGIT_W2,
	AWARD_ONE_W2,
	AWARD_FULL_COMBO_W1,
	AWARD_PERCENT_80_W3,
	AWARD_PERCENT_90_W3,
	AWARD_PERCENT_100_W3,
	NUM_PerDifficultyAward,
	PER_DIFFICULTY_AWARD_INVALID,
};
#define FOREACH_PerDifficultyAward( pma ) FOREACH_ENUM( PerDifficultyAward, NUM_PerDifficultyAward, pma )
const RString& PerDifficultyAwardToString( PerDifficultyAward pma );
const RString& PerDifficultyAwardToLocalizedString( PerDifficultyAward pma );
PerDifficultyAward StringToPerDifficultyAward( const RString& pma );
LuaDeclareType( PerDifficultyAward );


enum PeakComboAward 
{ 
	AWARD_1000_PEAK_COMBO,
	AWARD_2000_PEAK_COMBO,
	AWARD_3000_PEAK_COMBO,
	AWARD_4000_PEAK_COMBO,
	AWARD_5000_PEAK_COMBO,
	AWARD_6000_PEAK_COMBO,
	AWARD_7000_PEAK_COMBO,
	AWARD_8000_PEAK_COMBO,
	AWARD_9000_PEAK_COMBO,
	AWARD_10000_PEAK_COMBO,
	NUM_PeakComboAward,
	PEAK_COMBO_AWARD_INVALID,
};
#define FOREACH_PeakComboAward( pca ) FOREACH_ENUM( PeakComboAward, NUM_PeakComboAward, pca )
const RString& PeakComboAwardToString( PeakComboAward pma );
const RString& PeakComboAwardToLocalizedString( PeakComboAward pma );
PeakComboAward StringToPeakComboAward( const RString& pma );
LuaDeclareType( PeakComboAward );


struct DisplayBpms
{
	void Add( float f );
	float GetMin() const;
	float GetMax() const;
	bool BpmIsConstant() const;
	bool IsSecret() const;
	vector<float> vfBpms;
};

enum StyleType
{
	ONE_PLAYER_ONE_SIDE,	// e.g. single
	TWO_PLAYERS_TWO_SIDES,	// e.g. versus
	ONE_PLAYER_TWO_SIDES,	// e.g. double
	TWO_PLAYERS_SHARED_SIDES, // e.g. routine
	NUM_STYLE_TYPES,
	STYLE_TYPE_INVALID
};
const RString& StyleTypeToString( StyleType s );
StyleType StringToStyleType( const RString& s );


enum GoalType 
{
	GoalType_Calories, 
	GoalType_Time, 
	GoalType_None,
	NUM_GoalType,
	GoalType_INVALID,
};
#define FOREACH_GoalType( gt ) FOREACH_ENUM( GoalType, NUM_GoalType, gt )
const RString& GoalTypeToString( GoalType gt );
GoalType StringToGoalType( const RString& s );


enum EditMode
{
	EditMode_Practice,
	EditMode_CourseMods,
	EditMode_Home,
	EditMode_Full,
	NUM_EditMode,
	EditMode_INVALID,
};
const RString& EditModeToString( EditMode em );
EditMode StringToEditMode( const RString& s );
LuaDeclareType( EditMode );

enum Stage
{
	STAGE_1,
	STAGE_2,
	STAGE_3,
	STAGE_4,
	STAGE_5,
	STAGE_6,
	STAGE_FINAL,
	STAGE_EXTRA1,
	STAGE_EXTRA2,
	STAGE_NONSTOP,
	STAGE_ONI,
	STAGE_ENDLESS,
	STAGE_EVENT,
	STAGE_DEMO,
	NUM_Stage,
	STAGE_INVALID,
};
#define FOREACH_Stage( s ) FOREACH_ENUM( Stage, NUM_Stage, s )
const RString& StageToString( Stage s );
LuaDeclareType( Stage );


enum ProfileLoadResult
{
	ProfileLoadResult_Success, 
	ProfileLoadResult_FailedNoProfile, 
	ProfileLoadResult_FailedTampered
};


enum MultiPlayerStatus
{
	MultiPlayerStatus_Joined,
	MultiPlayerStatus_NotJoined,
	MultiPlayerStatus_Unplugged,
	MultiPlayerStatus_MissingMultitap,
	NUM_MultiPlayerStatus,
	MultiPlayerStatus_INVALID
};
const RString& MultiPlayerStatusToString( MultiPlayerStatus i );


#endif

/*
 * (c) 2001-2004 Chris Danford, Chris Gomez
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
