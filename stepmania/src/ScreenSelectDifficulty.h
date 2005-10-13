/* ScreenSelectDifficulty - Deprecated. Replaced by ScreenSelectMaster. */

#ifndef ScreenSelectDifficulty_H
#define ScreenSelectDifficulty_H

#include "ScreenSelect.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "RageSound.h"
#include "RandomSample.h"

#define MAX_CHOICES_PER_PAGE 15

class ScreenSelectDifficulty : public ScreenSelect
{
public:
	ScreenSelectDifficulty( CString sName );
	virtual void Init();
	virtual void BeginScreen();

	virtual void Update( float fDelta );

	virtual void MenuLeft( PlayerNumber pn );
	virtual void MenuRight( PlayerNumber pn );
	virtual void MenuUp( PlayerNumber pn ) {};
	virtual void MenuDown( PlayerNumber pn ) {};
	virtual void MenuStart( PlayerNumber pn );
//	virtual void MenuBack( PlayerNumber pn );
	virtual void TweenOffScreen();

	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	enum Page { PAGE_1, PAGE_2, NUM_PAGES };

	virtual int GetSelectionIndex( PlayerNumber pn );
	virtual void UpdateSelectableChoices();

	float GetCursorX( PlayerNumber pn );
	float GetCursorY( PlayerNumber pn );
	bool ChangeWithinPage( PlayerNumber pn, int iNewChoice, bool bChangingPages );
	void ChangePage( Page newPage );

	ActorFrame	m_framePages;	// to hold the 2 pages

	Sprite	m_sprInfo[NUM_PAGES][MAX_CHOICES_PER_PAGE];
	Sprite	m_sprPicture[NUM_PAGES][MAX_CHOICES_PER_PAGE];
	Sprite	m_sprExplanation[NUM_PAGES];
	Sprite	m_sprMore[NUM_PAGES];

	Sprite	m_sprCursor[NUM_PLAYERS];
	Sprite	m_sprShadow[NUM_PLAYERS];
	Sprite	m_sprOK[NUM_PLAYERS];

	RageSound	m_soundChange;
	RandomSample m_soundDifficult;

	vector<GameCommand> m_GameCommands[NUM_PAGES];

	Page m_CurrentPage;
	int m_iChoiceOnPage[NUM_PLAYERS];
	bool m_bChosen[NUM_PLAYERS];

	float m_fLockInputTime;
};

#endif

/*
 * (c) 2001-2004 Chris Danford
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
