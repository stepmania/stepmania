/*
-----------------------------------------------------------------------------
 Class: ScreenNameEntry

 Desc: Enter you name for a new high score.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "SongSelector.h"
#include "BitmapText.h"
#include "TransitionFade.h"
#include "RandomSample.h"
#include "GrayArrowRow.h"

const int MAX_COLS_IN_NAME_ENTRY = 12;

class ScreenNameEntry : public Screen
{
public:
	ScreenNameEntry();
	virtual ~ScreenNameEntry();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

private:

	BGAnimation		m_Background;

	GrayArrowRow	m_GrayArrowRow[NUM_PLAYERS];
	BitmapText		m_textSelectedChars[NUM_PLAYERS][MAX_COLS_IN_NAME_ENTRY];
	BitmapText		m_textScrollingChars[NUM_PLAYERS][MAX_COLS_IN_NAME_ENTRY];
	BitmapText		m_textCategory[NUM_PLAYERS];
	MenuTimer		m_Timer;

	TransitionFade	m_Fade;

	RageSound		m_soundStep;

	float			m_fFakeBeat;
	bool			m_bConfirmedName[NUM_PLAYERS];
};



