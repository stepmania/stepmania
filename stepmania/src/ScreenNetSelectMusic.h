/* ScreenNetSelectMusic - A method for Online/Net song selection */

#ifndef SCREENNETSELECTMUSIC_H
#define SCREENNETSELECTMUSIC_H

#include "ScreenWithMenuElements.h"
#include "Sprite.h"
#include "Quad.h"
#include "BitmapText.h"
#include "MusicBannerWheel.h"
#include "DifficultyRating.h"
#include "ModeSwitcher.h"
#include "DifficultyIcon.h"
#include "Difficulty.h"
#include "DifficultyMeter.h"

class ScreenNetSelectMusic : public ScreenWithMenuElements
{
public:
	ScreenNetSelectMusic( const CString& sName );
	virtual void DrawPrimitives();

	virtual void Update( float fDeltaTime );
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type,
						const GameInput& GameI, const MenuInput& MenuI,
						const StyleInput& StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void UpdateSongsListPos();
	void UpdateGroupsListPos();
	void UpdateSongsList();
	void UpdateTextInput();

	void StartSelectedSong();

private:
	int	m_iSongNum;
	int	m_iShowSongs;	//The number of songs to display to each side
	int	m_iGroupNum;
	int	m_iShowGroups;	//The number of groups to display to each side

	enum NetScreenSelectModes
	{
		SelectGroup = 0,
		SelectSong,
		SelectDifficulty,
		SelectOptions,
		SelectModes,
	};

	NetScreenSelectModes m_SelectMode;

protected:
	virtual void MenuStart( PlayerNumber pn );
	virtual void MenuLeft( PlayerNumber pn, const InputEventType type );
	virtual void MenuUp( PlayerNumber pn, const InputEventType type );
	virtual void MenuDown( PlayerNumber pn, const InputEventType type );
	virtual void MenuRight( PlayerNumber pn, const InputEventType type );
	virtual void MenuBack( PlayerNumber pn );

	void MusicChanged();

	void TweenOffScreen();

private:
	//Chatting
	BitmapText		m_textChatInput;
	BitmapText		m_textChatOutput;
	BitmapText		m_textOutHidden;
	Quad			m_rectChatInputBox;
	Quad			m_rectChatOutputBox;
	CString			m_sTextInput;
	CString			m_actualText;


	//Selection
	Quad			m_rectSelection;

	//Groups
	BitmapText		m_textGroups;
	Quad			m_rectGroupsBackground;
	vector <CString>	m_vGroups;

	//Songs
	BitmapText		m_textSongs;
	Quad			m_rectSongsBackground;
	vector <Song *>		m_vSongs;

	Quad			m_rectExInfo;
	Quad			m_rectDiff;

	BitmapText		m_textArtist;
	BitmapText		m_textSubtitle;

	//Difficulty Icon(s)
	DifficultyIcon		m_DifficultyIcon[NUM_PLAYERS];
	Difficulty			m_DC[NUM_PLAYERS];

	//Select Options
	Sprite			m_sprSelOptions;

	void UpdateDifficulties( PlayerNumber pn );
	DifficultyMeter		m_DifficultyMeters[NUM_PLAYERS];

	RageSound m_soundChangeOpt;
	RageSound m_soundChangeSel;

};

#endif

/*
 * (c) 2004 Charles Lohr
 * All rights reserved.
 *
 *     based off of ScreenEz2SelectMusic by "Frieza"
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
