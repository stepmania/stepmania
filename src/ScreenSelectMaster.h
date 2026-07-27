#ifndef ScreenSelectMaster_H
#define ScreenSelectMaster_H

#include "ScreenSelect.h"
#include "RageSound.h"
#include "RandomSample.h"
#include "ActorUtil.h"
#include "ActorScroller.h"

#include <vector>


enum MenuDir
{
	MenuDir_Up,
	MenuDir_Down,
	MenuDir_Left,
	MenuDir_Right,
	MenuDir_Auto, // when players join and the selection becomes invalid
	NUM_MenuDir,
};
/** @brief A special foreach loop through the different menu directions. */
#define FOREACH_MenuDir( md ) FOREACH_ENUM( MenuDir, md )
const RString& MenuDirToString( MenuDir md );

/** @brief The master Screen for many children Screens. */
class ScreenSelectMaster : public ScreenSelect
{
public:
	ScreenSelectMaster();
	//~ScreenSelectMaster();
	virtual void Init();
	virtual RString GetDefaultChoice();
	virtual void BeginScreen();

	virtual bool MenuLeft( const InputEventPlus &input );
	virtual bool MenuRight( const InputEventPlus &input );
	virtual bool MenuUp( const InputEventPlus &input );
	virtual bool MenuDown( const InputEventPlus &input );
	virtual bool MenuStart( const InputEventPlus &input );
	virtual void TweenOnScreen();
	virtual void TweenOffScreen();

	virtual void HandleScreenMessage( const ScreenMessage SM );
	virtual void HandleMessage( const Message &msg );
	virtual bool AllowLateJoin() const { return true; }

	// sm-ssc additions:
	int GetPlayerSelectionIndex(PlayerNumber pn){ return GetSelectionIndex(pn); }

	// Lua
	virtual void PushSelf( lua_State *L );

protected:
	enum Page { PAGE_1, PAGE_2, NUM_Page };	// on PAGE_2, cursors are locked together
	static PlayerNumber GetSharedPlayer();
	Page GetPage( int iChoiceIndex ) const;
	Page GetCurrentPage() const;

	ThemeMetric<bool>	DO_SWITCH_ANYWAYS;
	ThemeMetric<bool>	DOUBLE_PRESS_TO_SELECT;
	ThemeMetric<bool>	SHOW_ICON;
	ThemeMetric<bool>	SHOW_SCROLLER;
	ThemeMetric<bool>	SHOW_CURSOR;
	ThemeMetric<bool>	SHARED_SELECTION;
	ThemeMetric<bool>	USE_ICON_METRICS;
	ThemeMetric<int>	NUM_CHOICES_ON_PAGE_1;
	ThemeMetric1D<float>	CURSOR_OFFSET_X_FROM_ICON;
	ThemeMetric1D<float>	CURSOR_OFFSET_Y_FROM_ICON;
	ThemeMetric<bool>	PER_CHOICE_ICON_ELEMENT;
	ThemeMetric<float>	PRE_SWITCH_PAGE_SECONDS;
	ThemeMetric<float>	POST_SWITCH_PAGE_SECONDS;
	ThemeMetric1D<RString>	OPTION_ORDER;
	ThemeMetric<bool>	WRAP_CURSOR;
	ThemeMetric<bool>	WRAP_SCROLLER;
	ThemeMetric<bool>	LOOP_SCROLLER;
	ThemeMetric<bool>	PER_CHOICE_SCROLL_ELEMENT;
	ThemeMetric<bool>	ALLOW_REPEATING_INPUT;
	ThemeMetric<float>	SCROLLER_SECONDS_PER_ITEM;
	ThemeMetric<float>	SCROLLER_NUM_ITEMS_TO_DRAW;
	ThemeMetric<LuaReference> SCROLLER_TRANSFORM;
	//ThemeMetric<LuaReference> SCROLLER_TWEEN;
	ThemeMetric<int>	SCROLLER_SUBDIVISIONS;
	ThemeMetric<RString>	DEFAULT_CHOICE;

	std::map<int, int> m_mapCurrentChoiceToNextChoice[NUM_MenuDir];

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

	AutoActor	m_sprExplanation[NUM_Page];
	AutoActor	m_sprMore[NUM_Page];
	// icon is the shared, per-choice piece
	std::vector<AutoActor> m_vsprIcon;

	// preview is per-player, per-choice piece
	std::vector<AutoActor> m_vsprScroll[NUM_PLAYERS];

	ActorScroller	m_Scroller[NUM_PLAYERS];

	// cursor is the per-player, shared by all choices
	AutoActor	m_sprCursor[NUM_PLAYERS];

	RageSound	m_soundChange;
	RandomSample m_soundDifficult;
	RageSound	m_soundStart;

	int m_iChoice[NUM_PLAYERS];
	bool m_bChosen[NUM_PLAYERS];
	bool m_bDoubleChoice[NUM_PLAYERS];
	bool m_bDoubleChoiceNoSound;

	GameButton m_TrackingRepeatingInput;
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
