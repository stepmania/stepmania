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
#include "TransitionStarWipe.h"
#include "TransitionFade.h"
#include "BitmapText.h"
#include "Player.h"
#include "RandomSample.h"
#include "RandomStream.h"
#include "FocusingSprite.h"
#include "RageMusic.h"
#include "MotionBlurSprite.h"
#include "Background.h"
#include "LifeMeterBar.h"
#include "ScoreDisplayRolling.h"
#include "DifficultyBanner.h"


class ScreenGameplay : public Screen
{
public:
	ScreenGameplay();
	virtual ~ScreenGameplay();
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	enum DancingState { 
		STATE_INTRO = 0, // not allowed to press Back
		STATE_DANCING,
		STATE_OUTRO,	// not allowed to press Back
		NUM_DANCING_STATES
	};


private:
	void TweenOnScreen();
	void TweenOffScreen();
	void SaveSummary();
	void LoadNextSong();


	DancingState			m_DancingState;

	Song*					m_pCurSong;	// nearest songs are on back of queue
	CArray<Song*,Song*>		m_apSongQueue;	// nearest songs are on back of queue
	CArray<Notes*,Notes*>	m_apNotesQueue[NUM_PLAYERS];	// nearest notes are on back of queue
	bool					m_bBothHaveFailed;

	float					m_fTimeLeftBeforeDancingComment;	// this counter is only running while STATE_DANCING


	Background				m_Background;

	ActorFrame				m_frameTop;
	Sprite					m_sprTopFrame;
	Quad					m_quadLifeMeterBG[NUM_PLAYERS];	// just a black quad to fill in hole for the life meter
	LifeMeterBar			m_LifeMeter[NUM_PLAYERS];
	BitmapText				m_textStageNumber;

	ActorFrame				m_frameBottom;
	Sprite					m_sprBottomFrame;
	ScoreDisplayRolling		m_ScoreDisplay[NUM_PLAYERS];
	BitmapText				m_textPlayerOptions[NUM_PLAYERS];

	BitmapText				m_textDebug;


	TransitionStarWipe	m_StarWipe;

	FocusingSprite		m_sprReady;
	FocusingSprite		m_sprHereWeGo;
	FocusingSprite		m_sprCleared;
	MotionBlurSprite	m_sprFailed;

	Player				m_Player[NUM_PLAYERS];

	DifficultyBanner	m_DifficultyBanner[NUM_PLAYERS];

	RandomSample	m_soundFail;
	RandomSample	m_announcerReady;
	RandomSample	m_announcerHereWeGo;
	RandomSample	m_announcerDanger;
	RandomSample	m_announcerGood;
	RandomSample	m_announcerHot;
	RandomStream	m_announcerCleared;
	RandomStream	m_announcerFailComment;
	RandomStream	m_announcerGameOver;

	RandomSample	m_soundAssistTick;

	RageSoundStream		m_soundMusic;

};



