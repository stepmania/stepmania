/* ScreenGameplay - The music plays, the notes scroll, and the Player is pressing buttons. */

#ifndef SCREEN_GAMEPLAY_H
#define SCREEN_GAMEPLAY_H

#include "ScreenWithMenuElements.h"
#include "Sprite.h"
#include "Transition.h"
#include "BitmapText.h"
#include "Player.h"
#include "RandomSample.h"
#include "RageSound.h"
#include "Background.h"
#include "Foreground.h"
#include "LifeMeter.h"
#include "ScoreDisplay.h"
#include "DifficultyIcon.h"
#include "DifficultyMeter.h"
#include "BPMDisplay.h"
class Inventory;
#include "BeginnerHelper.h"
#include "LyricDisplay.h"
#include "Character.h"
#include "Attack.h"
#include "ActiveAttackList.h"
#include "NetworkSyncManager.h"
#include "AutoKeysounds.h"
#include "ThemeMetric.h"

AutoScreenMessage( SM_NotesEnded )

class LyricsLoader;
class ScreenGameplay : public ScreenWithMenuElements
{
public:
	ScreenGameplay( CString sName );
	virtual void Init();
	virtual ~ScreenGameplay();

	virtual void Update( float fDeltaTime );
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual bool UsesBackground() const { return false; }
	virtual ScreenType GetScreenType() const { return gameplay; }

	//
	// Lua
	//
	virtual void PushSelf( lua_State *L );
	Song *GetNextCourseSong() const;

protected:
	ThemeMetric<CString> PLAYER_TYPE;
	ThemeMetric<CString> GIVE_UP_TEXT;
	ThemeMetric<CString> GIVE_UP_ABORTED_TEXT;
	ThemeMetric<float> MUSIC_FADE_OUT_SECONDS;
	ThemeMetric<bool> START_GIVES_UP;
	ThemeMetric<bool> BACK_GIVES_UP;
	ThemeMetric<bool> GIVING_UP_GOES_TO_PREV_SCREEN;
	ThemeMetric<bool> GIVING_UP_GOES_TO_NEXT_SCREEN;
	ThemeMetric<bool> FAIL_AFTER_30_MISSES;
	ThemeMetric<bool> USE_FORCED_MODIFIERS_IN_BEGINNER;
	ThemeMetric<CString> FORCED_MODIFIERS_IN_BEGINNER;

	void TweenOursOnScreen();
	void TweenOursOffScreen();

	bool IsLastSong();
	void SetupSong( int iSongIndex );
	void LoadNextSong();
	void LoadCourseSongNumber( int SongNumber );
	float StartPlayingSong(float MinTimeToNotes, float MinTimeToMusic);
	void LoadLights();
	void PauseGame( bool bPause, GameController gc = GAME_CONTROLLER_INVALID );
	void PlayAnnouncer( CString type, float fSeconds );
	void UpdateLights();
	void SendCrossedMessages();
	void BackOutFromGameplay();

	void PlayTicks();
	void UpdateSongPosition( float fDeltaTime );
	void UpdateLyrics( float fDeltaTime );
	void SongFinished();
	void StageFinished( bool bBackedOut );

	virtual void InitSongQueues();

	enum DancingState { 
		STATE_INTRO = 0, // not allowed to press Back
		STATE_DANCING,
		STATE_OUTRO,	// not allowed to press Back
		NUM_DANCING_STATES
	} m_DancingState;
	bool				m_bPaused;
	GameController			m_PauseController;
	vector<Song*>		m_apSongsQueue;					// size may be >1 if playing a course
	vector<Steps*>		m_vpStepsQueue[NUM_PLAYERS];	// size may be >1 if playing a course
	vector<AttackArray>	m_asModifiersQueue[NUM_PLAYERS];// size may be >1 if playing a course

	float				m_fTimeSinceLastDancingComment;	// this counter is only running while STATE_DANCING

	LyricDisplay		m_LyricDisplay;

	Background			m_SongBackground;
	Foreground			m_SongForeground;

	Transition	m_NextSong;	// shows between songs in a course
	Transition	m_SongFinished;	// shows after each song, course or not

	Sprite				m_sprStaticBackground;
	AutoActor			m_sprLifeFrame;
	LifeMeter*			m_pLifeMeter[NUM_PLAYERS];
	CombinedLifeMeter*	m_pCombinedLifeMeter;
	Sprite				m_sprCourseSongNumber;
	AutoActor			m_sprStageFrame;
	BitmapText			m_textCourseSongNumber[NUM_PLAYERS];
	BitmapText			m_textStepsDescription[NUM_PLAYERS];

	BPMDisplay			m_BPMDisplay;
	float				m_fLastBPS;

	Sprite				m_sprScoreFrame;
	ScoreDisplay*		m_pPrimaryScoreDisplay[NUM_PLAYERS];
	ScoreDisplay*		m_pSecondaryScoreDisplay[NUM_PLAYERS];
	ScoreKeeper*		m_pPrimaryScoreKeeper[NUM_PLAYERS];
	ScoreKeeper*		m_pSecondaryScoreKeeper[NUM_PLAYERS];
	BitmapText			m_textPlayerOptions[NUM_PLAYERS];
	BitmapText			m_textSongOptions;
	ActiveAttackList	m_ActiveAttackList[NUM_PLAYERS];
	BitmapText			m_Scoreboard[NUM_NSSB_CATEGORIES];	// for NSMAN, so we can have a scoreboard

	bool				m_ShowScoreboard;

	BitmapText			m_textDebug;

	RageTimer			m_GiveUpTimer;
	void AbortGiveUp( bool bShowText );

	BitmapText			m_MaxCombo;

	Transition	m_Ready;
	Transition	m_Go;
	Transition	m_Cleared;
	Transition	m_Failed;
	Transition	m_Extra;
	Transition	m_Toasty;	// easter egg
	Transition	m_Win[NUM_PLAYERS];
	Transition	m_Draw;

	BitmapText			m_textSurviveTime;	// used in extra stage

	Player				m_Player[NUM_PLAYERS];

	AutoKeysounds	m_AutoKeysounds;

	// used in PLAY_MODE_BATTLE
	Inventory*			m_pInventory[NUM_PLAYERS];
	
	DifficultyIcon		m_DifficultyIcon[NUM_PLAYERS];
	DifficultyMeter		m_DifficultyMeter[NUM_PLAYERS];

	Sprite				m_sprOniGameOver[NUM_PLAYERS];
	void				ShowOniGameOver( PlayerNumber pn );

	RageSound		m_soundBattleTrickLevel1;
	RageSound		m_soundBattleTrickLevel2;
	RageSound		m_soundBattleTrickLevel3;

	bool			m_bZeroDeltaOnNextUpdate;

	RageSound		m_soundAssistTick;
	RageSound		*m_pSoundMusic;

	BeginnerHelper	m_BeginnerHelper;

	NoteData		m_CabinetLightsNoteData;
};


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
