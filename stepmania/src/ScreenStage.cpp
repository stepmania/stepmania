#include "global.h"
#include "ActorUtil.h"
#include "ScreenStage.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "BitmapText.h"
#include "SongManager.h"
#include "Sprite.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "LightsManager.h"
#include "song.h"

#define NEXT_SCREEN				THEME->GetMetric (m_sName,"NextScreen")
#define PREV_SCREEN				THEME->GetMetric (m_sName,"PrevScreen")
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

	m_bZeroDeltaOnNextUpdate = false;

	SOUND->StopMusic();

	LIGHTSMAN->SetLightsMode( LIGHTSMODE_STAGE );

	m_Overlay.Load( THEME->GetPathB(m_sName,"overlay") );
	m_Overlay->SetName( "Overlay" );
	ON_COMMAND( m_Overlay );
	this->AddChild( m_Overlay );

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

	/* Prep the new screen once m_In is complete. */ 	 
	this->PostScreenMessage( SM_PrepScreen, m_Overlay->GetTweenTimeLeft() );

	FOREACH_PlayerNumber(p)
	{
		m_sprCharacterIcon[p].SetName( ssprintf("CharacterIconP%d",p+1) );

		const Character *pChar = GAMESTATE->m_pCurCharacters[p];
		CString sPath = pChar->GetStageIconPath();
		if( sPath == "" )
			continue;

		m_sprCharacterIcon[p].Load( pChar->GetStageIconPath() );
		SET_XY_AND_ON_COMMAND( m_sprCharacterIcon[p] );
		this->AddChild( &m_sprCharacterIcon[p] );
	}

	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("stage "+StageToString(GAMESTATE->GetCurrentStage())) );

	this->SortByDrawOrder();
}

void ScreenStage::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_PrepScreen )
	{
		RageTimer length;
		SCREENMAN->PrepareScreen( NEXT_SCREEN );
		float fScreenLoadSeconds = length.GetDeltaTime();

		/* The screen load took fScreenLoadSeconds.  Move on to the next screen after
		 * no less than MINIMUM_DELAY seconds. */
		this->PostScreenMessage( SM_BeginFadingOut, max( 0, MINIMUM_DELAY-fScreenLoadSeconds) );
	}
	else if( SM == SM_BeginFadingOut )
	{
		m_Out.StartTransitioning();
		FOREACH_PlayerNumber( p )
			OFF_COMMAND( m_sprCharacterIcon[p] );
		this->PostScreenMessage( SM_GoToNextScreen, this->GetTweenTimeLeft() );
	}
	else if( SM == SM_GoToNextScreen )
	{
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
	}
	else if( SM == SM_GoToPrevScreen )
	{
		SCREENMAN->DeletePreparedScreens();
		SCREENMAN->SetNewScreen( PREV_SCREEN );
	}
}

void ScreenStage::Update( float fDeltaTime )
{
	if( m_bZeroDeltaOnNextUpdate )
	{
		m_bZeroDeltaOnNextUpdate = false;
		fDeltaTime = 0;
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
	m_bZeroDeltaOnNextUpdate = true;
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
