#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "Attack.h"
#include "Difficulty.h"
#include "GameConstantsAndTypes.h"
#include "Grade.h"
#include "MessageManager.h"
#include "ModsGroup.h"
#include "RageTimer.h"
#include "PlayerOptions.h"
#include "SongOptions.h"
#include "SongPosition.h"
#include "Preference.h"

#include <map>
#include <deque>
#include <set>

class Character;
class Course;
struct Game;
struct lua_State;
class LuaTable;
class PlayerState;
class PlayerOptions;
class Profile;
class Song;
class Steps;
class StageStats;
class Style;
class TimingData;
class Trail;

SortOrder GetDefaultSort();

/** @brief Holds game data that is not saved between sessions. */
class GameState
{
	/** @brief The player number used with Styles where one player controls both sides. */
	PlayerNumber	masterPlayerNumber;
	/** @brief The TimingData that is used for processing certain functions. */
	TimingData * processedTiming;
public:
	/** @brief Set up the GameState with initial values. */
	GameState();
	~GameState();
	/** @brief Reset the GameState back to initial values. */
	void Reset();
	void ResetPlayer( PlayerNumber pn );
	void ResetPlayerOptions( PlayerNumber pn );
	void ApplyCmdline(); // called by Reset
	void ApplyGameCommand( const RString &sCommand, PlayerNumber pn=PLAYER_INVALID );
	/** @brief Start the game when the first player joins in. */
	void BeginGame();
	void JoinPlayer( PlayerNumber pn );
	void UnjoinPlayer( PlayerNumber pn );
	bool JoinInput( PlayerNumber pn );
	bool JoinPlayers();
	void LoadProfiles( bool bLoadEdits = true );
	void SavePlayerProfiles();
	void SavePlayerProfile( PlayerNumber pn );
	bool HaveProfileToLoad();
	bool HaveProfileToSave();
	void SaveLocalData();
	void AddStageToPlayer( PlayerNumber pn );
	void LoadCurrentSettingsFromProfile( PlayerNumber pn );
	/**
	 * @brief Save the specified player's settings to his/her profile.
	 *
	 * This is called at the beginning of each stage.
	 * @param pn the PlayerNumber to save the stats to. */
	void SaveCurrentSettingsToProfile( PlayerNumber pn );
	Song* GetDefaultSong() const;

	bool CanSafelyEnterGameplay(RString& reason);
	void SetCompatibleStylesForPlayers();
	void ForceSharedSidesMatch();
	void ForceOtherPlayersToCompatibleSteps(PlayerNumber main);

	void Update( float fDelta );

	// Main state info

	/**
	 * @brief State what the current game is.
	 *
	 * Call this instead of m_pCurGame.Set to make sure that
	 * PREFSMAN->m_sCurrentGame stays in sync.
	 * @param pGame the game to start using. */
	void SetCurGame( const Game *pGame );
	BroadcastOnChangePtr<const Game>	m_pCurGame;
	private: // DO NOT access directly.  Use Get/SetCurrentStyle.
	BroadcastOnChangePtr<const Style>	m_pCurStyle;
	// Only used if the Game specifies that styles are separate.
	Style const* m_SeparatedStyles[NUM_PlayerNumber];
	public:
	/** @brief Determine which side is joined.
	 *
	 * The left side is player 1, and the right side is player 2. */
	bool					m_bSideIsJoined[NUM_PLAYERS];	// left side, right side
	MultiPlayerStatus			m_MultiPlayerStatus[NUM_MultiPlayer];
	BroadcastOnChange<PlayMode>		m_PlayMode;			// many screens display different info depending on this value
	/**
	 * @brief The number of coins presently in the machine.
	 *
	 * Note that coins are not "credits". One may have to put in two coins 
	 * to get one credit, only to have to put in another four coins to get
	 * the three credits needed to begin the game. */
	BroadcastOnChange<int>			m_iCoins;
	bool			m_bMultiplayer;
	int				m_iNumMultiplayerNoteFields;
	bool DifficultiesLocked() const;
	bool ChangePreferredDifficultyAndStepsType( PlayerNumber pn, Difficulty dc, StepsType st );
	bool ChangePreferredDifficulty( PlayerNumber pn, int dir );
	bool ChangePreferredCourseDifficultyAndStepsType( PlayerNumber pn, CourseDifficulty cd, StepsType st );
	bool ChangePreferredCourseDifficulty( PlayerNumber pn, int dir );
	bool IsCourseDifficultyShown( CourseDifficulty cd );
	Difficulty GetClosestShownDifficulty( PlayerNumber pn ) const;
	Difficulty GetEasiestStepsDifficulty() const;
	Difficulty GetHardestStepsDifficulty() const;
	RageTimer		m_timeGameStarted;	// from the moment the first player pressed Start
	LuaTable		*m_Environment;

