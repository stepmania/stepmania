#ifndef ScreenSelectDifficultyEX_H
#define ScreenSelectDifficultyEX_H
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectDifficultyEX

 Desc: DDR Extreme Difficulty Select

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Kevin Slaughter
-----------------------------------------------------------------------------
*/


#include "BitmapText.h"
#include "RageSound.h"
#include "RandomSample.h"
#include "ScreenSelect.h"
#include "Sprite.h"

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

	bool AllPlayersAreOnCourse();
	void Change( PlayerNumber pn, int iNewChoice );
	float GetCursorX( PlayerNumber pn );
	float GetCursorY( PlayerNumber pn );
	bool IsACourse( int mIndex );
	bool PlayerIsOnCourse( PlayerNumber pn );
	void SetAllPlayersSelection( int iChoice, bool bSwitchingModes );

	ActorFrame	m_framePages;
	Sprite	m_sprCursor[NUM_PLAYERS];
//	Sprite	m_sprExplanation;		Will implement properly soon -- Miryokuteki
	Sprite	m_sprInfo[NUM_PLAYERS];
//	Sprite	m_sprMore;				Will implement properly soon -- Miryokuteki
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
