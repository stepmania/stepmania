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
#include "ActorUtil.h"
#include "ActorScroller.h"

class ScreenSelectMaster : public ScreenSelect
{
public:
	ScreenSelectMaster( CString sName );

	virtual void Update( float fDelta );

	virtual void MenuLeft( PlayerNumber pn );
	virtual void MenuRight( PlayerNumber pn );
	virtual void MenuUp( PlayerNumber pn );
	virtual void MenuDown( PlayerNumber pn );
	virtual void MenuStart( PlayerNumber pn );
	void TweenOffScreen();
	void TweenOnScreen();

	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	enum Page { PAGE_1, PAGE_2, NUM_PAGES };	// on PAGE_2, cursors are locked together
	Page GetPage( int iChoiceIndex ) const;
	Page GetCurrentPage() const;

	enum Dirs
	{
		DIR_UP,
		DIR_DOWN,
		DIR_LEFT,
		DIR_RIGHT,
		DIR_AUTO, // when players join and the selection becomes invalid
		NUM_DIRS
	};
	static const char *dirs[NUM_DIRS];
	int m_Next[NUM_DIRS][MAX_CHOICES];

	virtual int GetSelectionIndex( PlayerNumber pn );
	virtual void UpdateSelectableChoices();

	bool Move( PlayerNumber pn, Dirs dir );
	bool ChangePage( int iNewChoice );
	bool ChangeSelection( PlayerNumber pn, int iNewChoice );
	float DoMenuStart( PlayerNumber pn );

	float GetCursorX( PlayerNumber pn, int iPartIndex );
	float GetCursorY( PlayerNumber pn, int iPartIndex );

	AutoActor	m_sprExplanation[NUM_PAGES];
	Sprite	m_sprMore[NUM_PAGES];
#define MAX_ICON_PARTS 3
	// icon is the piece shared, per-choice piece
	AutoActor m_sprIcon[MAX_ICON_PARTS][MAX_CHOICES];
#define MAX_PREVIEW_PARTS 3
	// preview is per-choice, per-player piece
	AutoActor m_sprPreview[MAX_PREVIEW_PARTS][MAX_CHOICES][NUM_PLAYERS];
#define MAX_CURSOR_PARTS 3
	// cursor is the per-player that's shared by all choices
	Sprite	m_sprCursor[MAX_CURSOR_PARTS][NUM_PLAYERS];
	// scroll is the per-player, per-choice piece that's scrolled
	AutoActor	m_sprScroll[MAX_CHOICES][NUM_PLAYERS];
	ActorScroller	m_Scroller[NUM_PLAYERS];

	RageSound	m_soundChange;
	RandomSample m_soundDifficult;

	int m_iChoice[NUM_PLAYERS];
	bool m_bChosen[NUM_PLAYERS];

	float m_fLockInputSecs;
};

//
// Aliases
//
class ScreenSelectStyle5th : public ScreenSelectMaster
{
public:
	ScreenSelectStyle5th( CString sName ): ScreenSelectMaster( sName ) { }
};


#endif
