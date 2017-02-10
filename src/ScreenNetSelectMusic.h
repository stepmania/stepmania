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

	virtual bool Input( const InputEventPlus &input );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void StartSelectedSong();
	bool SelectCurrent();

	MusicWheel* GetMusicWheel();
	// Lua
	virtual void PushSelf(lua_State *L);

protected:
	virtual bool MenuStart( const InputEventPlus &input );
	virtual bool MenuBack( const InputEventPlus &input );
	virtual bool MenuLeft( const InputEventPlus &input );
	virtual bool MenuRight( const InputEventPlus &input );
	virtual bool MenuUp( const InputEventPlus &input );
	virtual bool MenuDown( const InputEventPlus &input );
	bool LeftAndRightPressed( const PlayerNumber pn );

	virtual void Update( float fDeltaTime );

	void MusicChanged();

	void TweenOffScreen();

	ThemeMetric<SampleMusicPreviewMode> SAMPLE_MUSIC_PREVIEW_MODE;
	RString m_sSectionMusicPath;
	RString m_sRouletteMusicPath;
	RString m_sRandomMusicPath;

	ThemeMetric<RString>	MUSIC_WHEEL_TYPE;
	ThemeMetric<RString>	PLAYER_OPTIONS_SCREEN;

private:
	MusicWheel m_MusicWheel;

	StepsDisplay m_StepsDisplays[NUM_PLAYERS];
	Difficulty m_DC[NUM_PLAYERS];

	void UpdateDifficulties( PlayerNumber pn );

	RageSound m_soundChangeOpt;
	RageSound m_soundChangeSel;

	// todo: do this theme-side instead. -aj
	ModIconRow m_ModIconRow[NUM_PLAYERS];

	Song * m_cSong;

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
