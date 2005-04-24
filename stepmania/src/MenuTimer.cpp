#include "global.h"
#include "MenuTimer.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "AnnouncerManager.h"
#include "ThemeManager.h"
#include "Font.h"
#include "GameSoundManager.h"
#include "ThemeMetric.h"

CString WARNING_COMMAND_NAME( size_t i ) { return ssprintf("WarningCommand%d",i); }

static const ThemeMetric<int>				WARNING_START		("MenuTimer","WarningStart");
static const ThemeMetric<int>				WARNING_BEEP_START	("MenuTimer","WarningBeepStart");
static const ThemeMetric<float>				MAX_STALL_SECONDS	("MenuTimer","MaxStallSeconds");
static const ThemeMetric<apActorCommands>	ON_COMMAND	("MenuTimer","OnCommand");

static const float TIMER_PAUSE_SECONDS = 99;

MenuTimer::MenuTimer() :
	WARNING_COMMAND("MenuTimer", WARNING_COMMAND_NAME, WARNING_START+1)
{
	m_fStallSeconds = 0;
	m_fStallSecondsLeft = MAX_STALL_SECONDS;
	m_bPaused = false;

	m_textDigit1.LoadFromFont( THEME->GetPathF("MenuTimer","numbers") );
	m_textDigit2.LoadFromFont( THEME->GetPathF("MenuTimer","numbers") );

	const float fCharWidth = (float) m_textDigit1.m_pFont->GetLineWidthInSourcePixels(L"0");
	m_textDigit1.SetX( -fCharWidth/2 );
	m_textDigit2.SetX( +fCharWidth/2 );

	this->AddChild( &m_textDigit1 );
	this->AddChild( &m_textDigit2 );

	SetSeconds( TIMER_PAUSE_SECONDS );

	m_soundBeep.Load( THEME->GetPathS("MenuTimer","tick") );
}

void MenuTimer::EnableStealth( bool bStealth )
{
	if( bStealth )
		m_soundBeep.Unload(); // unload the sound
	else
		m_soundBeep.Load( THEME->GetPathS("MenuTimer","tick") ); // reload the sound

	m_textDigit1.SetHidden( bStealth );	
	m_textDigit2.SetHidden( bStealth );
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
	const float fNewSecondsLeft = m_fSecondsLeft;

	const int iOldDisplay = (int)(fOldSecondsLeft + 0.99f);
	const int iNewDisplay = (int)(fNewSecondsLeft + 0.99f);

	if( fOldSecondsLeft > 5.5  &&  fNewSecondsLeft < 5.5 )	// transition to below 5.5
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("hurry up") );

	if( iOldDisplay == iNewDisplay )
		return;

	SetText( iNewDisplay );

	if( iNewDisplay <= WARNING_START )
	{
		m_textDigit1.RunCommands( WARNING_COMMAND.GetValue(iNewDisplay) );
		m_textDigit2.RunCommands( WARNING_COMMAND.GetValue(iNewDisplay) );
	}
	
	if( iNewDisplay == 0 )
	{
		Stop();
		SCREENMAN->PostMessageToTopScreen( SM_MenuTimer, 0 );
		m_textDigit1.SetEffectNone();
		m_textDigit2.SetEffectNone();
	}

	if( iNewDisplay <= WARNING_BEEP_START )
	{
		if( m_soundBeep.IsLoaded() )
			m_soundBeep.Play();
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
	SetSeconds( 99 );
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

	m_textDigit1.RunCommands( ON_COMMAND );
	m_textDigit2.RunCommands( ON_COMMAND );

	SetText( (int)fSeconds );
}

void MenuTimer::Start()
{
	m_bPaused = false;
}

void MenuTimer::SetText( int iSeconds )
{
	const int iDigit1 = (int)(iSeconds)/10;
	const int iDigit2 = (int)(iSeconds)%10;

	m_textDigit1.SetText( ssprintf("%d",iDigit1) ); 
	m_textDigit2.SetText( ssprintf("%d",iDigit2) ); 
}

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
