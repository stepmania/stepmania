/* GameConstantsAndTypes - Things used in many places that don't change often. */

#ifndef GAME_CONSTANTS_AND_TYPES_H
#define GAME_CONSTANTS_AND_TYPES_H

#include "EnumHelper.h"
#include <float.h> // need the max for default.

// Note definitions
/** @brief Define the mininum difficulty value allowed. */
const int MIN_METER = 1;
/**
 * @brief Define the maximum difficulty value allowed.
 *
 * 35 is used rather than 13 due to a variety of Profile data.
 * For more examples, see Profile::InitGeneralData(). -aj
 */
const int MAX_METER = 35;

/** @brief The maximum number of credits for coin mode. */
const int MAX_NUM_CREDITS = 20;


/**
 * @brief The various radar categories available.
 *
 * This is just cached song data. Not all of it may actually be displayed
 * in the radar. */
enum RadarCategory
{
	RadarCategory_Stream = 0, /**< How much stream is in the song? */
	RadarCategory_Voltage, /**< How much voltage is in the song? */
	RadarCategory_Air, /**< How much air is in the song? */
	RadarCategory_Freeze, /**< How much freeze (holds) is in the song? */
	RadarCategory_Chaos, /**< How much chaos is in the song? */
	RadarCategory_Notes, /**< How many notes are in the song? */
	RadarCategory_TapsAndHolds, /**< How many taps and holds are in the song? */
	RadarCategory_Jumps, /**< How many jumps are in the song? */
	RadarCategory_Holds, /**< How many holds are in the song? */
	RadarCategory_Mines, /**< How many mines are in the song? */
	RadarCategory_Hands, /**< How many hands are in the song? */
	RadarCategory_Rolls, /**< How many rolls are in the song? */
	RadarCategory_Lifts, /**< How many lifts are in the song? */
	RadarCategory_Fakes, /**< How many fakes are in the song? */
	// If you add another radar category, make sure you update
	// NoteDataUtil::CalculateRadarValues to calculate it.
	// Also update NoteDataWithScoring::GetActualRadarValues. -Kyz
	NUM_RadarCategory,	/**< The number of radar categories. */
	RadarCategory_Invalid
};
/**
 * @brief Turn the radar category into a proper string.
 * @param cat the radar category.
 * @return the string version of the radar category.
 */
const RString& RadarCategoryToString( RadarCategory cat );
/**
 * @brief Turn the radar category into a proper localized string.
 * @param cat the radar category.
 * @return the localized string version of the radar category.
 */
const RString& RadarCategoryToLocalizedString( RadarCategory cat );
LuaDeclareType( RadarCategory );

/** @brief The different game categories available to play. */
enum StepsTypeCategory
{
	StepsTypeCategory_Single, /**< One person plays on one side. */
	StepsTypeCategory_Double, /**< One person plays on both sides. */
	StepsTypeCategory_Couple, /**< Two players play on their own side. */
	StepsTypeCategory_Routine, /**< Two players share both sides together. */
};

/** @brief The different steps types for playing. */
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
	StepsType_beat_versus5,
	StepsType_beat_double5,
	StepsType_beat_single7,
	StepsType_beat_versus7,
	StepsType_beat_double7,
	StepsType_maniax_single,
	StepsType_maniax_double,
	StepsType_techno_single4,
	StepsType_techno_single5,
	StepsType_techno_single8,
	StepsType_techno_double4,
	StepsType_techno_double5,
	StepsType_techno_double8,
	StepsType_popn_five,
	StepsType_popn_nine,
	StepsType_lights_cabinet,
	StepsType_kickbox_human,
	StepsType_kickbox_quadarm,
	StepsType_kickbox_insect,
	StepsType_kickbox_arachnid,
	NUM_StepsType,		// leave this at the end
	StepsType_Invalid,
};
LuaDeclareType( StepsType );

/** @brief The various play modes available. */
enum PlayMode
{
	PLAY_MODE_REGULAR, /**< The normal game mode, often with a set number of stages. */
	PLAY_MODE_NONSTOP, /**< Play a set of songs without stopping. */
	PLAY_MODE_ONI, /**< Similar to Nonstop, only there is also the danger of lives or a clock. */
	PLAY_MODE_ENDLESS, /**< Keep playing until you get a game over. */
	PLAY_MODE_BATTLE, /**< Choose when to send attacks to your opponent. */
	PLAY_MODE_RAVE,	/**< Have attacks launched during play automatically. */
	NUM_PlayMode,
	PlayMode_Invalid
};
/**
 * @brief Turn the play mode into a proper string.
 * @param pm the play mode.
 * @return the string version of the play mode.
 */