	// This is set to a random number per-game/round; it can be used for a random seed.
	int			m_iGameSeed, m_iStageSeed;
	RString		m_sStageGUID;

	void SetNewStageSeed();

	/**
	 * @brief Determine if a second player can join in at this time.
	 * @return true if a player can still enter the game, false otherwise. */
	bool	PlayersCanJoin() const;
	int 	GetCoinsNeededToJoin() const;
	bool	EnoughCreditsToJoin() const { return m_iCoins >= GetCoinsNeededToJoin(); }
	int		GetNumSidesJoined() const;

	const Game*	GetCurrentGame() const;
	const Style*	GetCurrentStyle(PlayerNumber pn) const;
	void	SetCurrentStyle(const Style *style, PlayerNumber pn);
	bool SetCompatibleStyle(StepsType stype, PlayerNumber pn);

	void GetPlayerInfo( PlayerNumber pn, bool& bIsEnabledOut, bool& bIsHumanOut );
	bool IsPlayerEnabled( PlayerNumber pn ) const;
	bool IsMultiPlayerEnabled( MultiPlayer mp ) const;
	bool IsPlayerEnabled( const PlayerState* pPlayerState ) const;
	int	GetNumPlayersEnabled() const;

	/**
	 * @brief Is the specified Player a human Player?
	 * @param pn the numbered Player to check.
	 * @return true if it's a human Player, or false otherwise. */
	bool IsHumanPlayer( PlayerNumber pn ) const;
	int GetNumHumanPlayers() const;
	PlayerNumber GetFirstHumanPlayer() const;
	PlayerNumber GetFirstDisabledPlayer() const;
	bool IsCpuPlayer( PlayerNumber pn ) const;
	bool AnyPlayersAreCpu() const;

	/**
	 * @brief Retrieve the present master player number.
	 * @return The master player number. */
	PlayerNumber GetMasterPlayerNumber() const;

	/**
	 * @brief Set the master player number.
	 * @param p the master player number. */
	void SetMasterPlayerNumber(const PlayerNumber p);

	/**
	 * @brief Retrieve the present timing data being processed.
	 * @return the timing data pointer. */
	TimingData * GetProcessedTimingData() const;

	/**
	 * @brief Set the timing data to be used with processing.
	 * @param t the timing data. */
	void SetProcessedTimingData(TimingData * t);

	bool IsCourseMode() const;
	bool IsBattleMode() const; // not Rave

	/**
	 * @brief Do we show the W1 timing judgment?
	 * @return true if we do, or false otherwise. */
	bool ShowW1() const;

