#include "global.h"
#include "MenuTimer.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "ScreenManager.h"
#include "AnnouncerManager.h"
#include "ThemeManager.h"
#include "Font.h"
#include "GameSoundManager.h"
#include "ThemeMetric.h"
#include "ActorUtil.h"

RString WARNING_COMMAND_NAME( size_t i ) { return ssprintf("Warning%dCommand",int(i)); }

static const ThemeMetric<int>		WARNING_START		("MenuTimer","WarningStart");
static const ThemeMetric<int>		WARNING_BEEP_START	("MenuTimer","WarningBeepStart");
static const ThemeMetric<float>		MAX_STALL_SECONDS	("MenuTimer","MaxStallSeconds");

static const float TIMER_PAUSE_SECONDS = 99.99f;

MenuTimer::MenuTimer() :
	WARNING_COMMAND("MenuTimer", WARNING_COMMAND_NAME, WARNING_START+1)
{
	m_fStallSeconds = 0;
	m_fStallSecondsLeft = MAX_STALL_SECONDS;
	m_bPaused = false;
}

void MenuTimer::Load()
{
	for( int i=0; i<NUM_MENU_TIMER_TEXTS; i++ )
	{
		m_text[i].LoadFromFont( THEME->GetPathF("MenuTimer","numbers") );
		m_text[i].SetName( ssprintf("Text%d",i+1) );
		ActorUtil::OnCommand( m_text[i], "MenuTimer" );
		this->AddChild( &m_text[i] );
	}

	m_exprFormatText[0] = THEME->GetMetricR("MenuTimer", "Text1FormatFunction");
	m_exprFormatText[1] = THEME->GetMetricR("MenuTimer", "Text2FormatFunction");

	SetSeconds( TIMER_PAUSE_SECONDS );

	m_soundBeep.Load( THEME->GetPathS("MenuTimer","tick") );
}

void MenuTimer::EnableStealth( bool bStealth )
{
	if( bStealth )
		m_soundBeep.Unload(); // unload the sound
	else
		m_soundBeep.Load( THEME->GetPathS("MenuTimer","tick") ); // reload the sound

	for( int i=0; i<NUM_MENU_TIMER_TEXTS; i++ )
	{
		m_text[i].SetHidden( bStealth );
	}
}

void MenuTimer::Update( float fDeltaTime ) 
{ 
	ActorFrame::Update( fDeltaTime );

	if( m_bPaused )
		return;

	// run down the stall time if any
	if( m_fStallSeconds > 0 )
		m_fStallSeconds = max( m_fStallSeconds - fDeltaTime, 0 );
	if( m_fStallSeconds > 0 )
		return;

	const float fOldSecondsLeft = m_fSecondsLeft;
	m_fSecondsLeft -= fDeltaTime;
	m_fSecondsLeft = max( 0, m_fSecondsLeft );
	const float fNewSecondsLeft = m_fSecondsLeft;

	SetText( fNewSecondsLeft );

	if( fOldSecondsLeft == fNewSecondsLeft )
		return;

	if( fOldSecondsLeft > 5.5  &&  fNewSecondsLeft < 5.5 )	// transition to below 5.5
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("hurry up") );


	int iCrossed = (int)floorf(fOldSecondsLeft);
	if( fOldSecondsLeft > iCrossed && fNewSecondsLeft < iCrossed )	// crossed
	{
		if( iCrossed <= WARNING_START )
		{
			for( int i=0; i<NUM_MENU_TIMER_TEXTS; i++ )
				m_text[i].RunCommands( WARNING_COMMAND.GetValue(iCrossed) );
		}
		
		if( iCrossed <= WARNING_BEEP_START && m_soundBeep.IsLoaded() )
			m_soundBeep.Play();
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
	/* Max amount of stall time we'll use: */
	const float Amt = min( 0.5f, m_fStallSecondsLeft );

	/* Amount of stall time to add: */
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
		lua_call(L, 1, 1); 

		RString sText;
		LuaHelpers::Pop( L, sText );
		
		m_text[i].SetText( sText );
	}

	LUA->Release(L);
}

// lua start
#include "LuaBinding.h"

class LunaMenuTimer: public Luna<MenuTimer>
{
public:
	LunaMenuTimer() { LUA->Register( Register ); }

	static int setseconds( T* p, lua_State *L )		{ p->SetSeconds(FArg(1)); return 0; }
	static int pause( T* p, lua_State *L )			{ p->Pause(); return 0; }
	static void Register(lua_State *L) {
  		ADD_METHOD( setseconds );
  		ADD_METHOD( pause );

		Luna<T>::Register( L );
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
