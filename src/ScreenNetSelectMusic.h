/* ScreenNetSelectMusic - A method for Online/Net song selection */

#ifndef SCREEN_NET_SELECT_MUSIC_H
#define SCREEN_NET_SELECT_MUSIC_H

#include "ScreenNetSelectBase.h"
#include "ScreenWithMenuElements.h"
#include "Sprite.h"
#include "DifficultyIcon.h"
#include "Difficulty.h"
#include "StepsDisplay.h"
#include "MusicWheel.h"
#include "ModIconRow.h"
#include "BPMDisplay.h"

class ScreenNetSelectMusic : public ScreenNetSelectBase
{
public:
	virtual void Init();

	virtual void Input( const InputEventPlus &input );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void StartSelectedSong();

protected:
	virtual void MenuStart( const InputEventPlus &input );
	virtual void MenuBack( const InputEventPlus &input );
	virtual void MenuLeft( const InputEventPlus &input );
	virtual void MenuRight( const InputEventPlus &input );
	bool DetectCodes( const InputEventPlus &input );

	virtual void Update( float fDeltaTime );

	void ChangeSteps( const InputEventPlus &input, int dir );
	void MusicChanged();

	void TweenOffScreen();

	vector<Steps*>	m_vpSteps;
	int				m_iSelection[NUM_PLAYERS];

	ThemeMetric<SampleMusicPreviewMode> SAMPLE_MUSIC_PREVIEW_MODE;
	RString m_sSectionMusicPath;
	RString m_sRouletteMusicPath;
	RString m_sRandomMusicPath;

	ThemeMetric<RString>	CODES;
	ThemeMetric<RString>	MUSIC_WHEEL_TYPE;

private:
	MusicWheel m_MusicWheel;

	Difficulty m_Difficulty[NUM_PLAYERS];
	StepsDisplay m_StepsDisplays[NUM_PLAYERS];
	void UpdateDifficulty( PlayerNumber pn );

	RageSound m_soundChangeOpt;
	RageSound m_soundChangeSel;

	// todo: do these theme-side instead. -aj
	BPMDisplay m_BPMDisplay;
	ModIconRow m_ModIconRow[NUM_PLAYERS];

	Song* m_cSong;

	bool m_bInitialSelect;
	bool m_bAllowInput;
};

#endif

/*
 * (c) 2004-2005 Charles Lohr
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
