#ifndef SCREENEZ2SELECTMUSIC_H
#define SCREENEZ2SELECTMUSIC_H
/*
-----------------------------------------------------------------------------
 Class: ScreenEz2SelectMusic

 Desc: A Scrolling List Of Song Banners used to select the song the player wants.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Andrew Livy
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "TransitionStarWipe.h"
#include "MenuElements.h"
#include "TipDisplay.h"
#include "MusicBannerWheel.h"
#include "MenuElements.h"
#include "FootMeter.h"

class ScreenEz2SelectMusic : public Screen
{
public:
	ScreenEz2SelectMusic();
	virtual void DrawPrimitives();

	virtual void Update( float fDeltaTime );
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );
	virtual void MenuStart( PlayerNumber pn );
	virtual void MenuLeft( PlayerNumber pn, const InputEventType type );
	virtual void MenuRight( PlayerNumber pn, const InputEventType type );
	virtual void MenuBack( PlayerNumber pn );

protected:
	void AfterNotesChange( PlayerNumber pn );
	void MusicChanged();

	void EasierDifficulty( PlayerNumber pn );
	void HarderDifficulty( PlayerNumber pn );

	Sprite  m_ChoiceListFrame;
	Sprite  m_ChoiceListHighlight;
	Sprite  m_Guide;
	Sprite				m_sprOptionsMessage;
	Sprite  m_PumpDifficultyCircle;
	BitmapText	m_PumpDifficultyRating;
	MusicBannerWheel			m_MusicBannerWheel;
	MenuElements		m_Menu;
	FootMeter			m_FootMeter[NUM_PLAYERS];
	CArray<Notes*, Notes*> m_arrayNotes[NUM_PLAYERS];
	int					m_iSelection[NUM_PLAYERS];
	bool m_bGoToOptions;
	bool m_bMadeChoice;

	int i_ErrorDetected;

	RageSoundSample		m_soundSelect;
};


#endif
