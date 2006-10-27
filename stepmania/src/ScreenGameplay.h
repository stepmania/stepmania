/* ScreenGameplay - The music plays, the notes scroll, and the Player is pressing buttons. */

#ifndef SCREEN_GAMEPLAY_H
#define SCREEN_GAMEPLAY_H

#include "ScreenWithMenuElements.h"
#include "Sprite.h"
#include "Transition.h"
#include "BitmapText.h"
#include "RageSound.h"
#include "BPMDisplay.h"
#include "BeginnerHelper.h"
#include "LyricDisplay.h"
#include "Attack.h"
#include "NetworkSyncManager.h"
#include "AutoKeysounds.h"
#include "ThemeMetric.h"
#include "PlayerStageStats.h"
#include "PlayerState.h"
#include "InputEventPlus.h"

class LyricsLoader;
class ActiveAttackList;
class CombinedLifeMeter;
class Player;
class LifeMeter;
class ScoreDisplay;
class DifficultyIcon;
class DifficultyMeter;
class Inventory;
class ScoreKeeper;
class Background;
class Foreground;

AutoScreenMessage( SM_NotesEnded )
AutoScreenMessage( SM_BeginFailed )
AutoScreenMessage( SM_LeaveGameplay )

class PlayerInfo
{
public:
	PlayerInfo();
	~PlayerInfo();

	void Load( PlayerNumber pn, MultiPlayer mp, bool bShowNoteField );
	void LoadDummyP1();

	void ShowOniGameOver();
	MultiPlayer GetPlayerStateAndStageStatsIndex()	{ return m_pn == PLAYER_INVALID ? m_mp : (MultiPlayer)m_pn; }
	PlayerState *GetPlayerState();
	PlayerStageStats *GetPlayerStageStats();
	PlayerNumber GetStepsAndTrailIndex()		{ return m_pn == PLAYER_INVALID ? PLAYER_1 : m_pn; }
	bool IsEnabled();
	bool IsMultiPlayer() const { return m_mp != MultiPlayer_Invalid; }
	RString GetName() const
	{
		if( m_bIsDummy )
			return "Dummy";
		if( IsMultiPlayer() ) 
			return MultiPlayerToString( m_mp );
		else
			return PlayerNumberToString( m_pn );
	}

	PlayerNumber		m_pn;
	MultiPlayer		m_mp;
	bool			m_bIsDummy;
	PlayerState		m_PlayerStateDummy;
	PlayerStageStats	m_PlayerStageStatsDummy;

	vector<Steps*>		m_vpStepsQueue;	// size may be >1 if playing a course
	vector<AttackArray>	m_asModifiersQueue;// size may be >1 if playing a course

	LifeMeter		*m_pLifeMeter;
	BitmapText		*m_ptextCourseSongNumber;
	BitmapText		*m_ptextStepsDescription;

	ScoreDisplay		*m_pPrimaryScoreDisplay;
	ScoreDisplay		*m_pSecondaryScoreDisplay;
	ScoreKeeper		*m_pPrimaryScoreKeeper;
	ScoreKeeper		*m_pSecondaryScoreKeeper;
	BitmapText		*m_ptextPlayerOptions;
	ActiveAttackList	*m_pActiveAttackList;

	NoteData		m_NoteData;
	Player			*m_pPlayer;

	// used in PLAY_MODE_BATTLE
	Inventory		*m_pInventory;
	
	DifficultyIcon		*m_pDifficultyIcon;
	DifficultyMeter		*m_pDifficultyMeter;

	AutoActor		m_sprOniGameOver;
};

class ScreenGameplay : public ScreenWithMenuElements
{
public:
	ScreenGameplay();
	virtual void Init();
	virtual ~ScreenGameplay();
	virtual void BeginScreen();

	virtual void Update( float fDeltaTime );
	virtual void Input( const InputEventPlus &input );
	virtual void HandleScreenMessage( const ScreenMessage SM );
	virtual void Cancel( ScreenMessage smSendWhenDone );

	virtual ScreenType GetScreenType() const { return gameplay; }

	bool Center1Player() const;

	//
	// Lua
	//
	virtual void PushSelf( lua_State *L );
	Song *GetNextCourseSong() const;

protected:
	virtual bool GenericTweenOn() const { return true; }
	virtual bool GenericTweenOff() const { return true; }

	virtual bool UseSongBackgroundAndForeground() const { return true; }

	ThemeMetric<RString> PLAYER_TYPE;
	ThemeMetric<apActorCommands> PLAYER_INIT_COMMAND;
	LocalizedString GIVE_UP_START_TEXT;
	LocalizedString GIVE_UP_BACK_TEXT;
	LocalizedString GIVE_UP_ABORTED_TEXT;
	ThemeMetric<float> MUSIC_FADE_OUT_SECONDS;
	ThemeMetric<bool> START_GIVES_UP;
	ThemeMetric<bool> BACK_GIVES_UP;
	ThemeMetric<bool> GIVING_UP_GOES_TO_PREV_SCREEN;
	ThemeMetric<bool> GIVING_UP_GOES_TO_NEXT_SCREEN;
	ThemeMetric<bool> FAIL_AFTER_30_MISSES;
	ThemeMetric<bool> ALLOW_CENTER_1_PLAYER;