const RString& PlayModeToString( PlayMode pm );
/**
 * @brief Turn the play mode into a proper localized string.
 * @param pm the play mode.
 * @return the localized string version of the play mode.
 */
const RString& PlayModeToLocalizedString( PlayMode pm );
/**
 * @brief Turn the string into the proper play mode.
 * @param s the string.
 * @return the play mode based on the string.
 */
PlayMode StringToPlayMode( const RString& s );
LuaDeclareType( PlayMode );

/** 
 * @brief The list of ways to sort songs and courses.
 *
 * All song sorts should be listed before course sorts.
 */
enum SortOrder 
{
	// song sorts
	SORT_PREFERRED, /**< Sort by the user's preferred settings. */
	SORT_GROUP, /**< Sort by the groups the Songs are in. */
	SORT_TITLE, /**< Sort by the Song's title. */
	SORT_BPM, /**< Sort by the Song's BPM. */
	SORT_POPULARITY, /**< Sort by how popular the Song is. */
	SORT_TOP_GRADES, /**< Sort by the highest grades earned on a Song. */
	SORT_ARTIST, /**< Sort by the name of the artist of the Song. */
	SORT_GENRE, /**< Sort by the Song's genre. */
	SORT_BEGINNER_METER, /**< Sort by the difficulty of the single beginner meter. */
	SORT_EASY_METER, /**< Sort by the difficulty of the single easy meter. */
	SORT_MEDIUM_METER, /**< Sort by the difficulty of the single medium meter. */
	SORT_HARD_METER, /**< Sort by the difficulty of the single hard meter. */
	SORT_CHALLENGE_METER, /**< Sort by the difficulty of the single challenge meter. */
	SORT_DOUBLE_EASY_METER, /**< Sort by the difficulty of the double easy meter. */
	SORT_DOUBLE_MEDIUM_METER, /**< Sort by the difficulty of the double medium meter. */
	SORT_DOUBLE_HARD_METER, /**< Sort by the difficulty of the double hard meter. */
	SORT_DOUBLE_CHALLENGE_METER, /**< Sort by the difficulty of the double challenge meter. */
	//
	SORT_MODE_MENU, /**< Have access to the menu for choosing the sort. */
	// course sorts
	SORT_ALL_COURSES, /**< Sort with all courses available. */
	SORT_NONSTOP_COURSES, /**< View only the nonstop courses. */
	SORT_ONI_COURSES, /**< View only the oni/survival courses. */
	SORT_ENDLESS_COURSES, /**< View only the endless courses. */
	SORT_LENGTH, /**< Sort the songs/courses by how long they would last. */
	SORT_ROULETTE,
	SORT_RECENT,
	NUM_SortOrder,
	SortOrder_Invalid
};
/** @brief Only allow certain sort modes to be selectable. */
const SortOrder MAX_SELECTABLE_SORT = (SortOrder)(SORT_ROULETTE-1);
/**
 * @brief Turn the sort order into a proper string.
 * @param so the sort order.
 * @return the string version of the sort order.
 */
const RString& SortOrderToString( SortOrder so );
/**
 * @brief Turn the sort order into a proper localized string.
 * @param so the sort order.
 * @return the localized string version of the sort order.
 */
const RString& SortOrderToLocalizedString( SortOrder so );
/**
 * @brief Turn the string into the proper sort order.
 * @param str the string.
 * @return the sort order based on the string.
 */
SortOrder StringToSortOrder( const RString& str );
LuaDeclareType( SortOrder );
/**
 * @brief Determine if the sort order in question is for songs or not.
 *
 * This function is mainly used for saving sort order to the profile. -aj
 */
inline bool IsSongSort( SortOrder so ) { return (so >= SORT_PREFERRED && so <= SORT_DOUBLE_CHALLENGE_METER) || so == SORT_LENGTH; }

/** @brief The list of tap note scores available during play. */
enum TapNoteScore { 
	TNS_None, /**< There is no score involved with this one. */
	TNS_HitMine, /**< A mine was hit successfully. */
	TNS_AvoidMine, /**< A mine was avoided successfully. */
	TNS_CheckpointMiss, /**< A checkpoint was missed during a hold. */
	TNS_Miss, /**< A note was missed entirely. */
	TNS_W5, /**< A note was almost missed, but not quite. */
	TNS_W4, /**< A note was hit either a bit early or a bit late. */
	TNS_W3, /**< A note was hit with decent accuracy, but not the best. */
	TNS_W2, /**< A note was hit off by just a miniscule amount. This used to be the best rating. */
	TNS_W1, /**< A note was hit perfectly. */
	TNS_CheckpointHit, /**< A checkpoint was held during a hold. */
	NUM_TapNoteScore, /**< The number of Tap Note Scores available. */
	TapNoteScore_Invalid,
};
/**
 * @brief Turn the tap note score into a proper string.
 * @param tns the tap note score.
 * @return the string version of the tap note score.
 */
