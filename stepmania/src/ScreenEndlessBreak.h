/* Class: ScreenEndlessBreak - Break periods during endless mode. */

#ifndef SCREENENDLESSBREAK_H
#define SCREENENDLESSBREAK_H

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

/*
 * (c) 2001-2003 Kevin Slaughter
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