	bool IsLastSong();
	void SetupSong( int iSongIndex );
	void ReloadCurrentSong();
	virtual void LoadNextSong();
	void LoadCourseSongNumber( int SongNumber );
	float StartPlayingSong(float MinTimeToNotes, float MinTimeToMusic);
	void LoadLights();
	void PauseGame( bool bPause, GameController gc = GameController_Invalid );
	void PlayAnnouncer( RString type, float fSeconds );
	void UpdateLights();
	void SendCrossedMessages();
	void BeginBackingOutFromGameplay();

	void PlayTicks();
	void UpdateSongPosition( float fDeltaTime );
	void UpdateLyrics( float fDeltaTime );
	void SongFinished();
	virtual void SaveStats();
	void StageFinished( bool bBackedOut );
	void SaveReplay();

	virtual void InitSongQueues();

	enum DancingState { 
		STATE_INTRO = 0, // not allowed to press Back
		STATE_DANCING,
		STATE_OUTRO,	// not allowed to press Back
		NUM_DANCING_STATES
	} m_DancingState;
	bool			m_bPaused;

	GameController		m_PauseController;
	vector<Song*>		m_apSongsQueue;					// size may be >1 if playing a course

	float			m_fTimeSinceLastDancingComment;	// this counter is only running while STATE_DANCING

	LyricDisplay		m_LyricDisplay;

	Background		*m_pSongBackground;
	Foreground		*m_pSongForeground;

	Transition		m_NextSong;	// shows between songs in a course
	Transition		m_SongFinished;	// shows after each song, course or not

	AutoActor		m_sprLifeFrame;
	CombinedLifeMeter*	m_pCombinedLifeMeter;
	Sprite			m_sprCourseSongNumber;

	BPMDisplay		m_BPMDisplay;
	float			m_fLastBPS;

	Sprite			m_sprScoreFrame;
	BitmapText		m_textSongOptions;
	BitmapText		m_Scoreboard[NUM_NSScoreBoardColumn];	// for NSMAN, so we can have a scoreboard

	bool			m_bShowScoreboard;

	BitmapText		m_textDebug;

	RageTimer		m_GiveUpTimer;
	void AbortGiveUp( bool bShowText );

	BitmapText		m_MaxCombo;		// TODO: move this into an overlay

	Transition		m_Ready;
	Transition		m_Go;
	Transition		m_Failed;
	Transition		m_Toasty;	// easter egg

	BitmapText		m_textSurviveTime;	// used in extra stage.  TODO: Move this into a BGA


	AutoKeysounds		m_AutoKeysounds;

	RageSound		m_soundBattleTrickLevel1;
	RageSound		m_soundBattleTrickLevel2;
	RageSound		m_soundBattleTrickLevel3;

	bool			m_bZeroDeltaOnNextUpdate;

	RageSound		m_soundAssistTick;
	RageSound		*m_pSoundMusic;

	BeginnerHelper		m_BeginnerHelper;

	NoteData		m_CabinetLightsNoteData;

	vector<PlayerInfo>	m_vPlayerInfo;	// filled by SGameplay derivatives in Init
	virtual void FillPlayerInfo( vector<PlayerInfo> &vPlayerInfoOut ) = 0;
	virtual PlayerInfo &GetPlayerInfoForInput( const InputEventPlus& iep )  { return m_vPlayerInfo[iep.pn]; }

	RageTimer		m_timerGameplaySeconds;
};

vector<PlayerInfo>::iterator GetNextEnabledPlayerInfo		( vector<PlayerInfo>::iterator iter, vector<PlayerInfo> &v );
vector<PlayerInfo>::iterator GetNextEnabledPlayerInfoNotDummy	( vector<PlayerInfo>::iterator iter, vector<PlayerInfo> &v );
vector<PlayerInfo>::iterator GetNextEnabledPlayerNumberInfo	( vector<PlayerInfo>::iterator iter, vector<PlayerInfo> &v );
vector<PlayerInfo>::iterator GetNextPlayerNumberInfo		( vector<PlayerInfo>::iterator iter, vector<PlayerInfo> &v );
vector<PlayerInfo>::iterator GetNextVisiblePlayerInfo		( vector<PlayerInfo>::iterator iter, vector<PlayerInfo> &v );

#define FOREACH_EnabledPlayerInfo( v, pi )		for( vector<PlayerInfo>::iterator pi = GetNextEnabledPlayerInfo		(v.begin(),v);	pi != v.end(); pi = GetNextEnabledPlayerInfo(++pi,v) )
#define FOREACH_EnabledPlayerInfoNotDummy( v, pi )	for( vector<PlayerInfo>::iterator pi = GetNextEnabledPlayerInfoNotDummy	(v.begin(),v);	pi != v.end(); pi = GetNextEnabledPlayerInfoNotDummy(++pi,v) )
#define FOREACH_EnabledPlayerNumberInfo( v, pi )	for( vector<PlayerInfo>::iterator pi = GetNextEnabledPlayerNumberInfo	(v.begin(),v);	pi != v.end(); pi = GetNextEnabledPlayerNumberInfo(++pi,v) )
#define FOREACH_PlayerNumberInfo( v, pi )		for( vector<PlayerInfo>::iterator pi = GetNextPlayerNumberInfo		(v.begin(),v);	pi != v.end(); pi = GetNextPlayerNumberInfo(++pi,v) )
#define FOREACH_VisiblePlayerInfo( v, pi )		for( vector<PlayerInfo>::iterator pi = GetNextVisiblePlayerInfo		(v.begin(),v);	pi != v.end(); pi = GetNextVisiblePlayerInfo(++pi,v) )


#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
