#include "global.h"
#include "ScreenPlayerOptions.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"
#include "GameSoundManager.h"
#include "ScreenSongOptions.h"
#include "PrefsManager.h"
#include "CodeDetector.h"


#define PREV_SCREEN		THEME->GetMetric ("ScreenPlayerOptions","PrevScreen")
#define NEXT_SCREEN		THEME->GetMetric ("ScreenPlayerOptions","NextScreen")

ScreenPlayerOptions::ScreenPlayerOptions( CString sClassName ) :
	ScreenOptionsMaster( sClassName )
{
	LOG->Trace( "ScreenPlayerOptions::ScreenPlayerOptions()" );

	m_bAskOptionsMessage =
		!GAMESTATE->m_bEditing && PREFSMAN->m_ShowSongOptions == PrefsManager::ASK;

	/* If we're going to "press start for more options" or skipping options
	 * entirely, we need a different fade out. XXX: this is a hack */
	if( PREFSMAN->m_ShowSongOptions == PrefsManager::NO || GAMESTATE->m_bEditing )
		m_Out.Load( THEME->GetPathToB("ScreenPlayerOptions direct out") ); /* direct to stage */
	else if( m_bAskOptionsMessage )
	{
		m_Out.Load( THEME->GetPathToB("ScreenPlayerOptions option out") ); /* optional song options */

		m_sprOptionsMessage.Load( THEME->GetPathToG("ScreenPlayerOptions options") );
		m_sprOptionsMessage.StopAnimating();
		m_sprOptionsMessage.SetXY( CENTER_X, CENTER_Y );
		m_sprOptionsMessage.SetZoom( 1 );
		m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
		//this->AddChild( &m_sprOptionsMessage );       // we have to draw this manually over the top of transitions
	}

	m_bAcceptedChoices = false;
	m_bGoToOptions = ( PREFSMAN->m_ShowSongOptions == PrefsManager::YES );

	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("player options intro") );

	FOREACH_HumanPlayer( pn )
	{
		m_sprCancelAll[pn].LoadAndSetName( "ScreenPlayerOptions", ssprintf("CancelAllP%d",pn+1) );
		SET_XY_AND_ON_COMMAND( m_sprCancelAll[pn] );
		this->AddChild( m_sprCancelAll[pn] );
	}

	UpdateDisqualified();
}


void ScreenPlayerOptions::GoToPrevState()
{
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen();
	else
		SCREENMAN->SetNewScreen( PREV_SCREEN );
}

void ScreenPlayerOptions::GoToNextState()
{
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen();
	else
	{
		GAMESTATE->AdjustFailType();

		if( m_bGoToOptions )
			SCREENMAN->SetNewScreen( NEXT_SCREEN );
		else
			SCREENMAN->SetNewScreen( ScreenSongOptions::GetNextScreen() );
	}
}


void ScreenPlayerOptions::Update( float fDelta )
{
	ScreenOptionsMaster::Update( fDelta );
	if( m_bAskOptionsMessage )
		m_sprOptionsMessage.Update( fDelta );
}

void ScreenPlayerOptions::DrawPrimitives()
{
	ScreenOptionsMaster::DrawPrimitives();
	if( m_bAskOptionsMessage )
		m_sprOptionsMessage.Draw();
}


void ScreenPlayerOptions::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( m_bAskOptionsMessage &&
		type == IET_FIRST_PRESS  &&
		!m_In.IsTransitioning()  &&
		MenuI.IsValid()  &&
		MenuI.button == MENU_BUTTON_START )
	{
		if( m_bAcceptedChoices  &&  !m_bGoToOptions )
		{
			m_bGoToOptions = true;
			m_sprOptionsMessage.SetState( 1 );
			SCREENMAN->PlayStartSound();
		}
	}

	PlayerNumber pn = StyleI.player;

	if( GAMESTATE->IsHumanPlayer(StyleI.player) && CodeDetector::EnteredCode(GameI.controller,CodeDetector::CODE_CANCEL_ALL_PLAYER_OPTIONS) )
	{
		SOUND->PlayOnce( THEME->GetPathToS("ScreenPlayerOptions cancel all") );
		
		// apply the game default mods, but not the Profile saved mods
		GAMESTATE->m_PlayerOptions[pn].Init();
		GAMESTATE->m_PlayerOptions[pn].FromString( PREFSMAN->m_sDefaultModifiers );
		
		UtilCommand( m_sprCancelAll[pn], m_sName, "Show" );

		this->ImportOptionsForPlayer( pn );
		this->PositionUnderlines();
		this->UpdateDisqualified();
	}

	ScreenOptionsMaster::Input( DeviceI, type, GameI, MenuI, StyleI );

	// UGLY: Update m_Disqualified whenever Start is pressed
	if( MenuI.IsValid() && MenuI.button == MENU_BUTTON_START )
		UpdateDisqualified();
}

void ScreenPlayerOptions::HandleScreenMessage( const ScreenMessage SM )
{
	if( m_bAskOptionsMessage )
	{
		switch( SM )
		{
		case SM_BeginFadingOut: // when the user accepts the page of options
			{
				m_bAcceptedChoices = true;

				float fShowSeconds = m_Out.GetLengthSeconds();

				// show "hold START for options"
				m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
				m_sprOptionsMessage.BeginTweening( 0.15f );     // fade in
				m_sprOptionsMessage.SetZoomY( 1 );
				m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,1) );
				m_sprOptionsMessage.BeginTweening( fShowSeconds-0.3f ); // sleep
				m_sprOptionsMessage.BeginTweening( 0.15f );     // fade out
				m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
				m_sprOptionsMessage.SetZoomY( 0 );
			}
			break;
		}
	}

	ScreenOptionsMaster::HandleScreenMessage( SM );
}

void ScreenPlayerOptions::UpdateDisqualified()
{
	// save current player options 
	PlayerOptions po[2];

	FOREACH_PlayerNumber( p )
	{
		po[p] = GAMESTATE->m_PlayerOptions[p];
	}
	
	// export the currently selection options, which will fill GAMESTATE->m_PlayerOptions
	ScreenOptionsMaster::ExportOptions();

	FOREACH_HumanPlayer( p )
	{
		bool bIsHandicap = GAMESTATE->IsDisqualified(p);
		
		m_sprDisqualify[p]->SetHidden( !bIsHandicap );

		// restore previous player options in case the user escapes back after this
		GAMESTATE->m_PlayerOptions[p] = po[p];
	}
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
