#include "global.h"
#include "ActorUtil.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenStage

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

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
#include "RageSounds.h"
#include "ThemeManager.h"
#include "LightsManager.h"
#include "song.h"

#define NEXT_SCREEN				THEME->GetMetric (m_sName,"NextScreen")

const ScreenMessage	SM_PrepScreen		= (ScreenMessage)(SM_User+0);

ScreenStage::ScreenStage( CString sClassName ) : Screen( sClassName )
{
	SOUND->StopMusic();

	LIGHTSMAN->SetLightsMode( LIGHTSMODE_STAGE );


	m_Background.LoadFromAniDir( THEME->GetPathToB(m_sName + " "+GAMESTATE->GetStageText()) );
	m_Background.SetZ( 10 ); // behind everything else
	this->AddChild( &m_Background );

	m_Overlay.SetName( "Overlay" );
	m_Overlay.LoadFromAniDir( THEME->GetPathToB(m_sName + " overlay"));
	ON_COMMAND( m_Overlay );
	this->AddChild( &m_Overlay );

	m_In.Load( THEME->GetPathToB(m_sName + " in") );
	m_In.StartTransitioning();
	m_In.SetZ( -2 ); // on top of everything else
	this->AddChild( &m_In );

	m_Out.Load( THEME->GetPathToB(m_sName + " out") );
	m_Out.SetZ( -2 ); // on top of everything else
	this->AddChild( &m_Out );

	m_Back.Load( THEME->GetPathToB("Common back") );
	m_Back.SetZ( -2 ); // on top of everything else
	this->AddChild( &m_Back );

	/* Prep the new screen once the animation is complete.  This way, we
	 * start loading the gameplay screen as soon as possible. */
	this->PostScreenMessage( SM_PrepScreen, m_Background.GetLengthSeconds() );

	/* Start fading out after m_In is complete, minus the length of m_Out.  This
	 * essentially makes m_In a timer to pad the length, so we always wait a minimum
	 * amount of time.  It's a slightly confusing timer, though; maybe we shouldn't
	 * make m_Out affect it.  (Maybe just make it a metric? XXX) */
	float fStartFadingOutSeconds = m_In.GetLengthSeconds() - m_Out.GetLengthSeconds();

	/* Never do this before we send SM_PrepScreen--we havn't loaded the screen yet. */
	fStartFadingOutSeconds = max(fStartFadingOutSeconds, m_Background.GetLengthSeconds());

	this->PostScreenMessage( SM_BeginFadingOut, fStartFadingOutSeconds );

	int p;
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		Character* pChar = GAMESTATE->m_pCurCharacters[p];
		m_sprCharacterIcon[p].SetName( ssprintf("CharacterIconP%d",p+1) );
		m_sprCharacterIcon[p].Load( pChar->GetStageIconPath() );
		SET_XY_AND_ON_COMMAND( m_sprCharacterIcon[p] );
		this->AddChild( &m_sprCharacterIcon[p] );
	}

	m_SongTitle.SetName( "SongTitle");
	m_Artist.SetName( "Artist" );
	m_SongTitle.LoadFromFont( THEME->GetPathToF("ScreenStage Title") );
	m_Artist.LoadFromFont( THEME->GetPathToF("ScreenStage Artist") );

	this->AddChild( &m_SongTitle );
	this->AddChild( &m_Artist );

	if(GAMESTATE->m_pCurSong != NULL)
	{
		m_SongTitle.SetText( GAMESTATE->m_pCurSong->m_sMainTitle );
		m_Artist.SetText( GAMESTATE->m_pCurSong->m_sArtist );
	}
	else
	{
		m_SongTitle.SetText( "" );
		m_Artist.SetText( "" );
	}

	SET_XY_AND_ON_COMMAND( m_Artist );
	SET_XY_AND_ON_COMMAND( m_SongTitle );

	if ( PREFSMAN->m_bShowBanners )
		if( GAMESTATE->m_pCurSong && GAMESTATE->m_pCurSong->HasBanner() )
		{
			m_Banner.LoadFromSong( GAMESTATE->m_pCurSong );
			this->AddChild( &m_Banner );
		}
	m_Banner.SetName("Banner");
	SET_XY( m_Banner );
	ON_COMMAND(m_Banner);

	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("stage "+GAMESTATE->GetStageText()) );

	this->SortByZ();
}

void ScreenStage::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_PrepScreen:
		SCREENMAN->PrepNewScreen( NEXT_SCREEN );
		break;
	case SM_BeginFadingOut:
		m_Out.StartTransitioning();
		int p;
		for( p=0; p<NUM_PLAYERS; p++ )
			OFF_COMMAND( m_sprCharacterIcon[p] );
		OFF_COMMAND( m_SongTitle );
		OFF_COMMAND( m_Artist );
		OFF_COMMAND( m_Banner );
		OFF_COMMAND( m_Background );

		this->PostScreenMessage( SM_GoToNextScreen, this->GetTweenTimeLeft() );
		
		break;
	case SM_GoToNextScreen:
		SCREENMAN->LoadPreppedScreen(); /* use prepped */
		break;
	case SM_GoToPrevScreen:
		SCREENMAN->DeletePreppedScreen();
		SCREENMAN->SetNewScreen( "ScreenSelectMusic" );
		break;
	}
}

void ScreenStage::MenuBack( PlayerNumber pn )
{
	if( m_In.IsTransitioning() || m_Out.IsTransitioning() || m_Back.IsTransitioning() )
		return;

	this->ClearMessageQueue();
	m_Back.StartTransitioning( SM_GoToPrevScreen );
}
