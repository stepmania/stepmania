#ifndef ScreenSelectMaster_H
#define ScreenSelectMaster_H
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectMaster

 Desc: Abstract class for a widget that selects a ModeChoice

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ScreenSelect.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "RageSound.h"
#include "RandomSample.h"

class ScreenSelectMaster : public ScreenSelect
{
public:
	ScreenSelectMaster();

	virtual void Update( float fDelta );

	virtual void MenuLeft( PlayerNumber pn );
	virtual void MenuRight( PlayerNumber pn );
	virtual void MenuUp( PlayerNumber pn ) {};
	virtual void MenuDown( PlayerNumber pn ) {};
	virtual void MenuStart( PlayerNumber pn );
//	virtual void MenuBack( PlayerNumber pn );
	float TweenOffScreen();	// return time in seconds to execute the command
	float TweenOnScreen();	// return time in seconds to execute the command

	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	enum Page { PAGE_1, PAGE_2, NUM_PAGES };	// on PAGE_2, cursors are locked together
	Page GetPage( int iChoiceIndex );
	Page GetCurrentPage();

	virtual int GetSelectionIndex( PlayerNumber pn );
	virtual void UpdateSelectableChoices();

	void ChangePage( Page page );
	void ChangeSelection( PlayerNumber pn, int iNewChoice );

	float GetCursorX( PlayerNumber pn, int iPartIndex );
	float GetCursorY( PlayerNumber pn, int iPartIndex );

	Sprite	m_sprExplanation[NUM_PAGES];
	Sprite	m_sprMore[NUM_PAGES];
#define MAX_ICON_PARTS 3
	Sprite	m_sprIcon[MAX_ICON_PARTS][MAX_CHOICES];
#define MAX_PREVIEW_PARTS 3
	Sprite	m_sprPreview[MAX_PREVIEW_PARTS][MAX_CHOICES][NUM_PLAYERS];
#define MAX_CURSOR_PARTS 3
	Sprite	m_sprCursor[MAX_CURSOR_PARTS][NUM_PLAYERS];

	RageSound	m_soundChange;
	RageSound	m_soundSelect;
	RandomSample m_soundDifficult;

	int m_iChoice[NUM_PLAYERS];
	bool m_bChosen[NUM_PLAYERS];

	float m_fLockInputSecs;
};

#endif
