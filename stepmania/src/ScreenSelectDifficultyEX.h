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
#include "DifficultyIcon.h"
#include "OptionsCursor.h"

class ScreenSelectDifficultyEX : public ScreenSelect
{
public:
	ScreenSelectDifficultyEX();

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
	DifficultyIcon	m_sprDifficultyIcon[8];
	Sprite	m_sprIconBar;
	BitmapText	m_textDifficultyText[8];
	OptionsCursor	m_sprHighlight[NUM_PLAYERS];

	int	GetIconIndex( CString DiffName );
	void SetDifficultyIconText( bool bDisplayCourseItems );
/* -------------- */


	Sprite	m_sprOK[NUM_PLAYERS];
	Sprite	m_sprPicture[NUM_PLAYERS];

	RageSound		m_soundChange;
	RageSound		m_soundSelect;
	RandomSample	m_soundDifficult;

	vector<ModeChoice> m_ModeChoices;

	Page m_CurrentPage;
	int m_iChoice[NUM_PLAYERS];
	bool m_bChosen[NUM_PLAYERS];

	float m_fLockInputTime;
};

#endif