	BroadcastOnChange<RString>	m_sPreferredSongGroup;		// GROUP_ALL denotes no preferred group
	BroadcastOnChange<RString>	m_sPreferredCourseGroup;	// GROUP_ALL denotes no preferred group
	bool		m_bFailTypeWasExplicitlySet;	// true if FailType was changed in the song options screen
	BroadcastOnChange<StepsType>				m_PreferredStepsType;
	BroadcastOnChange1D<Difficulty,NUM_PLAYERS>		m_PreferredDifficulty;
	BroadcastOnChange1D<CourseDifficulty,NUM_PLAYERS>	m_PreferredCourseDifficulty;// used in nonstop
	BroadcastOnChange<SortOrder>	m_SortOrder;			// set by MusicWheel
	SortOrder	m_PreferredSortOrder;		// used by MusicWheel
	EditMode	m_EditMode;
	bool		IsEditing() const { return m_EditMode != EditMode_Invalid; }
	/**
	 * @brief Are we in the demonstration or jukebox mode?
	 *
	 * ScreenGameplay often does special things when this is set to true. */
	bool		m_bDemonstrationOrJukebox;
	bool		m_bJukeboxUsesModifiers;
	int			m_iNumStagesOfThisSong;
	/**
	 * @brief Increase this every stage while not resetting on a continue.
	 *
	 * This is cosmetic: it's not use for Stage or Screen branching logic. */
	int				m_iCurrentStageIndex;
	/**
	 * @brief The number of stages available for the players.
	 *
	 * This resets whenever a player joins or continues. */
	int				m_iPlayerStageTokens[NUM_PLAYERS];
	// This is necessary so that IsFinalStageForEveryHumanPlayer knows to
	// adjust for the current song cost.
	bool m_AdjustTokensBySongCostForFinalStageCheck;

	RString sExpandedSectionName;

	static int GetNumStagesMultiplierForSong( const Song* pSong );
	static int GetNumStagesForSongAndStyleType( const Song* pSong, StyleType st );
	int GetNumStagesForCurrentSongAndStepsOrCourse() const;

	void		BeginStage();
	void		CancelStage();
	void		CommitStageStats();
	void		FinishStage();
	int			GetNumStagesLeft( PlayerNumber pn ) const;
	int			GetSmallestNumStagesLeftForAnyHumanPlayer() const;
	bool		IsFinalStageForAnyHumanPlayer() const;
	bool		IsFinalStageForEveryHumanPlayer() const;
	bool		IsAnExtraStage() const;
	bool		IsAnExtraStageAndSelectionLocked() const;
	bool		IsExtraStage() const;
	bool		IsExtraStage2() const;
	Stage		GetCurrentStage() const;
	int		GetCourseSongIndex() const;
	RString		GetPlayerDisplayName( PlayerNumber pn ) const;

	bool		m_bLoadingNextSong;
	int		GetLoadingCourseSongIndex() const;

	int prepare_song_for_gameplay();

	// State Info used during gameplay

	// nullptr on ScreenSelectMusic if the currently selected wheel item isn't a Song.
	BroadcastOnChangePtr<Song>	m_pCurSong;
	// The last Song that the user manually changed to.
	Song*		m_pPreferredSong;
	BroadcastOnChangePtr1D<Steps,NUM_PLAYERS> m_pCurSteps;

	// nullptr on ScreenSelectMusic if the currently selected wheel item isn't a Course.
	BroadcastOnChangePtr<Course>	m_pCurCourse;
	// The last Course that the user manually changed to.
	Course*		m_pPreferredCourse;
	BroadcastOnChangePtr1D<Trail,NUM_PLAYERS>	m_pCurTrail;

	bool		m_bBackedOutOfFinalStage;

	// Music statistics:
	SongPosition m_Position;

	BroadcastOnChange<bool> m_bGameplayLeadIn;

	// if re-adding noteskin changes in courses, add functions and such here -aj
	void GetAllUsedNoteSkins( vector<RString> &out ) const;

	static const float MUSIC_SECONDS_INVALID;

	void ResetMusicStatistics();	// Call this when it's time to play a new song.  Clears the values above.
	void SetPaused(bool p) { m_paused= p; }
	bool GetPaused() { return m_paused; }
	void UpdateSongPosition( float fPositionSeconds, const TimingData &timing, const RageTimer &timestamp = RageZeroTimer );
	float GetSongPercent( float beat ) const;

	bool AllAreInDangerOrWorse() const;
	bool OneIsHot() const;

	// Haste
	float	m_fHasteRate; // [-1,+1]; 0 = normal speed
	float	m_fLastHasteUpdateMusicSeconds;
	float	m_fAccumulatedHasteSeconds;