const RString& TapNoteScoreToString( TapNoteScore tns );
/**
 * @brief Turn the tap note score into a proper localized string.
 * @param tns the tap note score.
 * @return the localized string version of the tap note score.
 */
const RString& TapNoteScoreToLocalizedString( TapNoteScore tns );
/**
 * @brief Turn the string into the proper tap note score.
 * @param str the string.
 * @return the tap note score based on the string.
 */
TapNoteScore StringToTapNoteScore( const RString& str );
LuaDeclareType( TapNoteScore );

/** @brief The list of hold note scores available during play. */
enum HoldNoteScore 
{ 
	HNS_None,		/**< The HoldNote was not scored yet. */
	HNS_LetGo,		/**< The HoldNote has passed, but the player missed it. */
	HNS_Held,		/**< The HoldNote has passed, and was successfully held all the way. */
	HNS_Missed,		/**< The HoldNote has passed, and was never initialized. */
	NUM_HoldNoteScore,	/**< The number of hold note scores. */
	HoldNoteScore_Invalid,
};
/**
 * @brief Turn the hold note score into a proper string.
 * @param hns the hold note score.
 * @return the string version of the hold note score.
 */
const RString& HoldNoteScoreToString( HoldNoteScore hns );
/**
 * @brief Turn the hold note score into a proper localized string.
 * @param hns the hold note score.
 * @return the localized string version of the hold note score.
 */
const RString& HoldNoteScoreToLocalizedString( HoldNoteScore hns );
/**
 * @brief Turn the string into the proper hold note score.
 * @param str the string.
 * @return the hold note score based on the string.
 */
HoldNoteScore StringToHoldNoteScore( const RString& str );
LuaDeclareType( HoldNoteScore );

/** @brief The list of timing windows to deal with when playing. */
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
	TW_Checkpoint,
	NUM_TimingWindow
};
const RString& TimingWindowToString( TimingWindow tw );

/** @brief The list of score events that can take place while playing. */
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
	SE_Missed,
	NUM_ScoreEvent
};
const RString& ScoreEventToString( ScoreEvent se );

/** @brief The list of game button types available for all game modes. */
enum GameButtonType
{
	GameButtonType_Step,
	GameButtonType_Menu
};

/** @brief The list of judge types for the tap note scores. */
enum TapNoteScoreJudgeType
{
	TapNoteScoreJudgeType_MinimumScore,
	TapNoteScoreJudgeType_LastScore,
	NUM_TapNoteScoreJudgeType,
	TapNoteScoreJudgeType_Invalid,
};
const RString& TapNoteScoreJudgeTypeToString( TapNoteScoreJudgeType jt );
LuaDeclareType( TapNoteScoreJudgeType );


/** @brief The profile slots available. This is mainly for Profiles and Memory Cards. */
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

/** @brief The states of the memory card during play. */
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

/** @brief The different ranking categories based on difficulty meter average. */
enum RankingCategory
{
	RANKING_A,	/**< 1-3 meter per song avg. */
	RANKING_B,	/**< 4-6 meter per song avg. */
	RANKING_C,	/**< 7-9 meter per song avg. */
	RANKING_D,	/**< 10+ meter per song avg, not counting extra stages. */
	NUM_RankingCategory, /**< The number of ranking categories. */
	RankingCategory_Invalid
};
const RString& RankingCategoryToString( RankingCategory rc );
RankingCategory StringToRankingCategory( const RString& rc );
LuaDeclareType( RankingCategory );

extern const vector<RString> RANKING_TO_FILL_IN_MARKER;
inline bool IsRankingToFillIn( const RString& sName ) { return !sName.empty() && sName[0]=='#'; }

RankingCategory AverageMeterToRankingCategory( int iAverageMeter );

// Group stuff
extern const RString GROUP_ALL;


/** @brief The different types of players in the game. */
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

/** @brief The different health bar states. */
enum HealthState
{
	HealthState_Hot, /**< The health bar is very full. */
	HealthState_Alive, /**< The health bar is at a decent size. */
	HealthState_Danger, /**< The health bar is about to run out. */
	HealthState_Dead, /**< The health bar is drained completely. */
	NUM_HealthState,
	HealthState_Invalid
};
LuaDeclareType( HealthState );

