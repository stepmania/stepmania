/* GameConstantsAndTypes - Things used in many places that don't change often. */

#ifndef GAME_CONSTANTS_AND_TYPES_H
#define GAME_CONSTANTS_AND_TYPES_H

#include "EnumHelper.h"

// Note definitions
// Use 1-35 instead of 1-13. -aj
/* 35 is used because we have to be mindful of Profile data.
 * See Profile::InitGeneralData() for how MAX_METER is used. -aj */
const int MIN_METER = 1;
const int MAX_METER = 35;

// Credits
const int MAX_NUM_CREDITS = 20;


/* This is just cached song data. Not all of it may actually be displayed
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
	RadarCategory_Lifts,
	NUM_RadarCategory,	// leave this at the end
	RadarCategory_Invalid
};
const RString& RadarCategoryToString( RadarCategory cat );
const RString& RadarCategoryToLocalizedString( RadarCategory cat );
LuaDeclareType( RadarCategory );


enum StepsTypeCategory
{
	StepsTypeCategory_Single,
	StepsTypeCategory_Double,
	StepsTypeCategory_Couple,
	StepsTypeCategory_Routine,
};


enum StepsType
{
	StepsType_dance_single = 0,
	StepsType_dance_double,
	StepsType_dance_couple,
	StepsType_dance_solo,
	StepsType_dance_threepanel,
	StepsType_dance_routine,
	StepsType_pump_single,
	StepsType_pump_halfdouble,
	StepsType_pump_double,
	StepsType_pump_couple,
	StepsType_pump_routine,
	StepsType_kb7_single,
	StepsType_ez2_single,
	StepsType_ez2_double,
	StepsType_ez2_real,
	StepsType_para_single,
	StepsType_ds3ddx_single,
	StepsType_beat_single5,
	StepsType_beat_double5,
	StepsType_beat_single7,
	StepsType_beat_double7,
	StepsType_maniax_single,
	StepsType_maniax_double,
	StepsType_techno_single4,
	StepsType_techno_single5,
	StepsType_techno_single8,
	StepsType_techno_double4,
	StepsType_techno_double5,
	StepsType_popn_five,
	StepsType_popn_nine,
	StepsType_guitar_five,
	StepsType_lights_cabinet,
	NUM_StepsType,		// leave this at the end
	StepsType_Invalid,
};
LuaDeclareType( StepsType );

// Play mode stuff
enum PlayMode
{
	PLAY_MODE_REGULAR,
	PLAY_MODE_NONSTOP,
	PLAY_MODE_ONI,
	PLAY_MODE_ENDLESS,
	PLAY_MODE_BATTLE,	// manually launched attacks
	PLAY_MODE_RAVE,		// automatically launched attacks
	NUM_PlayMode,
	PlayMode_Invalid
};
const RString& PlayModeToString( PlayMode pm );
const RString& PlayModeToLocalizedString( PlayMode pm );
PlayMode StringToPlayMode( const RString& s );
LuaDeclareType( PlayMode );

enum SortOrder 
{
	// song sorts
	SORT_PREFERRED,
	SORT_GROUP, 
	SORT_TITLE, 
	SORT_BPM, 
	SORT_POPULARITY, 
	SORT_TOP_GRADES,
	SORT_ARTIST,
	SORT_GENRE,
	SORT_BEGINNER_METER,
	SORT_EASY_METER,
	SORT_MEDIUM_METER,
	SORT_HARD_METER,
	SORT_CHALLENGE_METER,
	SORT_DOUBLE_EASY_METER,
	SORT_DOUBLE_MEDIUM_METER,
	SORT_DOUBLE_HARD_METER,
	SORT_DOUBLE_CHALLENGE_METER,
	//
	SORT_MODE_MENU,
	// course sorts
	SORT_ALL_COURSES,
	SORT_NONSTOP_COURSES,
	SORT_ONI_COURSES,
	SORT_ENDLESS_COURSES,
	SORT_LENGTH,
	SORT_ROULETTE,
	SORT_RECENT,
	NUM_SortOrder,
	SortOrder_Invalid
};
const SortOrder MAX_SELECTABLE_SORT = (SortOrder)(SORT_ROULETTE-1);
const RString& SortOrderToString( SortOrder so );
const RString& SortOrderToLocalizedString( SortOrder so );
SortOrder StringToSortOrder( const RString& str );
LuaDeclareType( SortOrder );
// IsSongSort is only used for saving sort order to the profile. -aj
inline bool IsSongSort( SortOrder so ) { return so >= SORT_PREFERRED && so <= SORT_DOUBLE_CHALLENGE_METER; }

// Scoring stuff
enum TapNoteScore { 
	TNS_None, 
	TNS_HitMine,
	TNS_AvoidMine,
	TNS_CheckpointMiss,
	TNS_Miss,
	TNS_W5,
	TNS_W4,
	TNS_W3,
	TNS_W2,
	TNS_W1,
	TNS_CheckpointHit,
	NUM_TapNoteScore,
	TapNoteScore_Invalid,
};
const RString& TapNoteScoreToString( TapNoteScore tns );
const RString& TapNoteScoreToLocalizedString( TapNoteScore tns );
TapNoteScore StringToTapNoteScore( const RString& str );
LuaDeclareType( TapNoteScore );

enum HoldNoteScore 
{ 
	HNS_None,		// HoldNote not scored yet
	HNS_LetGo,		// HoldNote has passed, missed it
	HNS_Held,		// HoldNote has passed, successfully held all the way
	NUM_HoldNoteScore,
	HoldNoteScore_Invalid,
};
const RString& HoldNoteScoreToString( HoldNoteScore hns );
const RString& HoldNoteScoreToLocalizedString( HoldNoteScore hns );
HoldNoteScore StringToHoldNoteScore( const RString& str );
LuaDeclareType( HoldNoteScore );

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
	SE_CheckpointHit,
	SE_W1,
	SE_W2,
	SE_W3,
	SE_W4,
	SE_W5,
	SE_Miss,
	SE_HitMine,
	SE_CheckpointMiss,
	SE_Held,
	SE_LetGo,
	NUM_ScoreEvent
};
const RString& ScoreEventToString( ScoreEvent se );

enum GameButtonType
{
	GameButtonType_Step,
	GameButtonType_Fret,
	GameButtonType_Strum,
	GameButtonType_INVALID
};

enum TapNoteScoreJudgeType
{
	TapNoteScoreJudgeType_MinimumScore,
	TapNoteScoreJudgeType_LastScore,
	NUM_TapNoteScoreJudgeType,
	TapNoteScoreJudgeType_Invalid,
};
const RString& TapNoteScoreJudgeTypeToString( TapNoteScoreJudgeType jt );
LuaDeclareType( TapNoteScoreJudgeType );


// Profile and MemCard stuff
enum ProfileSlot
{
	ProfileSlot_Player1,
	ProfileSlot_Player2,
	ProfileSlot_Machine,
	NUM_ProfileSlot,
	ProfileSlot_Invalid
};
const RString& ProfileSlotToString( ProfileSlot ps );
LuaDeclareType( ProfileSlot );


enum MemoryCardState
{ 
	MemoryCardState_Ready, 
	MemoryCardState_Checking, 
	MemoryCardState_TooLate, 
	MemoryCardState_Error, 
	MemoryCardState_Removed,
	MemoryCardState_NoCard,
	NUM_MemoryCardState,
	MemoryCardState_Invalid,
};

const RString& MemoryCardStateToString( MemoryCardState mcs );
LuaDeclareType( MemoryCardState );

// Ranking stuff
enum RankingCategory
{
	RANKING_A,	// 1-3 meter per song avg.
	RANKING_B,	// 4-6 meter per song avg.
	RANKING_C,	// 7-9 meter per song avg.
	RANKING_D,	// 10+ meter per song avg.	// doesn't count extra stage!
	NUM_RankingCategory,
	RankingCategory_Invalid
};
const RString& RankingCategoryToString( RankingCategory rc );
RankingCategory StringToRankingCategory( const RString& rc );

extern const vector<RString> RANKING_TO_FILL_IN_MARKER;
inline bool IsRankingToFillIn( const RString& sName ) { return !sName.empty() && sName[0]=='#'; }

RankingCategory AverageMeterToRankingCategory( int iAverageMeter );

// Group stuff
extern const RString GROUP_ALL;


//
enum PlayerController
{
	PC_HUMAN,
	PC_AUTOPLAY,
	PC_CPU,
	//PC_REPLAY,
	NUM_PlayerController,
	PlayerController_Invalid
};
const RString& PlayerControllerToString( PlayerController pc );
LuaDeclareType( PlayerController );

enum HealthState
{
	HealthState_Hot,
	HealthState_Alive,
	HealthState_Danger,
	HealthState_Dead,
	NUM_HealthState,
	HealthState_Invalid
};
LuaDeclareType( HealthState );

enum StageResult
{
	RESULT_WIN,
	RESULT_LOSE,
	RESULT_DRAW
};


// Battle stuff
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


// Coin stuff
enum CoinMode
{
	CoinMode_Home, 
	CoinMode_Pay, 
	CoinMode_Free, 
	NUM_CoinMode,
	CoinMode_Invalid
};
const RString& CoinModeToString( CoinMode cm );
LuaDeclareType( CoinMode );


// Premium
enum Premium
{
	Premium_Off,
	Premium_DoubleFor1Credit,
	Premium_2PlayersFor1Credit,
	NUM_Premium,
	Premium_Invalid
};
const RString& PremiumToString( Premium p );
const RString& PremiumToLocalizedString( Premium p );
LuaDeclareType( Premium );


// Award stuff
enum StageAward
{
	StageAward_FullComboW3,
	StageAward_SingleDigitW3,
	StageAward_OneW3,
	StageAward_FullComboW2,
	StageAward_SingleDigitW2,
	StageAward_OneW2,
	StageAward_FullComboW1,
	StageAward_80PercentW3,
	StageAward_90PercentW3,
	StageAward_100PercentW3,
	NUM_StageAward,
	StageAward_Invalid,
};
const RString& StageAwardToString( StageAward pma );
const RString& StageAwardToLocalizedString( StageAward pma );
StageAward StringToStageAward( const RString& pma );
LuaDeclareType( StageAward );


enum PeakComboAward 
{ 
	PeakComboAward_1000,
	PeakComboAward_2000,
	PeakComboAward_3000,
	PeakComboAward_4000,
	PeakComboAward_5000,
	PeakComboAward_6000,
	PeakComboAward_7000,
	PeakComboAward_8000,
	PeakComboAward_9000,
	PeakComboAward_10000,
	NUM_PeakComboAward,
	PeakComboAward_Invalid,
};
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
	StyleType_OnePlayerOneSide,		// e.g. single
	StyleType_TwoPlayersTwoSides,	// e.g. versus
	StyleType_OnePlayerTwoSides,	// e.g. double
	StyleType_TwoPlayersSharedSides, // e.g. routine
	NUM_StyleType,
	StyleType_Invalid
};
const RString& StyleTypeToString( StyleType s );
StyleType StringToStyleType( const RString& s );
LuaDeclareType( StyleType );


enum GoalType 
{
	GoalType_Calories, 
	GoalType_Time, 
	GoalType_None,
	NUM_GoalType,
	GoalType_Invalid,
};
const RString& GoalTypeToString( GoalType gt );
GoalType StringToGoalType( const RString& s );
LuaDeclareType( GoalType );


enum EditMode
{
	EditMode_Practice,
	EditMode_CourseMods,
	EditMode_Home,
	EditMode_Full,
	NUM_EditMode,
	EditMode_Invalid,
};
const RString& EditModeToString( EditMode em );
EditMode StringToEditMode( const RString& s );
LuaDeclareType( EditMode );

/*
original options from ScreenEz2SelectMusic:
(if no confirm type is mentioned, there is none.)

0 = play music as you select;			SampleMusicPreviewMode_Normal
1 = no music plays, select 1x to play preview music, select again to confirm
2 = no music plays at all				(SampleMusicPreviewMode_ScreenMusic + redir to silent)
3 = play music as select, 2x to confirm (SampleMusicPreviewMode_Normal + [SSMusic] TwoPartConfirmsOnly)
4 = screen music plays;					SampleMusicPreviewMode_ScreenMusic
*/
enum SampleMusicPreviewMode
{
	SampleMusicPreviewMode_Normal,
	SampleMusicPreviewMode_StartToPreview,
	SampleMusicPreviewMode_ScreenMusic,
	SampleMusicPreviewMode_LastSong,	// continue playing the last song
	NUM_SampleMusicPreviewMode,
	SampleMusicPreviewMode_Invalid,
};
const RString& SampleMusicPreviewModeToString( SampleMusicPreviewMode );
SampleMusicPreviewMode StringToSampleMusicPreviewMode( const RString& s );
LuaDeclareType( SampleMusicPreviewMode );

