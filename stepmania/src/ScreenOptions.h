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
#include "TransitionInvisible.h"
#include "Quad.h"
#include "MenuElements.h"
#include "OptionsCursor.h"
#include "OptionIcon.h"


const unsigned MAX_OPTION_LINES = 20;
const unsigned MAX_OPTIONS_PER_LINE = 20;


// used to pass menu info into this class
struct OptionRowData {
	char szTitle[30];
	unsigned iNumOptions;
	char szOptionsText[MAX_OPTIONS_PER_LINE][60];
	char szExplanation[2048];
};

enum InputMode 
{ 
	INPUTMODE_PLAYERS, 	// each player controls their own cursor
	INPUTMODE_BOTH		// both players control the same cursor
};


class ScreenOptions : public Screen
{
public:
	ScreenOptions( CString sBackgroundPath, CString sPagePath, CString sTopEdgePath );
	void Init( InputMode im, OptionRowData OptionRowData[], int iNumOptionLines, bool bUseIcons );
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
	void UpdateEnabledDisabled();
	virtual void OnChange();

	void MenuBack( PlayerNumber pn );
	void MenuStart( PlayerNumber pn );

	virtual void GoToNextState() = 0;
	virtual void GoToPrevState() = 0;

	void MenuLeft( PlayerNumber pn );
	void MenuRight( PlayerNumber pn );
	void MenuUp( PlayerNumber pn );
	void MenuDown( PlayerNumber pn );

	InputMode		m_InputMode;
	bool			m_bUseIcons;

	OptionRowData*	m_OptionRowData;
	int				m_iNumOptionRows;

	MenuElements	m_Menu;

	ActorFrame		m_framePage;
	Sprite			m_sprPage;
	Sprite			m_sprLineArrows[MAX_OPTION_LINES];
	BitmapText		m_textOptionLineTitles[MAX_OPTION_LINES];
	BitmapText		m_textOptions[MAX_OPTION_LINES][MAX_OPTIONS_PER_LINE];	// this array has to be big enough to hold all of the options
	bool			m_OptionDim[MAX_OPTION_LINES][MAX_OPTIONS_PER_LINE];
	void DimOption(int line, int option, bool dim);
	bool RowCompletelyDimmed(int line) const;

	int m_iSelectedOption[NUM_PLAYERS][MAX_OPTION_LINES];
	int m_iCurrentRow[NUM_PLAYERS];

//	Quad m_OptionUnderline[NUM_PLAYERS][MAX_OPTION_LINES];
//	Quad m_SelectionHighlight[NUM_PLAYERS];

	OptionsCursor	m_Underline[NUM_PLAYERS][MAX_OPTION_LINES];
	OptionIcon		m_OptionIcons[NUM_PLAYERS][MAX_OPTION_LINES];
	OptionsCursor	m_Highlight[NUM_PLAYERS];

	BitmapText		m_textExplanation;

	RageSoundSample	m_SoundChangeCol;
	RageSoundSample	m_SoundNextRow;
	RageSoundSample	m_SoundPrevRow;
	RageSoundSample	m_SoundStart;

	TransitionInvisible		m_Wipe;
};


#endif
