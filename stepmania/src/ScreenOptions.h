/* ScreenOptions - A grid of options; the selected option is drawn with a highlight rectangle. */

#ifndef SCREENOPTIONS_H
#define SCREENOPTIONS_H

#include "ScreenWithMenuElements.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "RandomSample.h"
#include "Quad.h"
#include "OptionsCursor.h"
#include "OptionIcon.h"
#include "DualScrollBar.h"


struct OptionRowData
{
	CString name;
	bool bOneChoiceForAllPlayers;
	bool bMultiSelect;
	vector<CString> choices;

	OptionRowData(): name(""), bOneChoiceForAllPlayers(false), bMultiSelect(false) { }

	OptionRowData( const char *n, int b, const char *c0=NULL, const char *c1=NULL, const char *c2=NULL, const char *c3=NULL, const char *c4=NULL, const char *c5=NULL, const char *c6=NULL, const char *c7=NULL, const char *c8=NULL, const char *c9=NULL, const char *c10=NULL, const char *c11=NULL, const char *c12=NULL, const char *c13=NULL, const char *c14=NULL, const char *c15=NULL, const char *c16=NULL, const char *c17=NULL, const char *c18=NULL, const char *c19=NULL )
	{
		name = n;
		bOneChoiceForAllPlayers = !!b;
		bMultiSelect = false;
#define PUSH( c )	if(c) choices.push_back(c);
		PUSH(c0);PUSH(c1);PUSH(c2);PUSH(c3);PUSH(c4);PUSH(c5);PUSH(c6);PUSH(c7);PUSH(c8);PUSH(c9);PUSH(c10);PUSH(c11);PUSH(c12);PUSH(c13);PUSH(c14);PUSH(c15);PUSH(c16);PUSH(c17);PUSH(c18);PUSH(c19);
#undef PUSH
	}
};

enum InputMode 
{ 
	INPUTMODE_INDIVIDUAL, 	// each player controls their own cursor
	INPUTMODE_SHARE_CURSOR		// both players control the same cursor
};


class ScreenOptions : public ScreenWithMenuElements
{
public:
	ScreenOptions( CString sClassName );
	void Init( InputMode im, OptionRowData OptionRowData[], int iNumOptionLines );
	virtual ~ScreenOptions();
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	virtual void ImportOptions() = 0;
	virtual void ExportOptions() = 0;
	void InitOptionsText();
	void GetWidthXY( PlayerNumber pn, int iRow, int iChoiceOnRow, int &iWidthOut, int &iXOut, int &iYOut );
	CString GetExplanationText( int row ) const;
	CString GetExplanationTitle( int row ) const;
	BitmapText &GetTextItemForRow( PlayerNumber pn, int iRow, int iChoiceOnRow );
	void PositionUnderlines();
	void PositionIcons();
	virtual void RefreshIcons();
	void PositionCursors();
	void PositionItems();
	void TweenCursor( PlayerNumber pn );
	void UpdateText( int row );
	void UpdateEnabledDisabled();
	virtual void OnChange( PlayerNumber pn );

	virtual void MenuBack( PlayerNumber pn );
	virtual void MenuStart( PlayerNumber pn, const InputEventType type );

	void StartGoToNextState();

	virtual void GoToNextState() = 0;
	virtual void GoToPrevState() = 0;

	void MenuLeft( PlayerNumber pn, const InputEventType type ) { ChangeValueInRow(pn,-1,type != IET_FIRST_PRESS); }
	void MenuRight( PlayerNumber pn, const InputEventType type ) { ChangeValueInRow(pn,+1,type != IET_FIRST_PRESS); }
	void ChangeValueInRow( PlayerNumber pn, int iDelta, bool Repeat );
	void MenuUp( PlayerNumber pn, const InputEventType type ) { MoveRow( pn, -1, type != IET_FIRST_PRESS ); }
	void MenuDown( PlayerNumber pn, const InputEventType type ) { MoveRow( pn, +1, type != IET_FIRST_PRESS ); }
	void MoveRow( PlayerNumber pn, int dir, bool Repeat );

	/* Returns -1 if on a row with no OptionRowData (eg. EXIT). */
	int GetCurrentRow(PlayerNumber pn = PLAYER_1) const;
	bool AllAreOnExit() const;

protected:	// derived classes need access to these
	void LoadOptionIcon( PlayerNumber pn, int iRow, CString sText );
	enum Navigation { NAV_THREE_KEY, NAV_THREE_KEY_MENU, NAV_FIVE_KEY, NAV_TOGGLE_THREE_KEY, NAV_TOGGLE_FIVE_KEY };
	void SetNavigation( Navigation nav ) { m_OptionsNavigation = nav; }

protected:
	/* Map menu lines to m_OptionRow entries. */
	struct Row
	{
		Row();
		~Row();
		OptionRowData				m_RowDef;
		enum { ROW_NORMAL, ROW_EXIT } Type;
		vector<BitmapText *>	m_textItems;				// size depends on m_bRowIsLong and which players are joined
		vector<OptionsCursor *>	m_Underline[NUM_PLAYERS];	// size depends on m_bRowIsLong and which players are joined
		Sprite					m_sprBullet;
		BitmapText				m_textTitle;
		OptionIcon				m_OptionIcons[NUM_PLAYERS];

		float m_fY;
		bool m_bRowIsLong;	// goes off edge of screen
		bool m_bHidden; // currently off screen

		int m_iChoiceWithFocus[NUM_PLAYERS];	// this choice has input focus

		// Only one will true at a time if m_RowDef.bMultiSelect
		vector<bool> m_vbSelected[NUM_PLAYERS];	// size = m_RowDef.choices.size().
		int GetOneSelection( PlayerNumber pn ) const
		{
			for( unsigned i=0; i<(unsigned)m_vbSelected[pn].size(); i++ )
				if( m_vbSelected[pn][i] )
					return i;
			ASSERT(0);	// shouldn't call this if not expecting one to be selected
			return -1;
		}
		int GetOneSharedSelection() const
		{
			return GetOneSelection( (PlayerNumber)0 );
		}
		void SetOneSelection( PlayerNumber pn, int iChoice )
		{
			for( unsigned i=0; i<(unsigned)m_vbSelected[pn].size(); i++ )
				m_vbSelected[pn][i] = false;
			m_vbSelected[pn][iChoice] = true;
		}
		void SetOneSharedSelection( int iChoice )
		{
			FOREACH_HumanPlayer( pn )
				SetOneSelection( pn, iChoice );
		}
	};
	vector<Row*>		m_Rows;

	Navigation		m_OptionsNavigation;

	int				m_iCurrentRow[NUM_PLAYERS];
	int				m_iFocusX[NUM_PLAYERS];
	void StoreFocus( PlayerNumber pn );

	InputMode		m_InputMode;

	ActorFrame		m_framePage;
	AutoActor		m_sprPage;
	AutoActor		m_sprFrame;

	OptionsCursor	m_Highlight[NUM_PLAYERS];
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