/** @brief The different stage results during battle. */
enum StageResult
{
	RESULT_WIN,		/**< The player has won the battle. */
	RESULT_LOSE,	/**< The player has lost the battle. */
	RESULT_DRAW,	/**< The player has tied with the competitor. */
	NUM_StageResult,
	StageResult_Invalid
};
LuaDeclareType( StageResult );

// Battle stuff
/** @brief The number of inventory slots available for attacks. */
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
/** @brief The different coin modes to determine how one can play. */
enum CoinMode
{
	CoinMode_Home, /**< The full range of options are available. */
	CoinMode_Pay, /**< Coins must be inserted before a game can begin. */
	CoinMode_Free, /**< It costs no money to play, but otherwise is similar to Pay mode. */
	NUM_CoinMode,
	CoinMode_Invalid
};
const RString& CoinModeToString( CoinMode cm );
LuaDeclareType( CoinMode );


/** @brief The different types of premiums available to take advantage of. */
enum Premium
{
	Premium_Off, /**< It will cost one credit per side of the machine. */
	Premium_DoubleFor1Credit, /**< It will cost one credit per player of the machine. */
	Premium_2PlayersFor1Credit, /**< One credit gives one or both players full access. */
	NUM_Premium,
	Premium_Invalid
};
const RString& PremiumToString( Premium p );
const RString& PremiumToLocalizedString( Premium p );
LuaDeclareType( Premium );


