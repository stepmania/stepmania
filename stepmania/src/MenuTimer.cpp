#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: MenuTimer

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "MenuTimer.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "AnnouncerManager.h"
#include "ThemeManager.h"
#include "Font.h"

const float TIMER_SECONDS = 99;

MenuTimer::MenuTimer()
{
	m_fSecondsLeft = TIMER_SECONDS;
	m_fStallSeconds = 0;
	m_bPaused = false;

	m_textDigit1.LoadFromNumbers( THEME->GetPathTo("Numbers","MenuTimer numbers") );
	m_textDigit2.LoadFromNumbers( THEME->GetPathTo("Numbers","MenuTimer numbers") );

	m_textDigit1.EnableShadow( false );
	m_textDigit2.EnableShadow( false );

	const glyph &g = m_textDigit1.m_pFont->GetGlyph('0');
	float fCharWidth = (float)g.Texture->GetSourceFrameWidth();

	m_textDigit1.SetXY( -fCharWidth/2, 0 );
	m_textDigit2.SetXY( +fCharWidth/2, 0 );

	this->AddChild( &m_textDigit1 );
	this->AddChild( &m_textDigit2 );

	m_soundBeep.Load( THEME->GetPathTo("Sounds","MenuTimer tick") );
}

void MenuTimer::EnableStealth( bool bStealth )
{
	if( bStealth )
	{
		m_soundBeep.Unload(); // unload the sound
		// HACK:  this is not a good way to keep the numbers from drawing...
		m_textDigit1.SetZoomY( 0 );	
		m_textDigit2.SetZoomY( 0 );
	}
	else
	{
		m_soundBeep.Load( THEME->GetPathTo("Sounds","MenuTimer tick") ); // reload the sound
		m_textDigit1.SetZoomY( 1 );	
		m_textDigit2.SetZoomY( 1 );
	}
}

void MenuTimer::Update( float fDeltaTime ) 
{ 
	ActorFrame::Update( fDeltaTime );

	if( m_bPaused )
		return;

	// run down the stall time if any
	if( m_fStallSeconds > 0 )
	{
		m_fStallSeconds -= fDeltaTime;
		return;
	}

	float fOldSecondsLeft = m_fSecondsLeft;
	m_fSecondsLeft -= fDeltaTime;
	CLAMP( m_fSecondsLeft, 0, 99 );
	float fNewSecondsLeft = m_fSecondsLeft;

	int iOldDisplay = (int)(fOldSecondsLeft + 0.99f);
	int iNewDisplay = (int)(fNewSecondsLeft + 0.99f);

	if( fOldSecondsLeft > 5.5  &&  fNewSecondsLeft < 5.5 )	// transition to below 5.5
		SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("hurry up") );

	if( iOldDisplay != iNewDisplay )
	{
		SetText( iNewDisplay );

		switch( iNewDisplay )
		{
		case 6:		// transition to below 6
			m_textDigit1.StopTweening(); 
			m_textDigit1.BeginTweening( 0.8f );	// sleep
			m_textDigit1.BeginTweening( 0.2f );
			m_textDigit1.SetTweenZoomX( 0 );
			m_textDigit2.StopTweening(); 
			m_textDigit2.BeginTweening( 0.8f );	// sleep
			m_textDigit2.BeginTweening( 0.2f );
			m_textDigit2.SetTweenZoomX( 0 );
			break;
		case 5:		// transition to below 5
			m_textDigit1.SetEffectGlowShift( 0.15f, RageColor(1,0,0,0), RageColor(1,0,0,1) );
			m_textDigit2.SetEffectGlowShift( 0.15f, RageColor(1,0,0,0), RageColor(1,0,0,1) );
			// fall through
		case 4:
		case 3:
		case 2:
		case 1:
			m_textDigit1.StopTweening(); 
			m_textDigit1.BeginTweening( 0.2f );
			m_textDigit1.SetTweenZoomX( 1 );
			m_textDigit1.BeginTweening( 0.6f );	// sleep
			m_textDigit1.BeginTweening( 0.2f );
			m_textDigit1.SetTweenZoomX( 0 );
			m_textDigit2.StopTweening(); 
			m_textDigit2.BeginTweening( 0.2f );
			m_textDigit2.SetTweenZoomX( 1 );
			m_textDigit2.BeginTweening( 0.6f );	// sleep
			m_textDigit2.BeginTweening( 0.2f );
			m_textDigit2.SetTweenZoomX( 0 );

			m_soundBeep.Play();
			break;
		case 0:
			SCREENMAN->PostMessageToTopScreen( SM_MenuTimer, 0 );
			Stop();
			break;
		}
	}
}


void MenuTimer::Pause()
{
	m_bPaused = true;
}

void MenuTimer::Stop()
{
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
	m_fStallSeconds = 0.5f;
}

void MenuTimer::SetSeconds( int iSeconds )
{
	m_fSecondsLeft = (float)iSeconds;
	CLAMP( m_fSecondsLeft, 0, 99 );

	m_textDigit1.StopTweening(); 
	m_textDigit2.StopTweening(); 
	m_textDigit1.SetZoomX( 1 ); 
	m_textDigit2.SetZoomX( 1 ); 
	m_textDigit1.SetEffectNone();
	m_textDigit2.SetEffectNone();

	SetText( iSeconds );
}

void MenuTimer::Start()
{
	m_bPaused = false;
}

void MenuTimer::SetText( int iSeconds )
{
	int iDigit1 = (int)(iSeconds)/10;
	int iDigit2 = (int)(iSeconds)%10;

	m_textDigit1.SetText( ssprintf("%d",iDigit1) ); 
	m_textDigit2.SetText( ssprintf("%d",iDigit2) ); 
}
