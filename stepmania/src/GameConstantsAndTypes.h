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
	NUM_RADAR_CATEGORIES	// leave this at the end
};
#define FOREACH_RadarCategory( rc ) FOREACH_ENUM( RadarCategory, NUM_RADAR_CATEGORIES, rc )
const CString& RadarCategoryToString( RadarCategory cat );
const CString& RadarCategoryToThemedString( RadarCategory cat );


enum StepsType
{
	STEPS_TYPE_DANCE_SINGLE = 0,
	STEPS_TYPE_DANCE_DOUBLE,
	STEPS_TYPE_DANCE_COUPLE,
	STEPS_TYPE_DANCE_SOLO,
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
	NUM_PLAY_MODES,
	PLAY_MODE_INVALID
};
#define FOREACH_PlayMode( pm ) FOREACH_ENUM( PlayMode, NUM_PLAY_MODES, pm )
const CString& PlayModeToString( PlayMode pm );
const CString& PlayModeToThemedString( PlayMode pm );
PlayMode StringToPlayMode( const CString& s );




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
	NUM_SORT_ORDERS,
	SORT_INVALID
};
const SortOrder MAX_SELECTABLE_SORT = (SortOrder)(SORT_ROULETTE-1);
#define FOREACH_SortOrder( so ) FOREACH_ENUM( SortOrder, NUM_SORT_ORDERS, so )
const CString& SortOrderToString( SortOrder so );
SortOrder StringToSortOrder( const CString& str );

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
const CString& TapNoteScoreToString( TapNoteScore tns );
const CString& TapNoteScoreToThemedString( TapNoteScore tns );
TapNoteScore StringToTapNoteScore( const CString& str );


enum HoldNoteScore 
{ 
	HNS_None,	// this HoldNote has not been scored yet
	HNS_LetGo,		// the HoldNote has passed and they missed it
	HNS_Held,		// the HoldNote has passed and was successfully held all the way through
	NUM_HoldNoteScore,
	HNS_INVALID,
};
#define FOREACH_HoldNoteScore( hns ) FOREACH_ENUM( HoldNoteScore, NUM_HoldNoteScore, hns )
const CString& HoldNoteScoreToString( HoldNoteScore hns );
const CString& HoldNoteScoreToThemedString( HoldNoteScore hns );
HoldNoteScore StringToHoldNoteScore( const CString& str );


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
const CString& TimingWindowToString( TimingWindow tw );


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
const CString& ScoreEventToString( ScoreEvent se );


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

const CString& MemoryCardStateToString( MemoryCardState mcs );


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
const CString& RankingCategoryToString( RankingCategory rc );
RankingCategory StringToRankingCategory( const CString& rc );

extern const CString RANKING_TO_FILL_IN_MARKER[NUM_PLAYERS];
inline bool IsRankingToFillIn( const CString& sName ) { return !sName.empty() && sName[0]=='#'; }

RankingCategory AverageMeterToRankingCategory( int iAverageMeter );

//
// Group stuff
//
extern const CString GROUP_ALL;


//
//
//
enum PlayerController
{
	PC_HUMAN,
	PC_AUTOPLAY,
	PC_CPU,
	NUM_PLAYER_CONTROLLERS
};
const CString& PlayerControllerToString( PlayerController pc );

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
const CString& CoinModeToString( CoinMode cm );


//
// Premium
//
enum Premium
{
	PREMIUM_NONE,
	PREMIUM_DOUBLE,
	PREMIUM_JOINT,
	NUM_PREMIUMS
};
#define FOREACH_Premium( i ) FOREACH_ENUM( Premium, NUM_PREMIUMS, i )
const CString& PremiumToString( Premium p );


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
	NUM_PER_DIFFICULTY_AWARDS,
	PER_DIFFICULTY_AWARD_INVALID,
};
#define FOREACH_PerDifficultyAward( pma ) FOREACH_ENUM( PerDifficultyAward, NUM_PER_DIFFICULTY_AWARDS, pma )
const CString& PerDifficultyAwardToString( PerDifficultyAward pma );
const CString& PerDifficultyAwardToThemedString( PerDifficultyAward pma );
PerDifficultyAward StringToPerDifficultyAward( const CString& pma );


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
	NUM_PEAK_COMBO_AWARDS,
	PEAK_COMBO_AWARD_INVALID,
};
#define FOREACH_PeakComboAward( pca ) FOREACH_ENUM( PeakComboAward, NUM_PEAK_COMBO_AWARDS, pca )
const CString& PeakComboAwardToString( PeakComboAward pma );
const CString& PeakComboAwardToThemedString( PeakComboAward pma );
PeakComboAward StringToPeakComboAward( const CString& pma );


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
	NUM_STYLE_TYPES,
	STYLE_TYPE_INVALID
};
const CString& StyleTypeToString( StyleType s );
StyleType StringToStyleType( const CString& s );


enum MenuDir
{
	MENU_DIR_UP,
	MENU_DIR_DOWN,
	MENU_DIR_LEFT,
	MENU_DIR_RIGHT,
	MENU_DIR_AUTO, // when players join and the selection becomes invalid
	NUM_MENU_DIRS
};
#define FOREACH_MenuDir( md ) FOREACH_ENUM( MenuDir, NUM_MENU_DIRS, md )
const CString& MenuDirToString( MenuDir md );


enum GoalType 
{
	GOAL_CALORIES, 
	GOAL_TIME, 
	GOAL_NONE,
	NUM_GOAL_TYPES,
	GOAL_INVALID,
};
#define FOREACH_GoalType( md ) FOREACH_ENUM( GoalType, NUM_GOAL_TYPES, gt )
const CString& GoalTypeToString( GoalType gt );
GoalType StringToGoalType( const CString& s );


enum EditMode
{
	EditMode_Practice,
	EditMode_Home,
	EditMode_Full,
	NUM_EditMode,
	EditMode_INVALID,
};


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
	NUM_STAGES,
	STAGE_INVALID,
};
#define FOREACH_Stage( s ) FOREACH_ENUM( Stage, NUM_STAGES, s )
const CString& StageToString( Stage s );


enum ProfileLoadResult
{
	ProfileLoadResult_Success, 
	ProfileLoadResult_FailedNoProfile, 
	ProfileLoadResult_FailedTampered
};


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
