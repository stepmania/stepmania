/* GameState - Holds game data that is not saved between sessions. */

#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "GameConstantsAndTypes.h"
#include "SongOptions.h"
#include "Grade.h"
#include "Attack.h"
#include "RageTimer.h"
#include "Difficulty.h"
#include "MessageManager.h"
#include "SongOptions.h"
#include "ModsGroup.h"

#include <map>
#include <deque>
#include <set>

class Song;
class Steps;
class Course;
class Trail;
class Game;
class Style;
class Character;
class TimingData;
class StageStats;
class PlayerState;
struct lua_State;
class LuaTable;
class Profile;
class PlayerOptions;

class GameState
{
public:
	GameState();
	~GameState();
	void Reset();
	void ApplyCmdline(); // called by Reset
	void ApplyGameCommand( const RString &sCommand, PlayerNumber pn=PLAYER_INVALID );
	void BeginGame();	// called when first player joins
	void JoinPlayer( PlayerNumber pn );
	void PlayersFinalized();	// called after a style is chosen, which means the number of players is finalized
	void EndGame();	// called on ScreenGameOver, ScreenMusicScroll, ScreenCredits
	void SaveCurrentSettingsToProfile( PlayerNumber pn ); // called at the beginning of each stage
	Song* GetDefaultSong() const;

	void Update( float fDelta );


	//
	// Main state info
	//
	void SetCurGame( const Game *pGame );	// Call this instead of m_pCurGame.Set to make sure PREFSMAN->m_sCurrentGame stays in sync
	BroadcastOnChangePtr<const Game>	m_pCurGame;
	BroadcastOnChangePtr<const Style>	m_pCurStyle;
	bool					m_bSideIsJoined[NUM_PLAYERS];	// left side, right side
	MultiPlayerStatus			m_MultiPlayerStatus[NUM_MultiPlayer];
	BroadcastOnChange<PlayMode>		m_PlayMode;			// many screens display different info depending on this value
	int					m_iCoins;			// not "credits"
	PlayerNumber				m_MasterPlayerNumber;		// used in Styles where one player controls both sides
	bool					m_bMultiplayer;
	bool DifficultiesLocked() const;
	bool ChangePreferredDifficulty( PlayerNumber pn, Difficulty dc );
	bool ChangePreferredDifficulty( PlayerNumber pn, int dir );
	bool ChangePreferredCourseDifficulty( PlayerNumber pn, CourseDifficulty cd );
	bool ChangePreferredCourseDifficulty( PlayerNumber pn, int dir );
	bool IsCourseDifficultyShown( CourseDifficulty cd );
	Difficulty GetClosestShownDifficulty( PlayerNumber pn ) const;
	Difficulty GetEasiestStepsDifficulty() const;
	RageTimer			m_timeGameStarted;	// from the moment the first player pressed Start
	LuaTable			*m_Environment;

	/* This is set to a random number per-game/round; it can be used for a random seed. */
	int				m_iGameSeed, m_iStageSeed;

	bool		PlayersCanJoin() const;	// true if it's not too late for a player to join
	int 		GetCoinsNeededToJoin() const;
	bool		EnoughCreditsToJoin() const { return GetCoinsNeededToJoin() <= 0; }
	int		GetNumSidesJoined() const;

	const Game*	GetCurrentGame();
	const Style*	GetCurrentStyle() const;
	void		SetCurrentStyle( const Style *pStyle );

	void GetPlayerInfo( PlayerNumber pn, bool& bIsEnabledOut, bool& bIsHumanOut );
	bool IsPlayerEnabled( PlayerNumber pn ) const;
	bool IsMultiPlayerEnabled( MultiPlayer mp ) const;
	bool IsPlayerEnabled( const PlayerState* pPlayerState ) const;
	int	GetNumPlayersEnabled() const;

	bool IsHumanPlayer( PlayerNumber pn ) const;
	int GetNumHumanPlayers() const;
	PlayerNumber GetFirstHumanPlayer() const;
	PlayerNumber GetFirstDisabledPlayer() const;
	bool IsCpuPlayer( PlayerNumber pn ) const;
	bool AnyPlayersAreCpu() const;


	bool IsCourseMode() const;
	bool IsBattleMode() const; /* not Rave */

	bool ShowW1() const;

	BroadcastOnChange<RString>	m_sPreferredSongGroup;		// GROUP_ALL denotes no preferred group
	BroadcastOnChange<RString>	m_sPreferredCourseGroup;	// GROUP_ALL denotes no preferred group
	bool				m_bChangedFailTypeOnScreenSongOptions;	// true if FailType was changed in the song options screen
	BroadcastOnChange1D<Difficulty,NUM_PLAYERS>		m_PreferredDifficulty;
	BroadcastOnChange1D<CourseDifficulty,NUM_PLAYERS>	m_PreferredCourseDifficulty;// used in nonstop
	BroadcastOnChange<SortOrder>	m_SortOrder;			// set by MusicWheel
	SortOrder			m_PreferredSortOrder;		// used by MusicWheel
	EditMode			m_EditMode;
	bool				IsEditing() const { return m_EditMode != EditMode_Invalid; }
	bool				m_bDemonstrationOrJukebox;	// ScreenGameplay does special stuff when this is true
	bool				m_bJukeboxUsesModifiers;
	int				m_iNumStagesOfThisSong;
	int				m_iCurrentStageIndex;

