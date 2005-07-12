#ifndef ScreenWithMenuElements_H
#define ScreenWithMenuElements_H

#include "Screen.h"
#include "Sprite.h"
#include "Transition.h"
#include "ActorUtil.h"
#include "RageSound.h"
#include "MemoryCardDisplay.h"
#include "ThemeMetric.h"

class MenuTimer;
class HelpDisplay;

class ScreenWithMenuElements : public Screen
{
public:
	ScreenWithMenuElements( CString sName );
	virtual void Init();
	virtual ~ScreenWithMenuElements();

	void Update( float fDeltaTime );
	void StartTransitioning( ScreenMessage smSendWhenDone );
	void Cancel( ScreenMessage smSendWhenDone );
	bool IsTransitioning();
	bool m_bTimerEnabled;

	void StopTimer();
	void ResetTimer();

	void TweenOffScreen();

	//
	// Lua
	//
	virtual void PushSelf( lua_State *L );

protected:
	virtual void HandleMessage( const CString& sMessage );

	virtual void StartPlayingMusic();
	virtual void LoadHelpText();

	void UpdateStage();

	AutoActor			m_sprUnderlay;
	AutoActor			m_autoHeader;
	Sprite				m_sprStyleIcon;
	AutoActor			m_sprStage[NUM_STAGES];
	MemoryCardDisplay	m_MemoryCardDisplay[NUM_PLAYERS];
	MenuTimer			*m_MenuTimer;
	AutoActor			m_autoFooter;
	HelpDisplay			*m_textHelp;
	AutoActor			m_sprOverlay;

	Transition	m_In;
	Transition	m_Out;
	Transition	m_Cancel;

	ThemeMetric<LuaExpression>	FIRST_UPDATE_COMMAND;
	ThemeMetric<bool>			PLAY_MUSIC;
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
