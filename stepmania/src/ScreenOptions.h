#ifndef SCREENOPTIONS_H
#define SCREENOPTIONS_H
/*
-----------------------------------------------------------------------------
 File: ScreenOptions.h

 Desc: A grid of options, and the selected option is drawn with a highlight rectangle.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Screen.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "RandomSample.h"
#include "Quad.h"
#include "MenuElements.h"
#include "OptionsCursor.h"
#include "OptionIcon.h"
#include "DualScrollBar.h"


const unsigned MAX_OPTION_LINES = 40;

struct OptionRow
{
	CString name;
	bool bOneChoiceForAllPlayers;
	vector<CString> choices;

	OptionRow(): name(""), bOneChoiceForAllPlayers(false) { }

	OptionRow( const char *n, int b, const char *c0=NULL, const char *c1=NULL, const char *c2=NULL, const char *c3=NULL, const char *c4=NULL, const char *c5=NULL, const char *c6=NULL, const char *c7=NULL, const char *c8=NULL, const char *c9=NULL, const char *c10=NULL, const char *c11=NULL, const char *c12=NULL, const char *c13=NULL, const char *c14=NULL, const char *c15=NULL, const char *c16=NULL, const char *c17=NULL, const char *c18=NULL, const char *c19=NULL )
	{
		name = n;
		bOneChoiceForAllPlayers = !!b;
#define PUSH( c )	if(c) choices.push_back(c);
		PUSH(c0);PUSH(c1);PUSH(c2);PUSH(c3);PUSH(c4);PUSH(c5);PUSH(c6);PUSH(c7);PUSH(c8);PUSH(c9);PUSH(c10);PUSH(c11);PUSH(c12);PUSH(c13);PUSH(c14);PUSH(c15);PUSH(c16);PUSH(c17);PUSH(c18);PUSH(c19);
#undef PUSH
	}
};

enum InputMode 
{ 
	INPUTMODE_INDIVIDUAL, 	// each player controls their own cursor
	INPUTMODE_TOGETHER		// both players control the same cursor
};


class ScreenOptions : public Screen
{
public:
	ScreenOptions( CString sClassName );
	void Init( InputMode im, OptionRow OptionRow[], int iNumOptionLines );
	virtual ~ScreenOptions();
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	virtual void ImportOptions() = 0;
	virtual void ExportOptions() = 0;
	void InitOptionsText();
	void GetWidthXY( PlayerNumber pn, int iRow, int &iWidthOut, int &iXOut, int &iYOut );
	CString GetExplanationText( int row ) const;
	CString GetExplanationTitle( int row ) const;
	BitmapText &GetTextItemForRow( PlayerNumber pn, int iRow );
	void PositionUnderlines();
	void PositionIcons();
	virtual void RefreshIcons();
	void PositionCursors();
	void PositionItems();
	void TweenCursor( PlayerNumber player_no );
	void UpdateText( PlayerNumber player_no, int row );
	void UpdateEnabledDisabled();
	virtual void OnChange( PlayerNumber pn );

	virtual void MenuBack( PlayerNumber pn );
	virtual void MenuStart( PlayerNumber pn, const InputEventType type );

	void StartGoToNextState();

	virtual void GoToNextState() = 0;
	virtual void GoToPrevState() = 0;

	void MenuLeft( PlayerNumber pn, const InputEventType type ) { ChangeValue(pn,-1,type != IET_FIRST_PRESS); }
	void MenuRight( PlayerNumber pn, const InputEventType type ) { ChangeValue(pn,+1,type != IET_FIRST_PRESS); }
	void ChangeValue( PlayerNumber pn, int iDelta, bool Repeat );
	void MenuUp( PlayerNumber pn, const InputEventType type ) { Move( pn, -1, type != IET_FIRST_PRESS ); }
	void MenuDown( PlayerNumber pn, const InputEventType type ) { Move( pn, +1, type != IET_FIRST_PRESS ); }
	void Move( PlayerNumber pn, int dir, bool Repeat );

	int GetCurrentRow(PlayerNumber pn = PLAYER_1) const { return m_iCurrentRow[pn]; }

	MenuElements	m_Menu;
	OptionRow*		m_OptionRow;

protected:	// derived classes need access to these
	int m_iSelectedOption[NUM_PLAYERS][MAX_OPTION_LINES];
	int m_iCurrentRow[NUM_PLAYERS];

	InputMode		m_InputMode;

	int				m_iNumOptionRows;

	ActorFrame		m_framePage;
	AutoActor		m_sprPage;
	AutoActor		m_sprFrame;
	Sprite			m_sprBullets[MAX_OPTION_LINES];
	BitmapText		m_textTitles[MAX_OPTION_LINES];
	vector<BitmapText *>	m_textItems[MAX_OPTION_LINES];

	bool m_bRowIsLong[MAX_OPTION_LINES];	// goes off edge of screen

	/* True if the item is off of the screen. */
	bool m_bRowIsHidden[MAX_OPTION_LINES];
	float m_fRowY[MAX_OPTION_LINES];


	OptionsCursor	m_Underline[NUM_PLAYERS][MAX_OPTION_LINES];
	OptionIcon		m_OptionIcons[NUM_PLAYERS][MAX_OPTION_LINES];
	OptionsCursor	m_Highlight[NUM_PLAYERS];
	Sprite			m_sprLineHighlight[NUM_PLAYERS];

	BitmapText		m_textPlayerName[NUM_PLAYERS];
	BitmapText		m_textExplanation[NUM_PLAYERS];
	DualScrollBar	m_ScrollBar;

	RageSound		m_SoundChangeCol;
	RageSound		m_SoundNextRow;
	RageSound		m_SoundPrevRow;
	RageSound		m_SoundStart;
};


#endif