	// used by themes that support heart rate entry.
	RageTimer m_DanceStartTime;
	float m_DanceDuration;

	// Random Attacks & Attack Mines
	vector<RString>		m_RandomAttacks;

	// used in PLAY_MODE_BATTLE
	float	m_fOpponentHealthPercent;

	// used in PLAY_MODE_RAVE
	float	m_fTugLifePercentP1;

	// used in workout
	bool	m_bGoalComplete[NUM_PLAYERS];
	bool	m_bWorkoutGoalComplete;

	/** @brief Primarily called at the end of a song to stop all attacks. */
	void RemoveAllActiveAttacks();
	PlayerNumber GetBestPlayer() const;
	StageResult GetStageResult( PlayerNumber pn ) const;

	/** @brief Call this function when it's time to play a new stage. */
	void ResetStageStatistics();

	// Options stuff
	ModsGroup<SongOptions>	m_SongOptions;

	/**
	 * @brief Did the current game mode change the default Noteskin?
	 *
	 * This is true if it has: see Edit/Sync Songs for a common example.
	 * Note: any mode that wants to use this must set it explicitly. */
	bool m_bDidModeChangeNoteSkin;

	void GetDefaultPlayerOptions( PlayerOptions &po );
	void GetDefaultSongOptions( SongOptions &so );
	void ResetToDefaultSongOptions( ModsLevel l );
	void ApplyPreferredModifiers( PlayerNumber pn, RString sModifiers );
	void ApplyStageModifiers( PlayerNumber pn, RString sModifiers );
	void ClearStageModifiersIllegalForCourse();
	void ResetOptions();

	bool CurrentOptionsDisqualifyPlayer( PlayerNumber pn );
	bool PlayerIsUsingModifier( PlayerNumber pn, const RString &sModifier );

	FailType GetPlayerFailType( const PlayerState *pPlayerState ) const;

	// character stuff
	Character* m_pCurCharacters[NUM_PLAYERS];

	bool HasEarnedExtraStage() const { return m_bEarnedExtraStage; }
	EarnedExtraStage GetEarnedExtraStage() const { return CalculateEarnedExtraStage(); }

	// Ranking Stuff
	struct RankingFeat
	{
		enum { SONG, COURSE, CATEGORY } Type;
		Song* pSong;		// valid if Type == SONG
		Steps* pSteps;		// valid if Type == SONG
		Course* pCourse;	// valid if Type == COURSE
		Grade grade;
		int iScore;
		float fPercentDP;
		RString Banner;
		RString Feat;
		RString *pStringToFill;
	};

	void GetRankingFeats( PlayerNumber pn, vector<RankingFeat> &vFeatsOut ) const;
	bool AnyPlayerHasRankingFeats() const;
	void StoreRankingName( PlayerNumber pn, RString name );	// Called by name entry screens
	vector<RString*> m_vpsNamesThatWereFilled;	// filled on StoreRankingName, 

	// Award stuff
	// lowest priority in front, highest priority at the back.
	deque<StageAward> m_vLastStageAwards[NUM_PLAYERS];
	deque<PeakComboAward> m_vLastPeakComboAwards[NUM_PLAYERS];

	// Attract stuff
	int m_iNumTimesThroughAttract;	// negative means play regardless of m_iAttractSoundFrequency setting
	bool IsTimeToPlayAttractSounds() const;
	void VisitAttractScreen( const RString sScreenName );

	// PlayerState
	/** @brief Allow access to each player's PlayerState. */
	PlayerState* m_pPlayerState[NUM_PLAYERS];
	PlayerState* m_pMultiPlayerState[NUM_MultiPlayer];

	// Preferences
	static Preference<bool> m_bAutoJoin;

	// These options have weird interactions depending on m_bEventMode, 
	// so wrap them.
	bool		m_bTemporaryEventMode;
	bool		IsEventMode() const;
	CoinMode	GetCoinMode() const;
	Premium		GetPremium() const;

	// Edit stuff
	
