#ifndef SCREENGAMEPLAY_H
#define SCREENGAMEPLAY_H
/*
-----------------------------------------------------------------------------
 Class: ScreenGameplay

 Desc: The music plays, the notes scroll, and the Player is pressing buttons.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"
#include "Transition.h"
#include "BitmapText.h"
#include "Player.h"
#include "RandomSample.h"
#include "RageSound.h"
#include "Background.h"
#include "LifeMeter.h"
#include "ScoreDisplay.h"
#include "DifficultyIcon.h"
#include "BPMDisplay.h"
class Inventory;
#include "ActiveItemList.h"
//#include "BeginnerHelper.h"	// uncomment once it's checked in
#include "LyricDisplay.h"
#include "TimingAssist.h"

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
	ScreenGameplay( bool bDemonstration = false );
	virtual ~ScreenGameplay();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );


protected:
	//BeginnerHelper	m_bhDancer; // The model for training
	Sprite	m_bhDancer;

	void TweenOnScreen();
	void TweenOffScreen();

	bool IsLastSong();
	void LoadNextSong();
	float StartPlayingSong(float MinTimeToNotes, float MinTimeToMusic);

	bool OneIsHot();
	bool AllAreInDanger();
	bool AllAreFailing();
	bool AllFailedEarlier();
	bool IsTimeToPlayTicks() const;
	void UpdateLyrics( float fDeltaTime );

	enum DancingState { 
		STATE_INTRO = 0, // not allowed to press Back
		STATE_DANCING,
		STATE_OUTRO,	// not allowed to press Back
		NUM_DANCING_STATES
	} m_DancingState;
	vector<Song*>		m_apSongsQueue;					// size may be >1 if playing a course
	vector<Notes*>		m_apNotesQueue[NUM_PLAYERS];	// size may be >1 if playing a course
	CStringArray		m_asModifiersQueue[NUM_PLAYERS];// size may be >1 if playing a course

	bool				m_bChangedOffsetOrBPM;
	float				m_fTimeLeftBeforeDancingComment;	// this counter is only running while STATE_DANCING

	LyricDisplay		m_LyricDisplay;

	TimingAssist		m_TimingAssist;

	Background			m_Background;

	Transition	m_NextSongIn;	// shows between songs in a course
	Transition	m_NextSongOut;	// shows between songs in a course

	Sprite				m_sprLifeFrame;
	LifeMeter*			m_pLifeMeter[NUM_PLAYERS];
	CombinedLifeMeter*	m_pCombinedLifeMeter;
	Sprite				m_sprStage;
	BitmapText			m_textCourseSongNumber[NUM_PLAYERS];

	BPMDisplay			m_BPMDisplay;

	Sprite				m_sprScoreFrame;
	ScoreDisplay*		m_pScoreDisplay[NUM_PLAYERS];
	ScoreKeeper*		m_pPrimaryScoreKeeper[NUM_PLAYERS];
	ScoreKeeper*		m_pSecondaryScoreKeeper[NUM_PLAYERS];
	BitmapText			m_textPlayerOptions[NUM_PLAYERS];
	BitmapText			m_textSongOptions;

	BitmapText			m_textDebug;

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

	BitmapText			m_textSurviveTime;	// used in extra stage
	BitmapText			m_textSongTitle;

	Player				m_Player[NUM_PLAYERS];

	// used in PLAY_MODE_BATTLE
	Inventory*			m_pInventory[NUM_PLAYERS];
	ActiveItemList		m_ActiveItemList[NUM_PLAYERS];
	
	DifficultyIcon		m_DifficultyIcon[NUM_PLAYERS];

	BGAnimation			m_bgaBH;
	Sprite				m_sprBH;

	Sprite				m_sprOniGameOver[NUM_PLAYERS];
	void				ShowOniGameOver( PlayerNumber pn );

	RandomSample	m_soundOniDie;
	RandomSample	m_announcerReady;
	RandomSample	m_announcerHereWeGo;
	RandomSample	m_announcerDanger;
	RandomSample	m_announcerGood;
	RandomSample	m_announcerHot;
	RandomSample	m_announcerOni;
	RandomSample	m_announcer100Combo;
	RandomSample	m_announcer200Combo;
	RandomSample	m_announcer300Combo;
	RandomSample	m_announcer400Combo;
	RandomSample	m_announcer500Combo;
	RandomSample	m_announcer600Combo;
	RandomSample	m_announcer700Combo;
	RandomSample	m_announcer800Combo;
	RandomSample	m_announcer900Combo;
	RandomSample	m_announcer1000Combo;
	RandomSample	m_announcerComboStopped;
	RandomSample	m_announcerBattleTrickLevel1;
	RandomSample	m_announcerBattleTrickLevel2;
	RandomSample	m_announcerBattleTrickLevel3;
	RandomSample	m_soundBattleTrickLevel1;
	RandomSample	m_soundBattleTrickLevel2;
	RandomSample	m_soundBattleTrickLevel3;
	RandomSample	m_announcerBattleDamageLevel1;
	RandomSample	m_announcerBattleDamageLevel2;
	RandomSample	m_announcerBattleDamageLevel3;
	RandomSample	m_announcerBattleDie;

	bool			m_bZeroDeltaOnNextUpdate;
	bool			m_bDemonstration;

	RageSound		m_soundAssistTick;
	RageSound		m_soundMusic;
};


#endif
