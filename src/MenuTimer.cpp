#include "global.h"
#include "MenuTimer.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "ScreenManager.h"
#include "ThemeManager.h"
#include "Font.h"
#include "GameSoundManager.h"
#include "ThemeMetric.h"
#include "ActorUtil.h"

#include <cmath>
#include <cstddef>

RString WARNING_COMMAND_NAME( std::size_t i ) { return ssprintf("Warning%dCommand",int(i)); }

static const float TIMER_PAUSE_SECONDS = 99.99f;

MenuTimer::MenuTimer()
{
	m_fStallSeconds = 0;
	m_fStallSecondsLeft = 0;
	m_bPaused = false;
	m_bSilent = false;
	WARNING_COMMAND = nullptr;
}

MenuTimer::~MenuTimer()
{
	delete WARNING_COMMAND;
}

void MenuTimer::Load( RString sMetricsGroup )
{
	m_sprFrame.Load( THEME->GetPathG(sMetricsGroup, "Frame") );
	m_sprFrame->SetName( "Frame" );
	ActorUtil::LoadAllCommandsAndSetXY( m_sprFrame, sMetricsGroup );
	this->AddChild( m_sprFrame );

	for( int i=0; i<NUM_MENU_TIMER_TEXTS; i++ )
	{
		m_text[i].LoadFromFont( THEME->GetPathF(sMetricsGroup,"numbers") );
		m_text[i].SetName( ssprintf("Text%d",i+1) );
		ActorUtil::LoadAllCommandsAndOnCommand( m_text[i], sMetricsGroup );
		this->AddChild( &m_text[i] );
	}

	m_exprFormatText[0] = THEME->GetMetricR(sMetricsGroup, "Text1FormatFunction");
	m_exprFormatText[1] = THEME->GetMetricR(sMetricsGroup, "Text2FormatFunction");

	SetSeconds( TIMER_PAUSE_SECONDS );

	m_soundBeep.Load( THEME->GetPathS(sMetricsGroup,"tick") );

	WARNING_START.Load(sMetricsGroup,"WarningStart");
	WARNING_BEEP_START.Load(sMetricsGroup,"WarningBeepStart");
	MAX_STALL_SECONDS.Load(sMetricsGroup,"MaxStallSeconds");
	HURRY_UP_TRANSITION.Load(sMetricsGroup,"HurryUpTransition");

	// clear warning commands out if they already exist. -aj
	if(WARNING_COMMAND)
		WARNING_COMMAND->Clear();

	WARNING_COMMAND = new ThemeMetric1D<apActorCommands>(sMetricsGroup, WARNING_COMMAND_NAME, WARNING_START+1);

	m_fStallSecondsLeft = MAX_STALL_SECONDS;
}

void MenuTimer::EnableStealth( bool bStealth )
{
	EnableSilent( bStealth );

	for( int i=0; i<NUM_MENU_TIMER_TEXTS; i++ )
	{
		m_text[i].SetVisible( !bStealth );
	}
}

void MenuTimer::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	if( m_bPaused )
		return;

	// run down the stall time if any
	if( m_fStallSeconds > 0 )
		m_fStallSeconds = std::max( m_fStallSeconds - fDeltaTime, 0.0f );
	if( m_fStallSeconds > 0 )
		return;

	const float fOldSecondsLeft = m_fSecondsLeft;
	m_fSecondsLeft -= fDeltaTime;
	m_fSecondsLeft = std::max( 0.0f, m_fSecondsLeft );
	const float fNewSecondsLeft = m_fSecondsLeft;

	SetText( fNewSecondsLeft );

	if( fOldSecondsLeft == fNewSecondsLeft )
		return;

	if( fOldSecondsLeft > HURRY_UP_TRANSITION  &&  fNewSecondsLeft < HURRY_UP_TRANSITION )
		SOUND->PlayOnceFromAnnouncer( "hurry up" );


	int iCrossed = std::floor(fOldSecondsLeft);
	if( fOldSecondsLeft > iCrossed && fNewSecondsLeft < iCrossed )	// crossed
	{
		if( iCrossed <= WARNING_START )
		{
			for( int i=0; i<NUM_MENU_TIMER_TEXTS; i++ )
				m_text[i].RunCommands( WARNING_COMMAND->GetValue(iCrossed) );
		}

		if( iCrossed <= WARNING_BEEP_START && m_soundBeep.IsLoaded() && !m_bSilent )
			m_soundBeep.Play(false);
	}

	if( fNewSecondsLeft == 0 )
	{
		Stop();
		SCREENMAN->PostMessageToTopScreen( SM_MenuTimer, 0 );
		for( int i=0; i<NUM_MENU_TIMER_TEXTS; i++ )
			m_text[i].StopEffect();
	}
}


