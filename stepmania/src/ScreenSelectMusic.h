/*
-----------------------------------------------------------------------------
 Class: ScreenSelectMusic

 Desc: The screen in PLAY_MODE_ARCADE where you choose a Song and Notes.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "RandomStream.h"
#include "GameConstantsAndTypes.h"
#include "MusicWheel.h"
#include "Banner.h"
#include "SongInfoFrame.h"
#include "MenuElements.h"
#include "GrooveRadar.h"
#include "DifficultyIcon.h"
#include "FootMeter.h"


class ScreenSelectMusic : public Screen
{
public:
	ScreenSelectMusic();
	virtual ~ScreenSelectMusic();

	virtual void DrawPrimitives();

	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void TweenOnScreen();
	void TweenOffScreen();

	void MenuLeft( const PlayerNumber p, const InputEventType type );
	void MenuRight( const PlayerNumber p, const InputEventType type );
	void MenuStart( const PlayerNumber p );
	void MenuBack( const PlayerNumber p );

protected:
	void EasierDifficulty( const PlayerNumber p );
	void HarderDifficulty( const PlayerNumber p );

	void AfterNotesChange( const PlayerNumber p );
	void AfterMusicChange();
	void PlayMusicSample();

	CArray<Notes*, Notes*> m_arrayNotes;
	int				m_iSelection[NUM_PLAYERS];

	MenuElements	m_Menu;

	SongInfoFrame	m_SongInfoFrame;
	Sprite			m_sprDifficultyFrame;
	DifficultyIcon	m_DifficultyIcon[NUM_PLAYERS];
	GrooveRadar		m_GrooveRadar;
	Sprite			m_sprMeterFrame;
	FootMeter		m_FootMeter[NUM_PLAYERS];
	MusicWheel		m_MusicWheel;

	bool			m_bMadeChoice;
	bool			m_bGoToOptions;
	BitmapText		m_textHoldForOptions;

	RandomSample m_soundSelect;
	RandomSample m_soundChangeNotes;
};


