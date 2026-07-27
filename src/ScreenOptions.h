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

#include <vector>


class OptionRowHandler;

AutoScreenMessage( SM_ExportOptions );

/** @brief The list of input modes for the given row. */
enum InputMode
{
	INPUTMODE_INDIVIDUAL,		/**< each player controls their own cursor */
	INPUTMODE_SHARE_CURSOR,		/**< both players control the same cursor */
	NUM_InputMode,			/**< The number of input modes available. */
	InputMode_Invalid
};
InputMode StringToInputMode( const RString& str );

/** @brief A custom foreach loop for the player options for each player. */
#define FOREACH_OptionsPlayer( pn ) \
	for( PlayerNumber pn=GetNextHumanPlayer((PlayerNumber)-1); \
	pn!=PLAYER_INVALID && (m_InputMode==INPUTMODE_INDIVIDUAL || pn==0); \
	pn=GetNextHumanPlayer(pn) )

/** @brief A grid of options; the selected option is drawn with a highlight rectangle. */
class ScreenOptions : public ScreenWithMenuElements
{
public:
	ScreenOptions();
	virtual void Init();
	virtual void BeginScreen();
	void InitMenu( const std::vector<OptionRowHandler*> &vHands );
	virtual ~ScreenOptions();
	virtual void Update( float fDeltaTime );
	virtual bool Input( const InputEventPlus &input );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void TweenOnScreen();
	virtual void TweenOffScreen();

	// Lua
	virtual void PushSelf( lua_State *L );
	friend class LunaScreenOptions;

protected:
	virtual void ImportOptions( int iRow, const std::vector<PlayerNumber> &vpns ) = 0;
	virtual void ExportOptions( int iRow, const std::vector<PlayerNumber> &vpns ) = 0;

	void RestartOptions();
	void GetWidthXY( PlayerNumber pn, int iRow, int iChoiceOnRow,
			int &iWidthOut, int &iXOut, int &iYOut ) const;
	RString GetExplanationText( int iRow ) const;
	void RefreshIcons( int iRow, PlayerNumber pn );
	void PositionCursor( PlayerNumber pn );
	void PositionRows( bool bTween );
	void TweenCursor( PlayerNumber pn );
	void StoreFocus( PlayerNumber pn );

	void BeginFadingOut();
	virtual bool FocusedItemEndsScreen( PlayerNumber pn ) const;
	RString GetNextScreenForFocusedItem( PlayerNumber pn ) const;

	void ChangeValueInRowRelative( int iRow, PlayerNumber pn, int iDelta, bool bRepeat );
	void ChangeValueInRowAbsolute( int iRow, PlayerNumber pn, int iChoiceIndex, bool bRepeat );
	/**
	 * @brief Perform an action after a row has changed its value.
	 *
	 * Override this to detect when the value in a row has changed. */
	virtual void AfterChangeValueInRow( int iRow, PlayerNumber pn );
	bool MoveRowRelative( PlayerNumber pn, int iDir, bool bRepeat );
	bool MoveRowAbsolute( PlayerNumber pn, int iRow );
	/**
	 * @brief Perform an action after moving to a new row.
	 *
	 * Override this to detect when the row has changed. */
	virtual void AfterChangeRow( PlayerNumber pn );
	virtual void AfterChangeValueOrRow( PlayerNumber pn );

	virtual bool MenuBack( const InputEventPlus &input );
	virtual bool MenuStart( const InputEventPlus &input );
	virtual void ProcessMenuStart( const InputEventPlus &input );
	virtual bool MenuLeft( const InputEventPlus &input );
	virtual bool MenuRight( const InputEventPlus &input );
	virtual bool MenuUp( const InputEventPlus &input );
	virtual bool MenuDown( const InputEventPlus &input );
	virtual bool MenuSelect( const InputEventPlus &input );
	virtual void MenuUpDown( const InputEventPlus &input, int iDir );	// iDir == -1 or iDir == +1

	int GetCurrentRow( PlayerNumber pn = PLAYER_1 ) const { return m_iCurrentRow[pn]; }
	bool AllAreOnLastRow() const;
	OptionRow* GetRow( int iRow ) const { return m_pRows[iRow]; }
        //void SetOptionRowFromName( const RString& nombre );
	int GetNumRows() const { return static_cast<int>(m_pRows.size()); }

protected:	// derived classes need access to these
	enum Navigation { NAV_THREE_KEY, NAV_THREE_KEY_MENU, NAV_THREE_KEY_ALT, NAV_FIVE_KEY, NAV_TOGGLE_THREE_KEY, NAV_TOGGLE_FIVE_KEY };
	void SetNavigation( Navigation nav ) { m_OptionsNavigation = nav; }
	void SetInputMode( InputMode im ) { m_InputMode = im; }

	/** @brief Map menu lines to m_OptionRow entries. */
	std::vector<OptionRow*>	m_pRows;
	/** @brief The current row each player is on. */
	int			m_iCurrentRow[NUM_PLAYERS];

	OptionRowType	m_OptionRowTypeNormal;
	OptionRowType	m_OptionRowTypeExit;

	Navigation		m_OptionsNavigation;
	InputMode		m_InputMode;

	int				m_iFocusX[NUM_PLAYERS];
	bool			m_bWasOnExit[NUM_PLAYERS];

	/** @brief True if at least one player pressed Start after selecting the song.
	 *
	 * TRICKY: People hold Start to get to PlayerOptions, then the repeat events
	 * cause them to zip to the bottom. So, ignore Start repeat events until
	 * we've seen one first pressed event. */
	bool			m_bGotAtLeastOneStartPressed[NUM_PLAYERS];

	// actors
	ActorFrame		m_frameContainer;
	AutoActor		m_sprPage;

	OptionsCursor	m_Cursor[NUM_PLAYERS];
	AutoActor		m_sprLineHighlight[NUM_PLAYERS];

	BitmapText		m_textExplanation[NUM_PLAYERS];
	BitmapText		m_textExplanationTogether;
	DualScrollBar	m_ScrollBar;
	AutoActor		m_sprMore;

	RageSound		m_SoundChangeCol;
	RageSound		m_SoundNextRow;
	RageSound		m_SoundPrevRow;
	RageSound		m_SoundToggleOn;
	RageSound		m_SoundToggleOff;
	RageSound		m_SoundStart;

	// metrics
	ThemeMetric<int>		NUM_ROWS_SHOWN;
	ThemeMetric<apActorCommands>	ROW_INIT_COMMAND;
	ThemeMetric<apActorCommands>	ROW_ON_COMMAND;
	ThemeMetric<apActorCommands>	ROW_OFF_COMMAND;
	LuaExpressionTransform	m_exprRowPositionTransformFunction;	// params: self,offsetFromCenter,itemIndex,numItems
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
	ThemeMetric<RString>	OPTION_ROW_NORMAL_METRICS_GROUP;
	ThemeMetric<RString>	OPTION_ROW_EXIT_METRICS_GROUP;
};

#endif

/**
 * @file
 * @author Chris Danford, Glenn Maynard (c) 2001-2004
 * @section LICENSE
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