	int		GetStageIndex() const;
	void		BeginStage();
	void		CancelStage();
	void		CommitStageStats();
	void		FinishStage();
	int		GetNumStagesLeft() const;
	bool		IsFinalStage() const;
	bool		IsAnExtraStage() const;
	bool		IsExtraStage() const;
	bool		IsExtraStage2() const;
	Stage		GetCurrentStage() const;
	bool		IsStagePossible( Stage s ) const;
	int		GetCourseSongIndex() const;
	RString		GetPlayerDisplayName( PlayerNumber pn ) const;

	bool		m_bLoadingNextSong;
	int		GetLoadingCourseSongIndex() const;

	//
	// State Info used during gameplay
	//

	// NULL on ScreenSelectMusic if the currently selected wheel item isn't a Song.
	BroadcastOnChangePtr<Song>	m_pCurSong;
	// The last Song that the user manually changed to.
	Song*		m_pPreferredSong;
	BroadcastOnChangePtr1D<Steps,NUM_PLAYERS> m_pCurSteps;
	
	// NULL on ScreenSelectMusic if the currently selected wheel item isn't a Course.
	BroadcastOnChangePtr<Course>	m_pCurCourse;
	// The last Course that the user manually changed to.
	Course*		m_pPreferredCourse;
	BroadcastOnChangePtr1D<Trail,NUM_PLAYERS>	m_pCurTrail;

	bool        m_bBackedOutOfFinalStage;
	
	//
	// Music statistics:  Arcade: the current stage (one song).  Oni/Endles: a single song in a course
	//
	// Let a lot of classes access this info here so the don't have to keep their own copies.
	//
	float		m_fMusicSeconds;	// time into the current song
	float		m_fSongBeat;
	float		m_fSongBeatNoOffset;
	float		m_fCurBPS;
	float		m_fLightSongBeat; // g_fLightsFalloffSeconds ahead
	bool		m_bFreeze;	// in the middle of a freeze
	RageTimer	m_LastBeatUpdate; // time of last m_fSongBeat, etc. update
	BroadcastOnChange<bool> m_bGameplayLeadIn;

	void GetAllUsedNoteSkins( vector<RString> &out ) const;

	static const float MUSIC_SECONDS_INVALID;

	void ResetMusicStatistics();	// Call this when it's time to play a new song.  Clears the values above.
	void UpdateSongPosition( float fPositionSeconds, const TimingData &timing, const RageTimer &timestamp = RageZeroTimer );
	float GetSongPercent( float beat ) const;

	bool IsPlayerInDanger( const PlayerState *pPlayerState ) const;
	bool IsPlayerDead( const PlayerState *pPlayerState ) const;
	bool AllAreInDangerOrWorse() const;
	bool AllHumanHaveComboOf30OrMoreMisses() const;
	bool OneIsHot() const;

	// used in PLAY_MODE_BATTLE
	float	m_fOpponentHealthPercent;

	// used in PLAY_MODE_RAVE
	float	m_fTugLifePercentP1;

	// used in workout
	bool	m_bGoalComplete[NUM_PLAYERS];
	
	void RemoveAllActiveAttacks();	// called on end of song
	PlayerNumber GetBestPlayer() const;
	StageResult GetStageResult( PlayerNumber pn ) const;

	void ResetStageStatistics();	// Call this when it's time to play a new stage.

	// True if CommitStageStats() has been called and FinishStage() hasn't.
	bool			m_bStatsCommitted;


	//
	// Options stuff
	//

	ModsGroup<SongOptions>	m_SongOptions;

	void GetDefaultPlayerOptions( PlayerOptions &po );
	void GetDefaultSongOptions( SongOptions &so );
	void ApplyPreferredModifiers( PlayerNumber pn, RString sModifiers );
	void ApplyStageModifiers( PlayerNumber pn, RString sModifiers );
	void ResetOptions();

	bool IsDisqualified( PlayerNumber pn );
	bool PlayerIsUsingModifier( PlayerNumber pn, const RString &sModifier );

	SongOptions::FailType GetPlayerFailType( const PlayerState *pPlayerState ) const;

	// character stuff
	Character* m_pCurCharacters[NUM_PLAYERS];


	bool HasEarnedExtraStage() const;


	//
	// Ranking Stuff
	//
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


	//
	// Award stuff
	//
	// lowest priority in front, highest priority at the back.
	deque<PerDifficultyAward> m_vLastPerDifficultyAwards[NUM_PLAYERS];
	deque<PeakComboAward> m_vLastPeakComboAwards[NUM_PLAYERS];


	//
	// Attract stuff
	//
	int m_iNumTimesThroughAttract;	// negative means play regardless of m_iAttractSoundFrequency setting
	bool IsTimeToPlayAttractSounds() const;
	void VisitAttractScreen( const RString sScreenName );

	//
	// PlayerState
	//
	PlayerState* m_pPlayerState[NUM_PLAYERS];
	PlayerState* m_pMultiPlayerState[NUM_MultiPlayer];

	//
	// Preference wrappers
	//
	// These options have weird interactions depending on m_bEventMode, 
	// so wrap them
	bool		m_bTemporaryEventMode;
	bool		IsEventMode() const;
	CoinMode	GetCoinMode() const;
	Premium		GetPremium() const;
	
	//
	// Edit stuff
	//
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

	// Lua
	void PushSelf( lua_State *L );
};


extern GameState*	GAMESTATE;	// global and accessable from anywhere in our program

#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard, Chris Gomez
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
