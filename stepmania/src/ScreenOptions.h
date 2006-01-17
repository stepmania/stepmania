/* ScreenOptions - A grid of options; the selected option is drawn with a highlight rectangle. */

#ifndef SCREENOPTIONS_H
#define SCREENOPTIONS_H

#include "ScreenWithMenuElements.h"
#include "DualScrollBar.h"
#include "ThemeMetric.h"
#include "OptionRow.h"
#include "OptionsCursor.h"
#include "LuaExpressionTransform.h"
#include "InputEventPlus.h"
#include "RageSound.h"

class OptionRowHandler;

AutoScreenMessage( SM_ExportOptions )

enum InputMode 
{ 
	INPUTMODE_INDIVIDUAL, 	// each player controls their own cursor
	INPUTMODE_SHARE_CURSOR,		// both players control the same cursor
	NUM_InputMode,
	InputMode_Invalid
};
InputMode StringToInputMode( const CString& str );

#define FOREACH_OptionsPlayer( pn ) \
	for( PlayerNumber pn=GetNextHumanPlayer((PlayerNumber)-1); pn!=PLAYER_INVALID && (m_InputMode==INPUTMODE_INDIVIDUAL || pn==0); pn=GetNextHumanPlayer(pn) )

class ScreenOptions : public ScreenWithMenuElements
{
public:
	ScreenOptions();
	virtual void Init();
	virtual void BeginScreen();
	void InitMenu( const vector<OptionRowHandler*> &vHands );
	void InitMenu( const vector<OptionRowDefinition> &vDefs, const vector<OptionRowHandler*> &vHands );
	virtual ~ScreenOptions();
	virtual void Update( float fDeltaTime );
	virtual void Input( const InputEventPlus &input );
	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	virtual void TweenOnScreen();
	virtual void TweenOffScreen();

	virtual void ImportOptions( int iRow, const vector<PlayerNumber> &vpns ) = 0;
	virtual void ExportOptions( int iRow, const vector<PlayerNumber> &vpns ) = 0;

	void GetWidthXY( PlayerNumber pn, int iRow, int iChoiceOnRow, int &iWidthOut, int &iXOut, int &iYOut ) const;
	CString GetExplanationText( int iRow ) const;
	void PositionUnderlines( int iRow, PlayerNumber pn );
	void PositionAllUnderlines();
	void PositionIcons();
	void RefreshIcons( int iRow, PlayerNumber pn );
	void RefreshAllIcons();
	void PositionCursors();
	void PositionRows();
	void TweenCursor( PlayerNumber pn );
	void UpdateEnabledDisabled();
	void UpdateEnabledDisabled( int iRow );
	void StoreFocus( PlayerNumber pn );

	void BeginFadingOut();
	CString GetNextScreenForSelection( PlayerNumber pn ) const;

	void ChangeValueInRowRelative( int iRow, PlayerNumber pn, int iDelta, bool bRepeat );
	void ChangeValueInRowAbsolute( int iRow, PlayerNumber pn, int iChoiceIndex, bool bRepeat );
	virtual void AfterChangeValueInRow( int iRow, PlayerNumber pn );	// override this to detect when the value in a row has changed
	void MoveRowRelative( PlayerNumber pn, int iDir, bool bRepeat );
	void MoveRowAbsolute( PlayerNumber pn, int iRow, bool bRepeat );
	virtual void AfterChangeRow( PlayerNumber pn );	// override this to detect when the row has changed
	virtual void AfterChangeValueOrRow( PlayerNumber pn );

	virtual void MenuBack( PlayerNumber pn );
	virtual void MenuStart( const InputEventPlus &input );
	virtual void ProcessMenuStart( const InputEventPlus &input );
	virtual void MenuLeft( const InputEventPlus &input )		{ ChangeValueInRowRelative(m_iCurrentRow[input.MenuI.player],input.MenuI.player,-1, input.type != IET_FIRST_PRESS); }
	virtual void MenuRight( const InputEventPlus &input )	{ ChangeValueInRowRelative(m_iCurrentRow[input.MenuI.player], input.MenuI.player,+1, input.type != IET_FIRST_PRESS); }
	virtual void MenuUp( const InputEventPlus &input );
	virtual void MenuDown( const InputEventPlus &input );
	virtual void MenuSelect( const InputEventPlus &input );
	virtual void MenuUpDown( const InputEventPlus &input, int iDir );	// iDir == -1 or iDir == +1