	/**
	 * @brief Is the game right now using Song timing or Steps timing?
	 *
	 * Different options are available depending on this setting. */
	bool m_bIsUsingStepTiming;
	/**
	 * @brief Are we presently in the Step Editor, where some rules apply differently?
	 *
	 * TODO: Find a better way to implement this. */
	bool m_bInStepEditor;
	BroadcastOnChange<StepsType> m_stEdit;
	BroadcastOnChange<CourseDifficulty> m_cdEdit;
	BroadcastOnChangePtr<Steps> m_pEditSourceSteps;
	BroadcastOnChange<StepsType> m_stEditSource;
	BroadcastOnChange<int> m_iEditCourseEntryIndex;
	BroadcastOnChange<RString> m_sEditLocalProfileID;
	Profile* GetEditLocalProfile();

	// Workout stuff
	float GetGoalPercentComplete( PlayerNumber pn );
	bool IsGoalComplete( PlayerNumber pn )	{ return GetGoalPercentComplete( pn ) >= 1; }

	bool m_bDopefish;

	// Autogen stuff.  This should probably be moved to its own singleton or
	// something when autogen is generalized and more customizable. -Kyz
	float GetAutoGenFarg(size_t i)
	{
		if(i >= m_autogen_fargs.size()) { return 0.0f; }
		return m_autogen_fargs[i];
	}
	vector<float> m_autogen_fargs;

	// Lua
	void PushSelf( lua_State *L );

	// Keep extra stage logic internal to GameState.
private:
	EarnedExtraStage	CalculateEarnedExtraStage() const;
	int	m_iAwardedExtraStages[NUM_PLAYERS];
	bool	m_bEarnedExtraStage;

	// Timing position corrections
	RageTimer m_LastPositionTimer;
	float m_LastPositionSeconds;
	bool m_paused;

	GameState(const GameState& rhs);
	GameState& operator=(const GameState& rhs);

};

PlayerNumber GetNextHumanPlayer( PlayerNumber pn );
PlayerNumber GetNextEnabledPlayer( PlayerNumber pn );
PlayerNumber GetNextCpuPlayer( PlayerNumber pn );
PlayerNumber GetNextPotentialCpuPlayer( PlayerNumber pn );
MultiPlayer GetNextEnabledMultiPlayer( MultiPlayer mp );

/** @brief A foreach loop to act on each human Player. */
#define FOREACH_HumanPlayer( pn ) for( PlayerNumber pn=GetNextHumanPlayer((PlayerNumber)-1); pn!=PLAYER_INVALID; pn=GetNextHumanPlayer(pn) )
/** @brief A foreach loop to act on each enabled Player. */
#define FOREACH_EnabledPlayer( pn ) for( PlayerNumber pn=GetNextEnabledPlayer((PlayerNumber)-1); pn!=PLAYER_INVALID; pn=GetNextEnabledPlayer(pn) )
/** @brief A foreach loop to act on each CPU Player. */
#define FOREACH_CpuPlayer( pn ) for( PlayerNumber pn=GetNextCpuPlayer((PlayerNumber)-1); pn!=PLAYER_INVALID; pn=GetNextCpuPlayer(pn) )
/** @brief A foreach loop to act on each potential CPU Player. */
#define FOREACH_PotentialCpuPlayer( pn ) for( PlayerNumber pn=GetNextPotentialCpuPlayer((PlayerNumber)-1); pn!=PLAYER_INVALID; pn=GetNextPotentialCpuPlayer(pn) )
/** @brief A foreach loop to act on each Player in MultiPlayer. */
#define FOREACH_EnabledMultiPlayer( mp ) for( MultiPlayer mp=GetNextEnabledMultiPlayer((MultiPlayer)-1); mp!=MultiPlayer_Invalid; mp=GetNextEnabledMultiPlayer(mp) )



extern GameState*	GAMESTATE;	// global and accessible from anywhere in our program

#endif

/**
 * @file
 * @author Chris Danford, Glenn Maynard, Chris Gomez (c) 2001-2004
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
