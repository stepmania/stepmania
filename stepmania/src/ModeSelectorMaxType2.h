#ifndef ModeSelectorMaxType2_H
#define ModeSelectorMaxType2_H
/*
-----------------------------------------------------------------------------
 Class: ModeSelectorMaxType2

 Desc: Abstract class for a widget that selects a ModeChoice

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ModeSelector.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "RageSound.h"
#include "RandomSample.h"


#define MAX_CHOICES_PER_PAGE	10


class ModeSelectorMaxType2 : public ModeSelector
{
public:
	enum Page { PAGE_1, PAGE_2, NUM_PAGES };

	ModeSelectorMaxType2();
	virtual ~ModeSelectorMaxType2();

	virtual void Init( const vector<ModeChoice>& choices, CString sClassName, CString sThemeElementPrefix );

	virtual void Update( float fDelta );

	virtual void MenuLeft( PlayerNumber pn );
	virtual void MenuRight( PlayerNumber pn );
	virtual void MenuUp( PlayerNumber pn ) {};
	virtual void MenuDown( PlayerNumber pn ) {};
	virtual void MenuStart( PlayerNumber pn );
	virtual void MenuBack( PlayerNumber pn );
	virtual void TweenOffScreen();
	virtual void TweenOnScreen();

	virtual void GetSelectedModeChoice( PlayerNumber pn, ModeChoice* pModeChoiceOut );
	virtual void UpdateSelectableChoices();

	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	float GetCursorX( PlayerNumber pn );
	float GetCursorY( PlayerNumber pn );
	void ChangeWithinPage( PlayerNumber pn, int iNewChoice, bool bChangingPages );
	void ChangePage( Page newPage );

	ActorFrame	m_framePages;	// to hold the 2 pages

	Sprite	m_sprHeader[NUM_PAGES][MAX_CHOICES_PER_PAGE];
	Sprite	m_sprPicture[NUM_PAGES][MAX_CHOICES_PER_PAGE];
	Sprite	m_sprExplanation[NUM_PAGES];
	Sprite	m_sprMore[NUM_PAGES];

	Sprite	m_sprCursor[NUM_PLAYERS];
	Sprite	m_sprShadow[NUM_PLAYERS];
	Sprite	m_sprOK[NUM_PLAYERS];

	RageSound	m_soundChange;
	RageSound	m_soundSelect;
	RandomSample m_soundDifficult;

	vector<ModeChoice> m_ModeChoices[NUM_PAGES];

	Page m_CurrentPage;
	int m_iChoiceOnPage[NUM_PLAYERS];
	bool m_bChosen[NUM_PLAYERS];

	float m_fLockInputTime;
};

#endif
