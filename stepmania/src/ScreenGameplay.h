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
#include "TransitionFade.h"
#include "TransitionStarWipe.h"
#include "TransitionFadeWipe.h"
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
#include "FocusingSprite.h"
#include "Inventory.h"
#include "ActiveItemList.h"


// messages sent by Combo
const ScreenMessage SM_BeginToasty			= ScreenMessage(SM_User+104);

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

	enum DancingState { 
		STATE_INTRO = 0, // not allowed to press Back
		STATE_DANCING,
		STATE_OUTRO,	// not allowed to press Back
		NUM_DANCING_STATES
	} m_DancingState;
	vector<Song*>		m_apCourseSongs;		// used only in a GameModes with courses
	vector<Notes*>		m_apCourseNotes;		// used only in a GameModes with courses
	CStringArray		m_asCourseModifiers;	// used only in a GameModes with courses

	bool				m_bChangedOffsetOrBPM;

	float				m_fTimeLeftBeforeDancingComment;	// this counter is only running while STATE_DANCING


	Background			m_Background;

	TransitionOniFade	m_OniFade;	// shows between songs in a course

	Sprite				m_sprLifeFrame;
	LifeMeter*			m_pLifeMeter[NUM_PLAYERS];
	BitmapText			m_textStageNumber;
	BitmapText			m_textCourseSongNumber[NUM_PLAYERS];

	Sprite				m_sprMiddleFrame;
	BPMDisplay			m_BPMDisplay;

	Sprite				m_sprScoreFrame;
	ScoreDisplay*		m_pScoreDisplay[NUM_PLAYERS];
	BitmapText			m_textPlayerOptions[NUM_PLAYERS];
	BitmapText			m_textSongOptions;

	BitmapText			m_textDebug;

#define NUM_STATUS_ICONS	2
	Sprite				m_sprStatusIcons[NUM_STATUS_ICONS];	// shows whether these options are on.
	void	PositionStatusIcons();	// reposition the three above when the value of one changes
	
	BitmapText			m_MaxCombo;

	TransitionFadeWipe	m_Fade;
	TransitionStarWipe	m_StarWipe;

	FocusingSprite		m_sprReady;
	FocusingSprite		m_sprHereWeGo;
	MotionBlurSprite	m_sprCleared;
	MotionBlurSprite	m_sprFailed;
	MotionBlurSprite	m_sprTryExtraStage;

	BitmapText			m_textSurviveTime;	// only shown in extra stage
	BitmapText			m_StageName;

	Player				m_Player[NUM_PLAYERS];

	Inventory			m_Inventory;
	ActiveItemList		m_ActiveItemList[NUM_PLAYERS];

	DifficultyIcon		m_DifficultyIcon[NUM_PLAYERS];

	Sprite				m_sprOniGameOver[NUM_PLAYERS];
	void				ShowOniGameOver( PlayerNumber pn );

	Sprite				m_sprToasty;	// easter egg

	RandomSample	m_soundFail;
	RandomSample	m_soundOniDie;
	RandomSample	m_soundTryExtraStage;
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

	
	int				m_iRowLastCrossed;

	RageSound		m_soundAssistTick;
	RageSound		m_soundToasty;
	RageSound		m_soundMusic;
};


#endif