enum Stage // Shared stage values (not per-player) that are shown in StageDisplay
{
	Stage_1st,
	Stage_2nd,
	Stage_3rd,
	Stage_4th,
	Stage_5th,
	Stage_6th,
	Stage_Next, // after Stage_6th but not Final. This won't normally happen because 7 stages is the max in the UI.
	Stage_Final,
	Stage_Extra1,
	Stage_Extra2,
	Stage_Nonstop,
	Stage_Oni,
	Stage_Endless,
	Stage_Event,
	Stage_Demo,
	NUM_Stage,
	Stage_Invalid,
};
const RString& StageToString( Stage s );
LuaDeclareType( Stage );
const RString& StageToLocalizedString( Stage i );


enum EarnedExtraStage
{
	EarnedExtraStage_No,
	EarnedExtraStage_Extra1,
	EarnedExtraStage_Extra2,
	NUM_EarnedExtraStage,
	EarnedExtraStage_Invalid
};
const RString& EarnedExtraStageToString( EarnedExtraStage s );
LuaDeclareType( EarnedExtraStage );


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
	MultiPlayerStatus_Invalid
};
const RString& MultiPlayerStatusToString( MultiPlayerStatus i );


enum CourseType
{
	COURSE_TYPE_NONSTOP,	// if life meter type is BAR
	COURSE_TYPE_ONI,		// if life meter type is BATTERY
	COURSE_TYPE_ENDLESS,	// if set to REPEAT
	COURSE_TYPE_SURVIVAL,	// if life meter type is TIME
	NUM_CourseType,
	CourseType_Invalid
};
#define FOREACH_CourseType( i ) FOREACH_ENUM( CourseType, i )
const RString& CourseTypeToString( CourseType i );
const RString& CourseTypeToLocalizedString( CourseType i );
LuaDeclareType( CourseType );


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
