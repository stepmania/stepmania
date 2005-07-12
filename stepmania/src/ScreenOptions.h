/* ScreenOptions - A grid of options; the selected option is drawn with a highlight rectangle. */

#ifndef SCREENOPTIONS_H
#define SCREENOPTIONS_H

#include "ScreenWithMenuElements.h"
#include "Sprite.h"
#include "RandomSample.h"
#include "DualScrollBar.h"
#include "ThemeMetric.h"
#include "OptionRow.h"

class OptionRowHandler;

AutoScreenMessage( SM_ExportOptions )

enum InputMode 
{ 
	INPUTMODE_INDIVIDUAL, 	// each player controls their own cursor
	INPUTMODE_SHARE_CURSOR		// both players control the same cursor
};

#define FOREACH_OptionsPlayer( pn ) \
	for( PlayerNumber pn=GetNextHumanPlayer((PlayerNumber)-1); pn!=PLAYER_INVALID && (m_InputMode==INPUTMODE_INDIVIDUAL || pn==0); pn=GetNextHumanPlayer(pn) )

class ScreenOptions : public ScreenWithMenuElements
{
public:
	ScreenOptions( CString sClassName );
	virtual void Init();
	void InitMenu( InputMode im, const vector<OptionRowDefinition> &vDefs, const vector<OptionRowHandler*> &vHands );
	virtual ~ScreenOptions();
	virtual void Update( float fDeltaTime );
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	virtual void ImportOptions( int row, const vector<PlayerNumber> &vpns ) = 0;
	virtual void ExportOptions( int row, const vector<PlayerNumber> &vpns ) = 0;

	void InitOptionsText();
	void GetWidthXY( PlayerNumber pn, int iRow, int iChoiceOnRow, int &iWidthOut, int &iXOut, int &iYOut );
	CString GetExplanationText( int row ) const;
	BitmapText &GetTextItemForRow( PlayerNumber pn, int iRow, int iChoiceOnRow );
	void PositionUnderlines( int row, PlayerNumber pn );
	void PositionAllUnderlines();
	void PositionIcons();
	virtual void RefreshIcons( int row, PlayerNumber pn );
	void RefreshAllIcons();
	void PositionCursors();
	void PositionItems();
	void TweenCursor( PlayerNumber pn );
	void UpdateText( int row );
	void UpdateEnabledDisabled();
	void UpdateEnabledDisabled( int row );
	virtual void OnChange( PlayerNumber pn );

	virtual void MenuBack( PlayerNumber pn );
	virtual void MenuStart( PlayerNumber pn, const InputEventType type );
	virtual void ProcessMenuStart( PlayerNumber pn, const InputEventType type );

	virtual void BeginFadingOut() { this->PostScreenMessage( SM_BeginFadingOut, 0 ); }
	virtual void GoToNextScreen() { }
	virtual void GoToPrevScreen() { }

	void ChangeValueInRow( PlayerNumber pn, int iDelta, bool Repeat );
	virtual void AfterChangeValueInRow( PlayerNumber pn ) {}	// override this to detect when the value in a row has changed
	void MoveRow( PlayerNumber pn, int dir, bool Repeat );

	void MenuLeft( PlayerNumber pn, const InputEventType type )		{ ChangeValueInRow(pn,-1,type != IET_FIRST_PRESS); }
	void MenuRight( PlayerNumber pn, const InputEventType type )	{ ChangeValueInRow(pn,+1,type != IET_FIRST_PRESS); }
	void MenuUp( PlayerNumber pn, const InputEventType type );
	void MenuDown( PlayerNumber pn, const InputEventType type );
	void MenuSelect( PlayerNumber pn, const InputEventType type );
	void MenuUpDown( PlayerNumber pn, const InputEventType type, int iDir );	// iDir == -1 or iDir == +1

