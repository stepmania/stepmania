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
	this->AddChild( &m_Background );

	m_Overlay.LoadFromAniDir( THEME->GetPathToB(m_sName + " overlay"));

	m_In.Load( THEME->GetPathToB(m_sName + " in") );
	m_In.StartTransitioning();
	this->AddChild( &m_In );

	m_Out.Load( THEME->GetPathToB(m_sName + " out") );
	this->AddChild( &m_Out );

	m_Back.Load( THEME->GetPathToB("Common back") );
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
		SET_XY( m_sprCharacterIcon[p] );
		ON_COMMAND( m_sprCharacterIcon[p] );
	}

	m_SongTitle.SetName( "SongTitle");
	m_Artist.SetName( "Artist" );
	m_SongTitle.LoadFromFont( THEME->GetPathToF("ScreenStage Title") );
	m_Artist.LoadFromFont( THEME->GetPathToF("ScreenStage Artist") );

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
		if( GAMESTATE->m_pCurSong != NULL)
			if( GAMESTATE->m_pCurSong->HasBanner() )
				m_Banner.LoadFromSong( GAMESTATE->m_pCurSong );
	m_Banner.SetName("Banner");
	SET_XY( m_Banner );
	ON_COMMAND(m_Banner);

	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("stage "+GAMESTATE->GetStageText()) );
}

void ScreenStage::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
	int p;
	for( p=0; p<NUM_PLAYERS; p++ )
		m_sprCharacterIcon[p].Update(fDeltaTime);
	m_Overlay.Update(fDeltaTime);
	if ( PREFSMAN->m_bShowBanners )
		if( GAMESTATE->m_pCurSong != NULL)
			if( GAMESTATE->m_pCurSong->HasBanner() )
				m_Banner.Update(fDeltaTime);
	m_SongTitle.Update(fDeltaTime);
	m_Artist.Update(fDeltaTime);
}

void ScreenStage::DrawPrimitives()
{
	Screen::DrawPrimitives();
	int p;
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_sprCharacterIcon[p].Draw();
	}
	m_Overlay.Draw();
	if ( PREFSMAN->m_bShowBanners )
		if( GAMESTATE->m_pCurSong != NULL)
			if( GAMESTATE->m_pCurSong->HasBanner() )
				m_Banner.Draw();
	m_SongTitle.Draw();
	m_Artist.Draw();
}

void ScreenStage::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_PrepScreen:
		SCREENMAN->PrepNewScreen( NEXT_SCREEN );
		break;
	case SM_BeginFadingOut:
		m_Out.StartTransitioning( SM_GoToNextScreen );
		int p;
		for( p=0; p<NUM_PLAYERS; p++ )
		{
			OFF_COMMAND( m_sprCharacterIcon[p] );
		}
		OFF_COMMAND( m_SongTitle );
		OFF_COMMAND( m_Artist );
		OFF_COMMAND( m_Banner );
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
