#pragma once
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


const int MAX_OPTION_LINES = 20;
const int MAX_OPTIONS_PER_LINE = 20;


// used to pass menu info into this class
struct OptionLineData {
	char szTitle[30];
	int iNumOptions;
	char szOptionsText[MAX_OPTIONS_PER_LINE][30];
};

enum InputMode { INPUTMODE_P1_ONLY, INPUTMODE_2PLAYERS, INPUTMODE_BOTH };	// both means both players control the same cursor


class ScreenOptions : public Screen
{
public:
	ScreenOptions( CString sBackgroundPath, CString sTopEdgePath );
	void Init( InputMode im, OptionLineData optionLineData[], int iNumOptionLines );
	virtual ~ScreenOptions();
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	virtual void ImportOptions() = 0;
	virtual void ExportOptions() = 0;
	void InitOptionsText();
	void GetWidthXY( PlayerNumber p, int iRow, float &fWidthOut, float &fXOut, float &fYOut );
	void PositionUnderlines();
	void PositionHighlights();
	void TweenHighlight( PlayerNumber player_no );
	void OnChange();

	void MenuBack( PlayerNumber p );
	void MenuStart( PlayerNumber p );

	virtual void GoToNextState() = 0;
	virtual void GoToPrevState() = 0;

	void MenuLeft( PlayerNumber p );
	void MenuRight( PlayerNumber p );
	void MenuUp( PlayerNumber p );
	void MenuDown( PlayerNumber p );

	InputMode m_InputMode;

	OptionLineData* m_OptionLineData;
	int m_iNumOptionLines;

	MenuElements m_Menu;

	BitmapText	m_textOptionLineTitles[MAX_OPTION_LINES];
	BitmapText	m_textOptions[MAX_OPTION_LINES][MAX_OPTIONS_PER_LINE];	// this array has to be big enough to hold all of the options

	int m_iSelectedOption[NUM_PLAYERS][MAX_OPTION_LINES];
	int m_iCurrentRow[NUM_PLAYERS];

	Quad m_OptionUnderline[NUM_PLAYERS][MAX_OPTION_LINES];
	Quad m_SelectionHighlight[NUM_PLAYERS];

	RandomSample m_SoundChange;
	RandomSample m_SoundNext;

	TransitionInvisible		m_Wipe;
};