void MenuTimer::Pause()
{
	m_bPaused = true;
}

void MenuTimer::Stop()
{
	/* Don't call SetSeconds if we're already at 0: let the existing tweens finish. */
	if( m_fSecondsLeft >= 1 )
		SetSeconds( 0 );
	Pause();
}

void MenuTimer::Disable()
{
	SetSeconds( TIMER_PAUSE_SECONDS );
	Pause();
}

void MenuTimer::Stall()
{
	// Max amount of stall time we'll use:
	const float Amt = std::min( 0.5f, m_fStallSecondsLeft );

	// Amount of stall time to add:
	const float ToAdd = Amt - m_fStallSeconds;

	m_fStallSeconds += ToAdd;
	m_fStallSecondsLeft -= ToAdd;
}

void MenuTimer::SetSeconds( float fSeconds )
{
	m_fSecondsLeft = fSeconds;

	for( int i=0; i<NUM_MENU_TIMER_TEXTS; i++ )
		m_text[i].PlayCommand( "On" );

	SetText( fSeconds );
}

void MenuTimer::Start()
{
	m_bPaused = false;
}

void MenuTimer::SetText( float fSeconds )
{
	Lua *L = LUA->Get();

	for( int i=0; i<NUM_MENU_TIMER_TEXTS; i++ )
	{
		// function
		m_exprFormatText[i].PushSelf( L );
		ASSERT( !lua_isnil(L, -1) );

		// 1st parameter
		LuaHelpers::Push( L, fSeconds );

		// call function with 1 argument and 1 result
		RString errorMessage = ssprintf("Error running Text%dFormatFunction: ", i+1);
		LuaHelpers::RunScriptOnStack(L, errorMessage, 1, 1, true);

		RString sText;
		LuaHelpers::Pop( L, sText );

		m_text[i].SetText( sText );
	}

	LUA->Release(L);
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the MenuTimer. */
class LunaMenuTimer: public Luna<MenuTimer>
{
public:
	static int SetSeconds( T* p, lua_State *L )	{ p->SetSeconds(FArg(1)); COMMON_RETURN_SELF; }
	static int GetSeconds( T* p, lua_State *L )	{ lua_pushnumber( L, p->GetSeconds() ); return 1; }
	static int start( T* p, lua_State *L )		{ p->Start(); COMMON_RETURN_SELF; }
	static int pause( T* p, lua_State *L )		{ p->Pause(); COMMON_RETURN_SELF; }
	static int stop( T* p, lua_State *L )			{ p->Stop(); COMMON_RETURN_SELF; }
	static int disable( T* p, lua_State *L )		{ p->Disable(); COMMON_RETURN_SELF; }
	static int silent( T* p, lua_State *L )		{ p->EnableSilent(BArg(1)); COMMON_RETURN_SELF; }
	static int stealth( T* p, lua_State *L )		{ p->EnableStealth(BArg(1)); COMMON_RETURN_SELF; }

	LunaMenuTimer()
	{
		ADD_METHOD( SetSeconds );
		ADD_METHOD( GetSeconds );
		ADD_METHOD( start );
		ADD_METHOD( pause );
		ADD_METHOD( stop );
		ADD_METHOD( disable );
		ADD_METHOD( silent );
		ADD_METHOD( stealth );
	}
};

LUA_REGISTER_DERIVED_CLASS( MenuTimer, ActorFrame )
// lua end

/*
 * (c) 2002-2004 Chris Danford
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
