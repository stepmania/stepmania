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
#include "TransitionBGAnimation.h"
#include "TransitionOniFade.h"
#include "BitmapText.h"
#include "Player.h"
#include "RandomSample.h"
#include "RageSoundManager.h"
#include "RageSound.h"
#include "MotionBlurSprite.h"
#include "Background.h"
#include "LifeMeter.h"
#include "ScoreDisplay.h"
#include "DifficultyIcon.h"
#include "BPMDisplay.h"
#include "Inventory.h"
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

	TransitionOniFade	m_OniFade;	// shows between songs in a course

	Sprite				m_sprLifeFrame;
	LifeMeter*			m_pLifeMeter[NUM_PLAYERS];
	Sprite				m_sprStage;
	BitmapText			m_textCourseSongNumber[NUM_PLAYERS];

	BPMDisplay			m_BPMDisplay;

	Sprite				m_sprScoreFrame;
	ScoreDisplay*		m_pScoreDisplay[NUM_PLAYERS];
	ScoreKeeper*		m_pScoreKeeper[NUM_PLAYERS];
	BitmapText			m_textPlayerOptions[NUM_PLAYERS];
	BitmapText			m_textSongOptions;

	BitmapText			m_textDebug;

	BitmapText			m_textAutoPlay;	// for AutoPlay, AutoAdjust
	void	UpdateAutoPlayText();
	
	BitmapText			m_MaxCombo;

	TransitionBGAnimation	m_Ready;
	TransitionBGAnimation	m_Go;
	TransitionBGAnimation	m_Cleared;
	TransitionBGAnimation	m_Failed;
	TransitionBGAnimation	m_Extra;
	TransitionBGAnimation	m_Toasty;	// easter egg
	TransitionBGAnimation	m_In;
	TransitionBGAnimation	m_Back;

	BitmapText			m_textSurviveTime;	// used in extra stage
	BitmapText			m_textSongTitle;

	Player				m_Player[NUM_PLAYERS];

	Inventory			m_Inventory[NUM_PLAYERS];
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

	bool			m_bDemonstration;
	int				m_iRowLastCrossed;

	RageSound		m_soundAssistTick;
	RageSound		m_soundMusic;
};


#endif
