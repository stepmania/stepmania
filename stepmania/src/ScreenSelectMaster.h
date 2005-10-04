#ifndef ScreenSelectMaster_H
#define ScreenSelectMaster_H

#include "ScreenSelect.h"
#include "RageSound.h"
#include "RandomSample.h"
#include "ActorUtil.h"
#include "ActorScroller.h"
#include "MenuInput.h"

#define MAX_CHOICES 30

class ScreenSelectMaster : public ScreenSelect
{
public:
	ScreenSelectMaster( CString sName );
	virtual void Init();
	virtual void BeginScreen();

	virtual void Update( float fDelta );

	virtual void MenuLeft( const InputEventPlus &input );
	virtual void MenuRight( const InputEventPlus &input );
	virtual void MenuUp( const InputEventPlus &input );
	virtual void MenuDown( const InputEventPlus &input );
	virtual void MenuStart( PlayerNumber pn );
	void TweenOursOnScreen();
	void TweenOursOffScreen();

	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	enum Page { PAGE_1, PAGE_2, NUM_PAGES };	// on PAGE_2, cursors are locked together
	static PlayerNumber GetSharedPlayer();
	Page GetPage( int iChoiceIndex ) const;
	Page GetCurrentPage() const;

	ThemeMetric<bool>		SHOW_ICON;
	ThemeMetric<bool>		SHOW_SCROLLER;
	ThemeMetric<bool>		SHOW_CURSOR;
	ThemeMetric<bool>		SHARED_SELECTION;
	ThemeMetric<int>		NUM_CHOICES_ON_PAGE_1;
	ThemeMetric1D<float>	CURSOR_OFFSET_X_FROM_ICON;
	ThemeMetric1D<float>	CURSOR_OFFSET_Y_FROM_ICON;
	ThemeMetric<bool>		OVERRIDE_LOCK_INPUT_SECONDS;
	ThemeMetric<float>		LOCK_INPUT_SECONDS;
	ThemeMetric<float>		PRE_SWITCH_PAGE_SECONDS;
	ThemeMetric<float>		POST_SWITCH_PAGE_SECONDS;
	ThemeMetric<bool>		OVERRIDE_SLEEP_AFTER_TWEEN_OFF_SECONDS;
	ThemeMetric<float>		SLEEP_AFTER_TWEEN_OFF_SECONDS;
	ThemeMetric1D<CString>	OPTION_ORDER;
	ThemeMetric<bool>		WRAP_CURSOR;
	ThemeMetric<bool>		WRAP_SCROLLER;
	ThemeMetric<bool>		LOOP_SCROLLER;
	ThemeMetric<bool>		SCROLLER_FAST_CATCHUP;
	ThemeMetric<bool>		ALLOW_REPEATING_INPUT;
	ThemeMetric<float>		SCROLLER_SECONDS_PER_ITEM;
	ThemeMetric<float>		SCROLLER_NUM_ITEMS_TO_DRAW;
	ThemeMetric<CString>	SCROLLER_TRANSFORM;
	ThemeMetric<int>		SCROLLER_SUBDIVISIONS;
	ThemeMetric<CString>	DEFAULT_CHOICE;

	map<int,int> m_mapCurrentChoiceToNextChoice[NUM_MENU_DIRS];

	virtual int GetSelectionIndex( PlayerNumber pn );
	virtual void UpdateSelectableChoices();
	bool AnyOptionsArePlayable() const;

	bool Move( PlayerNumber pn, MenuDir dir );
	bool ChangePage( int iNewChoice );
	bool ChangeSelection( PlayerNumber pn, MenuDir dir, int iNewChoice );
	float DoMenuStart( PlayerNumber pn );
	virtual bool ProcessMenuStart( PlayerNumber pn ) { return true; }

	float GetCursorX( PlayerNumber pn );
	float GetCursorY( PlayerNumber pn );

	AutoActor	m_sprExplanation[NUM_PAGES];
	AutoActor	m_sprMore[NUM_PAGES];
	// icon is the shared, per-choice piece
	AutoActor m_sprIcon[MAX_CHOICES];
	// preview is per-player, per-choice piece
	AutoActor m_sprScroll[MAX_CHOICES][NUM_PLAYERS];
	ActorScroller	m_Scroller[NUM_PLAYERS];
	// cursor is the per-player, shared by all choices
	AutoActor	m_sprCursor[NUM_PLAYERS];

	RageSound	m_soundChange;
	RandomSample m_soundDifficult;
	RageSound	m_soundStart;

	int m_iChoice[NUM_PLAYERS];
	bool m_bChosen[NUM_PLAYERS];

	float m_fLockInputSecs;
	MenuInput m_TrackingRepeatingInput;
};


#endif

/*
 * (c) 2003-2004 Chris Danford
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
