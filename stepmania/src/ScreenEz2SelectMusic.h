/* ScreenEz2SelectMusic - A Scrolling List Of Song Banners used to select the song the player wants. */

#ifndef SCREENEZ2SELECTMUSIC_H
#define SCREENEZ2SELECTMUSIC_H

#include "ScreenWithMenuElements.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "MusicBannerWheel.h"
#include "DifficultyRating.h"
#include "ModeSwitcher.h"
#include "RageTexturePreloader.h"
#include "RageSound.h"

class ScreenEz2SelectMusic : public ScreenWithMenuElements
{
public:
	virtual void Init();

	virtual void Update( float fDeltaTime );
	virtual void Input( const InputEventPlus &input );
	virtual void HandleScreenMessage( const ScreenMessage SM );
	virtual void MenuStart( PlayerNumber pn );
	virtual void MenuLeft( const InputEventPlus &input );
	virtual void MenuRight( const InputEventPlus &input );
	virtual void MenuBack( PlayerNumber pn );

protected:
	void AfterNotesChange( PlayerNumber pn );
	void MusicChanged();

	void EasierDifficulty( PlayerNumber pn );
	void HarderDifficulty( PlayerNumber pn );

	void UpdateOptions( PlayerNumber pn, int nosound );

	RString sOptions;
	vector<RString> asOptions;

	void TweenOffScreen();

	ModeSwitcher m_ModeSwitcher;
	Sprite  m_ChoiceListFrame;
	Sprite  m_ChoiceListHighlight;
	Sprite  m_Guide;
	Sprite	m_sprOptionsMessage;
	Sprite	m_InfoFrame;
	Sprite  m_PumpDifficultyCircle;
	Sprite	m_SpeedIcon[NUM_PLAYERS];
	Sprite	m_MirrorIcon[NUM_PLAYERS];
	Sprite	m_ShuffleIcon[NUM_PLAYERS];
	Sprite	m_HiddenIcon[NUM_PLAYERS];
	Sprite	m_VanishIcon[NUM_PLAYERS];
	Sprite				m_sprBalloon;
	BitmapText	m_PumpDifficultyRating;
	BitmapText  m_CurrentGroup;
	BitmapText  m_CurrentTitle;
	BitmapText  m_CurrentSubTitle;
	BitmapText  m_CurrentArtist;


	RageSound			m_soundOptionsChange;
	RageSound			m_soundMusicChange;
	RageSound			m_soundMusicCycle;
	RageSound			m_soundBackMusic;
	RageSound			m_soundButtonPress;

	float m_fRemainingWaitTime;
	MusicBannerWheel			m_MusicBannerWheel;
	DifficultyRating	m_DifficultyRating;
	vector<Steps*>		m_arrayNotes[NUM_PLAYERS];

	int					m_iSelection[NUM_PLAYERS];
	bool m_bGoToOptions;
	bool m_bMadeChoice;
	bool m_bTransitioning;
	bool m_bScanning;

	int i_SkipAheadOffset;
	float ScrollStartTime;
	float LastInputTime;

	int i_ErrorDetected;

	int iConfirmSelection;

	RageTexturePreloader m_TexturePreload;
};

#endif

/*
 * (c) 2002-2003 "Frieza"
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
