/* ScreenSelectMode - The "Style Select Screen" for Ez2dancer */

#ifndef SCREEN_SELECT_MODE_H
#define SCREEN_SELECT_MODE_H
/* Includes */

#include "ScreenSelect.h"
#include "BGAnimation.h"
#include "Screen.h"
#include "Sprite.h"
#include "Quad.h"
#include "ScrollingList.h"
#include "GameConstantsAndTypes.h"
#include "ModeChoice.h"
#include "BitmapText.h"
#include "RandomSample.h"
#include "Character.h"

/* Class Definition */

#define MAX_ELEMS 30

class ScreenSelectMode : public ScreenSelect
{
public:
	ScreenSelectMode( CString sName ); // Constructor
	virtual ~ScreenSelectMode(); // Destructor
	virtual void MenuLeft( PlayerNumber pn );
	virtual void MenuRight( PlayerNumber pn );
	virtual void MenuUp(PlayerNumber pn );
	virtual void MenuDown(PlayerNumber pn);
	virtual void MenuStart( PlayerNumber pn );
	virtual void Update( float fDelta );
	virtual void DrawPrimitives();
	virtual void HandleScreenMessage( const ScreenMessage SM );
protected:
	virtual int GetSelectionIndex( PlayerNumber pn );
	virtual void UpdateSelectableChoices();
	void SetCharacters();
	void ChangeBGA();
	int m_iNumChoices;
	int m_iSelectableChoices[MAX_ELEMS];

	RageSound			m_soundModeChange;
	RageSound			m_soundConfirm;
	RageSound			m_soundStart;
	CStringArray arrayLocations;
	ScrollingList m_ScrollingList;
	Sprite m_ChoiceListFrame;
	Sprite m_ChoiceListHighlight;
	Sprite m_sprJoinMessage[NUM_PLAYERS];
	Sprite m_sprJoinFrame[NUM_PLAYERS];
	Sprite m_CurChar[NUM_PLAYERS];
	int m_iCurrentChar[NUM_PLAYERS];
	Sprite m_Guide;
	vector<BGAnimation*>	m_Backgrounds;
	bool m_bSelected;
	bool m_b2DAvailable;
	bool m_bCharsAvailable;
	
//	private:
//	vector<Character*>		m_Characters;
};

#endif

/*
 * (c) 2001-2003 "Frieza", Chris Danford
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
