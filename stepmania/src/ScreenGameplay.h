/* ScreenGameplay - The music plays, the notes scroll, and the Player is pressing buttons. */

#ifndef SCREEN_GAMEPLAY_H
#define SCREEN_GAMEPLAY_H

#include "Screen.h"
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
#include "MeterDisplay.h"
#include "ActiveAttackList.h"
#include "NetworkSyncManager.h"

// messages sent by Combo
const ScreenMessage SM_PlayToasty			= ScreenMessage(SM_User+104);

const ScreenMessage	SM_100Combo					= ScreenMessage(SM_User+200);
const ScreenMessage	SM_200Combo					= ScreenMessage(SM_User+201);
const ScreenMessage	SM_300Combo					= ScreenMessage(SM_User+202);
const ScreenMessage	SM_400Combo					= ScreenMessage(SM_User+203);
const ScreenMessage	SM_500Combo					= ScreenMessage(SM_User+204);
const ScreenMessage	SM_600Combo					= ScreenMessage(SM_User+205);
const ScreenMessage	SM_700Combo					= ScreenMessage(SM_User+206);
const ScreenMessage	SM_800Combo					= ScreenMessage(SM_User+207);
const ScreenMessage	SM_900Combo					= ScreenMessage(SM_User+208);
const ScreenMessage	SM_1000Combo				= ScreenMessage(SM_User+209);
const ScreenMessage	SM_ComboStopped				= ScreenMessage(SM_User+210);
const ScreenMessage SM_ComboContinuing			= ScreenMessage(SM_User+211);
const ScreenMessage	SM_MissComboAborted			= ScreenMessage(SM_User+212);

const ScreenMessage SM_BattleTrickLevel1		= ScreenMessage(SM_User+301);
const ScreenMessage SM_BattleTrickLevel2		= ScreenMessage(SM_User+302);
const ScreenMessage SM_BattleTrickLevel3		= ScreenMessage(SM_User+303);
const ScreenMessage SM_BattleDamageLevel1		= ScreenMessage(SM_User+304);
const ScreenMessage SM_BattleDamageLevel2		= ScreenMessage(SM_User+305);
const ScreenMessage SM_BattleDamageLevel3		= ScreenMessage(SM_User+306);

class LyricsLoader;
class ScreenGameplay : public Screen
{
public:
	ScreenGameplay( CString sName, bool bDemonstration = false );
	void Init();
	virtual ~ScreenGameplay();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );


protected:
	void TweenOnScreen();
	void TweenOffScreen();

	bool IsLastSong();
	void SetupSong( PlayerNumber p, int iSongIndex );
	void LoadNextSong();
	void LoadCourseSongNumber( int SongNumber );
	float StartPlayingSong(float MinTimeToNotes, float MinTimeToMusic);
	void ShowSavePrompt( ScreenMessage SM_SendWhenDone );
	void PlayAnnouncer( CString type, float fSeconds );

	void PlayTicks();
	void UpdateSongPosition( float fDeltaTime );
	void UpdateLyrics( float fDeltaTime );
	void SongFinished();
	void StageFinished( bool bBackedOut );

	enum DancingState { 
		STATE_INTRO = 0, // not allowed to press Back
		STATE_DANCING,
		STATE_OUTRO,	// not allowed to press Back
		NUM_DANCING_STATES
	} m_DancingState;
	vector<Song*>		m_apSongsQueue;					// size may be >1 if playing a course
	vector<Steps*>		m_vpStepsQueue[NUM_PLAYERS];	// size may be >1 if playing a course
	vector<AttackArray>	m_asModifiersQueue[NUM_PLAYERS];// size may be >1 if playing a course

	bool				m_bChangedOffsetOrBPM;
	float				m_fTimeSinceLastDancingComment;	// this counter is only running while STATE_DANCING

	LyricDisplay		m_LyricDisplay;

	Background			m_Background;
	Foreground			m_Foreground;

	Transition	m_NextSongIn;	// shows between songs in a course
	Transition	m_NextSongOut;	// shows between songs in a course
	Transition	m_SongFinished;	// shows after each song, course or not

	Sprite				m_sprStaticBackground;
	Sprite				m_sprLifeFrame;
	LifeMeter*			m_pLifeMeter[NUM_PLAYERS];
	CombinedLifeMeter*	m_pCombinedLifeMeter;
	Sprite				m_sprStage;
	Sprite				m_sprCourseSongNumber;
	AutoActor			m_sprStageFrame;
	BitmapText			m_textCourseSongNumber[NUM_PLAYERS];
	BitmapText			m_textPlayerName[NUM_PLAYERS];
	BitmapText			m_textStepsDescription[NUM_PLAYERS];

	BPMDisplay			m_BPMDisplay;

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
	void AbortGiveUp();

	BitmapText			m_textAutoPlay;	// for AutoPlay, AutoAdjust
	void	UpdateAutoPlayText();
	
	BitmapText			m_MaxCombo;

	Transition	m_Ready;
	Transition	m_Go;
	Transition	m_Cleared;
	Transition	m_Failed;
	Transition	m_Extra;
	Transition	m_Toasty;	// easter egg
	Transition	m_Win[NUM_PLAYERS];
	Transition	m_Draw;
	Transition	m_In;
	Transition	m_Back;
	BGAnimation	m_Overlay;

	BitmapText			m_textSurviveTime;	// used in extra stage
	BitmapText			m_textSongTitle;
	MeterDisplay		m_meterSongPosition;

	Player				m_Player[NUM_PLAYERS];

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
	bool			m_bDemonstration;

	RageSound		m_soundAssistTick;
	RageSound		m_soundMusic;

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
