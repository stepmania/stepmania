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


const unsigned MAX_OPTION_LINES = 20;
const unsigned MAX_OPTIONS_PER_LINE = 20;

struct OptionRow
{
	CString name;
	vector<CString> choices;

	OptionRow( CString n, CString c0="", CString c1="", CString c2="", CString c3="", CString c4="", CString c5="", CString c6="", CString c7="", CString c8="", CString c9="", CString c10="", CString c11="", CString c12="", CString c13="", CString c14="" )
	{
		name = n;
#define PUSH( c )	if(c!="") choices.push_back(c);
		PUSH(c0);PUSH(c1);PUSH(c2);PUSH(c3);PUSH(c4);PUSH(c5);PUSH(c6);PUSH(c7);PUSH(c8);PUSH(c9);PUSH(c10);PUSH(c11);PUSH(c12);PUSH(c13);PUSH(c14);
#undef PUSH
	}
};

enum InputMode 
{ 
	INPUTMODE_PLAYERS, 	// each player controls their own cursor
	INPUTMODE_BOTH		// both players control the same cursor
};


class ScreenOptions : public Screen
{
public:
	ScreenOptions( CString sClassName, bool bEnableTimer );
	void Init( InputMode im, OptionRow OptionRow[], int iNumOptionLines, bool bUseIcons, bool bLoadExplanations );
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
	void PositionUnderlines();
	void PositionIcons();
	void RefreshIcons();
	void PositionCursors();
	void TweenCursor( PlayerNumber player_no );
	void UpdateText( PlayerNumber player_no );
	void UpdateEnabledDisabled();
	virtual void OnChange();

	virtual void MenuBack( PlayerNumber pn );
	virtual void MenuStart( PlayerNumber pn );

	void StartGoToNextState();

	virtual void GoToNextState() = 0;
	virtual void GoToPrevState() = 0;

	void MenuLeft( PlayerNumber pn );
	void MenuRight( PlayerNumber pn );
	void MenuUp( PlayerNumber pn );
	void MenuDown( PlayerNumber pn );

	CString			m_sName;
	InputMode		m_InputMode;
	bool			m_bUseIcons;
	bool			m_bLoadExplanations;

	OptionRow*		m_OptionRow;
	int				m_iNumOptionRows;

	MenuElements	m_Menu;

	ActorFrame		m_framePage;
	Sprite			m_sprPage;
	Sprite			m_sprBullets[MAX_OPTION_LINES];
	BitmapText		m_textTitles[MAX_OPTION_LINES];
	BitmapText		m_textItems[MAX_OPTION_LINES][MAX_OPTIONS_PER_LINE];	// this array has to be big enough to hold all of the options

	bool m_bRowIsLong[MAX_OPTION_LINES];	// goes off edge of screen
	int m_iSelectedOption[NUM_PLAYERS][MAX_OPTION_LINES];
	int m_iCurrentRow[NUM_PLAYERS];

	OptionsCursor	m_Underline[NUM_PLAYERS][MAX_OPTION_LINES];
	OptionIcon		m_OptionIcons[NUM_PLAYERS][MAX_OPTION_LINES];
	OptionsCursor	m_Highlight[NUM_PLAYERS];

private:
	BitmapText		m_textExplanation;

	RageSound		m_SoundChangeCol;
	RageSound		m_SoundNextRow;
	RageSound		m_SoundPrevRow;
	RageSound		m_SoundStart;
};


#endif