	/* Returns -1 if on a row with no OptionRowDefinition (eg. EXIT). */
	int GetCurrentRow(PlayerNumber pn = PLAYER_1) const;
	bool AllAreOnLastRow() const;

public:
	//
	// Lua
	//
	virtual void PushSelf( lua_State *L );
	friend class LunaScreenOptions;

protected:	// derived classes need access to these
	enum Navigation { NAV_THREE_KEY, NAV_THREE_KEY_MENU, NAV_FIVE_KEY, NAV_TOGGLE_THREE_KEY, NAV_TOGGLE_FIVE_KEY };
	void SetNavigation( Navigation nav ) { m_OptionsNavigation = nav; }
	void SetInputMode( InputMode im ) { m_InputMode = im; }

protected:
	/* Map menu lines to m_OptionRow entries. */
	vector<OptionRow*>	m_pRows;
	OptionRowType		m_OptionRowType;

	Navigation		m_OptionsNavigation;
	InputMode		m_InputMode;

	int			m_iCurrentRow[NUM_PLAYERS];
	int			m_iFocusX[NUM_PLAYERS];
	bool			m_bWasOnExit[NUM_PLAYERS];
	float			m_fLockInputSecs;

	// TRICKY: People hold Start to get to PlayerOptions, then 
	// the repeat events cause them to zip to the bottom.  So, ignore
	// Start repeat events until we've seen one first pressed event.
	bool			m_bGotAtLeastOneStartPressed[NUM_PLAYERS];

	ActorFrame		m_framePage;
	AutoActor		m_sprPage;

	OptionsCursorPlus	m_Cursor[NUM_PLAYERS];
	AutoActor		m_sprLineHighlight[NUM_PLAYERS];

	BitmapText		m_textExplanation[NUM_PLAYERS];
	DualScrollBar		m_ScrollBar;
	AutoActor		m_sprMore;

	RageSound		m_SoundChangeCol;
	RageSound		m_SoundNextRow;
	RageSound		m_SoundPrevRow;
	RageSound		m_SoundToggleOn;
	RageSound		m_SoundToggleOff;

	// metrics
	ThemeMetric<int>		NUM_ROWS_SHOWN;
	ThemeMetric<apActorCommands>	ROW_INIT_COMMAND;
	ThemeMetric<apActorCommands>	ROW_ON_COMMAND;
	ThemeMetric<apActorCommands>	ROW_OFF_COMMAND;
	LuaExpressionTransform m_exprRowPositionTransformFunction;		// params: self,positionIndex,itemIndex,numItems
	ThemeMetric1D<float>		EXPLANATION_X;
	ThemeMetric1D<float>		EXPLANATION_Y;
	ThemeMetric1D<apActorCommands>	EXPLANATION_ON_COMMAND;
	ThemeMetric<float>		EXPLANATION_TOGETHER_X;
	ThemeMetric<float>		EXPLANATION_TOGETHER_Y;
	ThemeMetric<apActorCommands>	EXPLANATION_TOGETHER_ON_COMMAND;
	ThemeMetric<bool>		SHOW_SCROLL_BAR;
	ThemeMetric<float>		SCROLL_BAR_HEIGHT;
	ThemeMetric<float>		SCROLL_BAR_TIME;
	ThemeMetric<float>		LINE_HIGHLIGHT_X;
	ThemeMetric<bool>		SHOW_EXIT_ROW;
	ThemeMetric<bool>		SEPARATE_EXIT_ROW;
	ThemeMetric<float>		SEPARATE_EXIT_ROW_Y;
	ThemeMetric<bool>		SHOW_EXPLANATIONS;
	ThemeMetric<bool>		ALLOW_REPEATING_CHANGE_VALUE_INPUT;
	ThemeMetric<float>		CURSOR_TWEEN_SECONDS;
	ThemeMetric<bool>		WRAP_VALUE_IN_ROW;
};

#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
