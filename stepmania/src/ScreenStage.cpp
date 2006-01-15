#include "global.h"
#include "ScreenStage.h"
#include "ActorUtil.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "LightsManager.h"

#define MINIMUM_DELAY			THEME->GetMetricF(m_sName,"MinimumDelay")

AutoScreenMessage( SM_PrepScreen )

REGISTER_SCREEN_CLASS( ScreenStage );
ScreenStage::ScreenStage( CString sClassName ) : Screen( sClassName )
{
}

void ScreenStage::Init()
{
	Screen::Init();

	ALLOW_BACK.Load( m_sName, "AllowBack" );

	SOUND->StopMusic();

	LIGHTSMAN->SetLightsMode( LIGHTSMODE_STAGE );

	m_sprOverlay.Load( THEME->GetPathB(m_sName,"overlay") );
	m_sprOverlay->SetName( "Overlay" );
	m_sprOverlay->SetDrawOrder( DRAW_ORDER_OVERLAY );
	ON_COMMAND( m_sprOverlay );
	this->AddChild( m_sprOverlay );

	m_In.Load( THEME->GetPathB(m_sName,"in") );
	m_In.StartTransitioning();
	m_In.SetDrawOrder( DRAW_ORDER_TRANSITIONS );
	this->AddChild( &m_In );

	m_Out.Load( THEME->GetPathB(m_sName,"out") );
	m_Out.SetDrawOrder( DRAW_ORDER_TRANSITIONS );
	this->AddChild( &m_Out );

	m_Cancel.Load( THEME->GetPathB(m_sName,"cancel") );
	m_Cancel.SetDrawOrder( DRAW_ORDER_TRANSITIONS );
	this->AddChild( &m_Cancel );

	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("stage "+StageToString(GAMESTATE->GetCurrentStage())) );

	this->SortByDrawOrder();
}

void ScreenStage::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_PrepScreen )
	{
		RageTimer length;
		SCREENMAN->PrepareScreen( GetNextScreen() );
		float fScreenLoadSeconds = length.GetDeltaTime();

		/* The screen load took fScreenLoadSeconds.  Move on to the next screen after
		 * no less than MINIMUM_DELAY seconds. */
		this->PostScreenMessage( SM_BeginFadingOut, max( 0, MINIMUM_DELAY-fScreenLoadSeconds) );
		return;
	}
	else if( SM == SM_BeginFadingOut )
	{
		if( SCREENMAN->IsConcurrentlyLoading() || m_sprOverlay->GetTweenTimeLeft() )
			return;

		/* Clear any other SM_BeginFadingOut messages. */
		this->ClearMessageQueue( SM_BeginFadingOut );

		m_Out.StartTransitioning();
		this->PostScreenMessage( SM_GoToNextScreen, this->GetTweenTimeLeft() );
		return;
	}
	
	Screen::HandleScreenMessage( SM );
}

void ScreenStage::Update( float fDeltaTime )
{
	if( this->IsFirstUpdate() )
	{
		if( SCREENMAN->ConcurrentlyPrepareScreen(GetNextScreen(), SM_BeginFadingOut) )
		{
			/* Continue when both the screen finishes loading and the tween finishes. */
			this->PostScreenMessage( SM_BeginFadingOut, m_sprOverlay->GetTweenTimeLeft() );
		}
		else
		{
			/* Prep the new screen once m_In is complete. */ 	 
			this->PostScreenMessage( SM_PrepScreen, m_sprOverlay->GetTweenTimeLeft() );
		}
	}

	Screen::Update( fDeltaTime );
}

void ScreenStage::MenuBack( PlayerNumber pn )
{
	if( m_In.IsTransitioning() || m_Out.IsTransitioning() || m_Cancel.IsTransitioning() )
		return;

	if( !ALLOW_BACK )
		return;
	
	this->ClearMessageQueue();
	m_Cancel.StartTransitioning( SM_GoToPrevScreen );

	/* If a Back is buffered while we're prepping the screen (very common), we'll
	 * get it right after the prep finishes.  However, the next update will contain
	 * the time spent prepping the screen.  Zero the next delta, so the update is
	 * seen. */
	SCREENMAN->ZeroNextUpdate();
}

/*
 * (c) 2001-2004 Chris Danford
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
