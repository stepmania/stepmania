#ifndef SCREENENDLESSBREAK_H
#define SCREENENDLESSBREAK_H

/*
-----------------------------------------------------------------------------
 Class: ScreenEndlessBreak

 Desc: Screen for break periods during endless mode.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Kevin Slaughter
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "PrefsManager.h"
#include "Banner.h"
#include "Character.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "BitmapText.h"
#include "Transition.h"

// Sent to ScreenGameplay so it can keep all it's data ready for when we resume play.
const ScreenMessage SM_BreakInitiated	= ScreenMessage(SM_User+100);
const ScreenMessage SM_BreakCompleted	= ScreenMessage(SM_User+101);

class ScreenEndlessBreak : public Screen
{
public:
	ScreenEndlessBreak( CString sName );

	virtual void DrawPrimitives();
	virtual void Update(float fDeltaTime);
	virtual void Input(const DeviceInput &DeviceI, const InputEventType type, const GameInput *GameI, const MenuInput &MenuI, const StyleInput &StyleI);

private:
	Banner		m_sprBreakPicture;
	BitmapText	m_textTimeRemaining;
	float		m_fCountdownSecs;
	Transition	m_In;
	Transition	m_Out;
	bool		m_bExiting;
};
#endif