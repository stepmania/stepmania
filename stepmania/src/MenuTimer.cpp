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
	m_bTimerStopped = false;

	m_textDigit1.LoadFromNumbers( THEME->GetPathTo("Numbers","menu timer numbers") );
	m_textDigit2.LoadFromNumbers( THEME->GetPathTo("Numbers","menu timer numbers") );

	m_textDigit1.TurnShadowOff();
	m_textDigit2.TurnShadowOff();

	const glyph &g = m_textDigit1.m_pFont->GetGlyph('0');
	float fCharWidth = (float)g.Texture->GetSourceFrameWidth();

	m_textDigit1.SetXY( -fCharWidth/2, 0 );
	m_textDigit2.SetXY( +fCharWidth/2, 0 );

	this->AddChild( &m_textDigit1 );
	this->AddChild( &m_textDigit2 );

	m_soundBeep.Load( THEME->GetPathTo("Sounds","menu timer") );
}

void MenuTimer::StealthTimer( int iActive )
{
	if ( iActive == 0 ) // we wanna keep everything as it is...
	{
		m_soundBeep.Load( THEME->GetPathTo("Sounds","menu timer") ); // reload the sound
	}
	else if ( iActive == 1 ) // otherwise we wanna make the timer invisible and silent....
	{
		m_soundBeep.Unload(); // unload the sound
	}
	// else take no action
}

void MenuTimer::Update( float fDeltaTime ) 
{ 
	ActorFrame::Update( fDeltaTime );

	if( m_bTimerStopped )
	{
		m_textDigit1.SetText( ssprintf("%d", ((int)m_fSecondsLeft)/10) ); 
		m_textDigit2.SetText( ssprintf("%d", ((int)m_fSecondsLeft)%10) ); 

		m_textDigit1.SetZoomX( 1 ); 
		m_textDigit2.SetZoomX( 1 ); 

		return;
	}


	if( m_fStallSeconds > 0 )
	{
		m_fStallSeconds -= fDeltaTime;
		return;
	}

	float fOldSecondsLeft = m_fSecondsLeft;
	float fNewSecondsLeft = fOldSecondsLeft - fDeltaTime;

	if( fOldSecondsLeft > 5.5  &&  fNewSecondsLeft < 5.5 )	// transition to below 5.5
		SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("hurry up") );
	else if( fOldSecondsLeft > 5  &&  fNewSecondsLeft < 5 )	// transition to below 5
	{
		m_textDigit1.SetEffectGlowCamelion( 0.15f, RageColor(1,0,0,0), RageColor(1,0,0,1) );
		m_textDigit2.SetEffectGlowCamelion( 0.15f, RageColor(1,0,0,0), RageColor(1,0,0,1) );
		m_soundBeep.Play();
	}
	else if( fOldSecondsLeft > 4  &&  fNewSecondsLeft < 4 )	// transition to below 4
		m_soundBeep.Play();
	else if( fOldSecondsLeft > 3  &&  fNewSecondsLeft < 3 )	// transition to below 3
		m_soundBeep.Play();
	else if( fOldSecondsLeft > 2  &&  fNewSecondsLeft < 2 )	// transition to below 2
		m_soundBeep.Play();
	else if( fOldSecondsLeft > 1  &&  fNewSecondsLeft < 1 )	// transition to below 1
		m_soundBeep.Play();
	else if( fOldSecondsLeft > 0  &&  fNewSecondsLeft < 0 )	// transition to below 0
		SCREENMAN->SendMessageToTopScreen( SM_MenuTimer, 0 );

	m_fSecondsLeft = fNewSecondsLeft;
	m_fSecondsLeft = max( 0, m_fSecondsLeft );

	m_textDigit1.SetText( ssprintf("%d", ((int)m_fSecondsLeft+1)/10) ); 
	m_textDigit2.SetText( ssprintf("%d", ((int)m_fSecondsLeft+1)%10) ); 

	// "flip" the numbers
	float fRemainder = m_fSecondsLeft - (int)m_fSecondsLeft;
	float fDistFromNearestNumber = min( fRemainder, 1-fRemainder );	// this is between 0 and 0.5;

	if( m_fSecondsLeft < 4.5f )
	{
		m_textDigit1.SetZoomX( min(1, fDistFromNearestNumber*8) ); 
		m_textDigit2.SetZoomX( min(1, fDistFromNearestNumber*8) ); 
	}
}


void MenuTimer::StopTimer()
{
	SetTimer( 0 );
	m_bTimerStopped = true;
}

void MenuTimer::StallTimer()
{
	m_fStallSeconds = 0.5f;
}

void MenuTimer::SetTimer( int iSeconds )
{
	m_fSecondsLeft = (float)iSeconds;
	CLAMP( m_fSecondsLeft, 0, 99 );

	m_textDigit1.SetZoomX( 1 ); 
	m_textDigit2.SetZoomX( 1 ); 
	m_textDigit1.SetEffectNone();
	m_textDigit2.SetEffectNone();
}

void MenuTimer::StartTimer()
{
	m_bTimerStopped = false;
}