/** @brief The various stage awards that can be given based on excellent play. */
enum StageAward
{
	StageAward_FullComboW3, /**< A full great combo (or equivalent) was earned. */
	StageAward_SingleDigitW3, /**< A single digit great combo (or equivalent) was earned. */
	StageAward_OneW3, /**< Only one great (or equivalent) was earned. */
	StageAward_FullComboW2, /**< A full excellent combo (or equivalent) was earned. */
	StageAward_SingleDigitW2, /**< A single digit excellent combo (or equivalent) was earned. */
	StageAward_OneW2, /**< Only one excellent (or equivalent) was earned. */
	StageAward_FullComboW1, /**< All fantastics (or equivalent) were earned. */
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

/** @brief The various peak combo awards should such a combo be attained during play. */
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

/** @brief The list of BPMs to display */
struct DisplayBpms
{
	/**
	 * @brief Add a BPM to the list.
	 * @param f the BPM to add.
	 */
	void Add( float f );
	/**
	 * @brief Retrieve the minimum BPM of the set.
	 * @return the minimum BPM.
	 */
	float GetMin() const;
	/**
	 * @brief Retrieve the maximum BPM of the set.
	 * @return the maximum BPM.
	 */
	float GetMax() const;
	/**
	 * @brief Retrieve the maximum BPM of the set,
	 * but no higher than a certain value.
	 * @param highest the highest BPM to use.
	 * @return the maximum BPM.
	 */
	float GetMaxWithin(float highest = FLT_MAX) const;
	/**
	 * @brief Determine if the BPM is really constant.
	 * @return Whether the BPM is constant or not.
	 */
	bool BpmIsConstant() const;
	/**
	 * @brief Determine if the BPM is meant to be a secret.
	 * @return Whether the BPM is a secret or not.
	 */
	bool IsSecret() const;
	/**
	 * @brief The list of the BPMs for the song or course.
	 */
	vector<float> vfBpms;
};

/** @brief The various style types available. */
enum StyleType
{
	StyleType_OnePlayerOneSide,		/**< Single style */
	StyleType_TwoPlayersTwoSides,		/**< Versus style */
	StyleType_OnePlayerTwoSides,		/**< Double style */
	StyleType_TwoPlayersSharedSides,	/**< Routine style */
	NUM_StyleType,
	StyleType_Invalid
};
const RString& StyleTypeToString( StyleType s );
StyleType StringToStyleType( const RString& s );
LuaDeclareType( StyleType );

/** @brief The different goal types, mainly meant for fitness modes. */
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

/** @brief The different types of Edit modes available. */
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

/**
 * @brief The different types of sample music previews available.
 *
 * These were originally from the deleted screen ScreenEz2SelectMusic.
 * (if no confirm type is mentioned, there is none.)
 *
 * 0 = play music as you select;				SampleMusicPreviewMode_Normal
 * 1 = no music plays, select 1x to play preview music, select again to confirm
 * 2 = no music plays at all					(SampleMusicPreviewMode_ScreenMusic + redir to silent)
 * 3 = play music as select, 2x to confirm			(SampleMusicPreviewMode_Normal + [SSMusic] TwoPartConfirmsOnly)
 * 4 = screen music plays;					SampleMusicPreviewMode_ScreenMusic
 */
enum SampleMusicPreviewMode
{
	SampleMusicPreviewMode_Normal,		/**< Music is played as the song is highlighted. */
	SampleMusicPreviewMode_StartToPreview,	
	SampleMusicPreviewMode_ScreenMusic,	/**< No music plays. Select it once to preview the music, 
						 * then once more to select the song. */
	SampleMusicPreviewMode_LastSong,	/**< continue playing the last song */
	NUM_SampleMusicPreviewMode,
	SampleMusicPreviewMode_Invalid,
};
const RString& SampleMusicPreviewModeToString( SampleMusicPreviewMode );
SampleMusicPreviewMode StringToSampleMusicPreviewMode( const RString& s );
LuaDeclareType( SampleMusicPreviewMode );

/**
 * @brief The different kinds of Stages available.
 *
 * These are shared stage values shown in StageDisplay. These are not per-player.
 */
enum Stage
{
	Stage_1st, /**< The first stage. */
	Stage_2nd, /**< The second stage. */
	Stage_3rd, /**< The third stage. */
	Stage_4th, /**< The fourth stage. */
	Stage_5th, /**< The fifth stage. */
	Stage_6th, /**< The sixth stage. */
	Stage_Next, /**< Somewhere between the sixth and final stage. 
		     * This won't normally happen because 7 stages is the max in the UI. */
	Stage_Final, /**< The last stage. */
	Stage_Extra1, /**< The first bonus stage, AKA the extra stage. */
	Stage_Extra2, /**< The last bonus stage, AKA the encore extra stage. */
	Stage_Nonstop, /**< Playing a nonstop course. */
	Stage_Oni, /**< Playing an oni or survival course. */
	Stage_Endless, /**< Playing an endless course. */
	Stage_Event, /**< Playing in event mode. */
	Stage_Demo, /**< Playing the demonstration. */
	NUM_Stage, /**< The number of stage types. */
	Stage_Invalid,
};
const RString& StageToString( Stage s );
LuaDeclareType( Stage );
const RString& StageToLocalizedString( Stage i );

/** @brief The different possibilities of earning an extra stage. */
enum EarnedExtraStage
{
	EarnedExtraStage_No, /**< No extra stage was earned. */
	EarnedExtraStage_Extra1, /**< The first extra stage was earned. */
	EarnedExtraStage_Extra2, /**< The second extra stage (or encore extra stage) was earned. */
	NUM_EarnedExtraStage,
	EarnedExtraStage_Invalid
};
const RString& EarnedExtraStageToString( EarnedExtraStage s );
LuaDeclareType( EarnedExtraStage );

/** @brief The different results of loading a profile. */
enum ProfileLoadResult
{
	ProfileLoadResult_Success, 
	ProfileLoadResult_FailedNoProfile, 
	ProfileLoadResult_FailedTampered
};

/** @brief The different statuses for multiplayer. */
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

/** @brief The different course types. */
enum CourseType
{
	COURSE_TYPE_NONSTOP,	/**< The life meter type is set to BAR. */
	COURSE_TYPE_ONI,	/**< The life meter type is set to BATTERY. */
	COURSE_TYPE_ENDLESS,	/**< The life meter type is set to REPEAT. */
	COURSE_TYPE_SURVIVAL,	/**< The life meter type is set to TIME. */
	NUM_CourseType,
	CourseType_Invalid
};
/** @brief A special iterator for handling the CourseTypes. */
#define FOREACH_CourseType( i ) FOREACH_ENUM( CourseType, i )
const RString& CourseTypeToString( CourseType i );
const RString& CourseTypeToLocalizedString( CourseType i );
LuaDeclareType( CourseType );

/** @brief How can the Player fail a song? */
enum FailType
{
	FailType_Immediate,		/**< fail immediately when life touches 0 */
	FailType_ImmediateContinue,	/**< Same as above, but allow playing the rest of the song */
	FailType_EndOfSong,			/**< fail if life is at 0 when the song ends */
	FailType_Off,			/**< never fail */
	NUM_FailType,
	FailType_Invalid
};

const RString& FailTypeToString( FailType cat );
const RString& FailTypeToLocalizedString( FailType cat );
LuaDeclareType( FailType );


#endif

/**
 * @file
 * @author Chris Danford, Chris Gomez (c) 2001-2004
 * @section LICENSE
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
