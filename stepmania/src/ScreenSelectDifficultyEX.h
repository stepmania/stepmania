#ifndef ScreenSelectDifficultyEX_H
#define ScreenSelectDifficultyEX_H
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectDifficultyEX

 Desc: DDR Extreme Difficulty Select

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Kevin Slaughter
-----------------------------------------------------------------------------
*/


#include "BitmapText.h"
#include "RageSound.h"
#include "RandomSample.h"
#include "ScreenSelect.h"
#include "Sprite.h"
#include "OptionsCursor.h"

class ScreenSelectDifficultyEX : public ScreenSelect
{
public:
	ScreenSelectDifficultyEX( CString sName );

	virtual void HandleScreenMessage( const ScreenMessage SM );
	virtual void MenuLeft( PlayerNumber pn );
	virtual void MenuRight( PlayerNumber pn );
	virtual void MenuUp( PlayerNumber pn ) {};
	virtual void MenuDown( PlayerNumber pn ) {};
	virtual void MenuStart( PlayerNumber pn );
	virtual void TweenOffScreen();
	virtual void TweenOnScreen();
	virtual void Update( float fDelta );

protected:
	enum Page { PAGE_1, NUM_PAGES };

	virtual int GetSelectionIndex( PlayerNumber pn );
	virtual void UpdateSelectableChoices();

	void Change( PlayerNumber pn, int iNewChoice );
	float GetCursorX( PlayerNumber pn );
	float GetCursorY( PlayerNumber pn );
	bool IsACourse( int mIndex );
	bool IsValidModeName( CString ModeName );
	void SetAllPlayersSelection( int iChoice, bool bSwitchingModes );

	ActorFrame	m_framePages;
	Sprite	m_sprCursor[NUM_PLAYERS];
	Sprite	m_sprInfo[NUM_PLAYERS];


/* Icon Bar stuff */
	unsigned m_NumModes;
	Sprite	m_sprDifficulty[8];
	Sprite	m_sprIconBar;
	BitmapText	m_textDifficultyText[8];
	OptionsCursor	m_sprHighlight[NUM_PLAYERS];

	int	GetIconIndex( CString DiffName );
	void SetDifficultyIconText( bool bDisplayCourseItems );
/* -------------- */


	Sprite	m_sprOK[NUM_PLAYERS];
	Sprite	m_sprPicture[NUM_PLAYERS];

	RageSound		m_soundChange;
	RandomSample	m_soundDifficult;

	vector<ModeChoice> m_ModeChoices;

	Page m_CurrentPage;
	int m_iChoice[NUM_PLAYERS];
	bool m_bChosen[NUM_PLAYERS];

	float m_fLockInputTime;
};

#endif

/*
 * (c) 2001-2003 Kevin Slaughter
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