	/* Returns -1 if on a row with no OptionRowDefinition (eg. EXIT). */
	int GetCurrentRow(PlayerNumber pn = PLAYER_1) const;
	bool IsOnLastRow( PlayerNumber pn ) const;
	bool AllAreOnLastRow() const;

protected:	// derived classes need access to these
	void LoadOptionIcon( PlayerNumber pn, int iRow, CString sText );
	enum Navigation { NAV_THREE_KEY, NAV_THREE_KEY_MENU, NAV_FIVE_KEY, NAV_TOGGLE_THREE_KEY, NAV_TOGGLE_FIVE_KEY };
	void SetNavigation( Navigation nav ) { m_OptionsNavigation = nav; }

protected:
	/* Map menu lines to m_OptionRow entries. */
	vector<OptionRow*>	m_pRows;

	Navigation		m_OptionsNavigation;

	int				m_iCurrentRow[NUM_PLAYERS];
	int				m_iFocusX[NUM_PLAYERS];
	void StoreFocus( PlayerNumber pn );

	InputMode		m_InputMode;

	ActorFrame		m_framePage;
	AutoActor		m_sprPage;

	OptionsCursor	m_Cursor[NUM_PLAYERS];
	Sprite			m_sprLineHighlight[NUM_PLAYERS];

	BitmapText		m_textPlayerName[NUM_PLAYERS];
	BitmapText		m_textExplanation[NUM_PLAYERS];
	DualScrollBar	m_ScrollBar;

	AutoActor		m_sprMore;
	bool			m_bMoreShown, m_bWasOnExit[NUM_PLAYERS];

	// show if the current selections will disqualify a high score
	AutoActor		m_sprDisqualify[NUM_PLAYERS];

	// TRICKY: People hold Start to get to PlayerOptions, then 
	// the repeat events cause them to zip to the bottom.  So, ignore
	// Start repeat events until we've seen one first pressed event.
	bool			m_bGotAtLeastOneStartPressed[NUM_PLAYERS];

	RageSound		m_SoundChangeCol;
	RageSound		m_SoundNextRow;
	RageSound		m_SoundPrevRow;
	RageSound		m_SoundToggleOn;
	RageSound		m_SoundToggleOff;

	// metrics
	ThemeMetric<int>				NUM_ROWS_SHOWN;
	ThemeMetric1D<float>			ROW_Y;
	ThemeMetric<float>				ROW_Y_OFF_SCREEN_TOP;
	ThemeMetric<float>				ROW_Y_OFF_SCREEN_CENTER;
	ThemeMetric<float>				ROW_Y_OFF_SCREEN_BOTTOM;
	ThemeMetric1D<float>			EXPLANATION_X;
	ThemeMetric1D<float>			EXPLANATION_Y;
	ThemeMetric1D<apActorCommands>	EXPLANATION_ON_COMMAND;
	ThemeMetric<float>				EXPLANATION_TOGETHER_X;
	ThemeMetric<float>				EXPLANATION_TOGETHER_Y;
	ThemeMetric<apActorCommands>	EXPLANATION_TOGETHER_ON_COMMAND;
	ThemeMetric<bool>				SHOW_SCROLL_BAR;
	ThemeMetric<float>				SCROLL_BAR_HEIGHT;
	ThemeMetric<float>				SCROLL_BAR_TIME;
	ThemeMetric<float>				LINE_HIGHLIGHT_X;
	ThemeMetric<float>				EXPLANATION_ZOOM;
	ThemeMetric<bool>				SHOW_EXIT_ROW;
	ThemeMetric<bool>				SEPARATE_EXIT_ROW;
	ThemeMetric<float>				SEPARATE_EXIT_ROW_Y;
	ThemeMetric<bool>				SHOW_EXPLANATIONS;
	ThemeMetric<bool>				ALLOW_REPEATING_CHANGE_VALUE_INPUT;
	ThemeMetric<float>				CURSOR_TWEEN_SECONDS;

	float m_fLockInputSecs;
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
