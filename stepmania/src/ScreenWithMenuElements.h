#ifndef ScreenWithMenuElements_H
#define ScreenWithMenuElements_H

#include "Screen.h"
#include "Sprite.h"
#include "Transition.h"
#include "BGAnimation.h"
#include "ActorUtil.h"
#include "RageSound.h"
#include "MemoryCardDisplay.h"

class MenuTimer;
class HelpDisplay;

class ScreenWithMenuElements : public Screen
{
public:
	ScreenWithMenuElements( CString sName );
	virtual ~ScreenWithMenuElements();

	void StartTransitioning( ScreenMessage smSendWhenDone );
	void Back( ScreenMessage smSendWhenDone );
	bool IsTransitioning();
	bool m_bTimerEnabled;

	void StopTimer();
	void ResetTimer();

protected:
	BGAnimation			m_Background;

	AutoActor			m_autoHeader;
	Sprite				m_sprStyleIcon;
	MemoryCardDisplay	m_MemoryCardDisplay[NUM_PLAYERS];
	MenuTimer			*m_MenuTimer;
	AutoActor			m_autoFooter;
	HelpDisplay			*m_textHelp;

	Transition	m_In;
	Transition	m_Out;
	Transition	m_Back;
};

#endif

/*
 * (c) 2004 Chris Danford
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
