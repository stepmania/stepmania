#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: Background.cpp

 Desc: Background behind arrows while dancing

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "Background.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "PrefsManager.h"
#include "RageBitmapTexture.h"


const CString VIS_DIR = "Visualizations\\";


Background::Background()
{
	m_fSongBeat = 0;

	m_bShowDanger = false;

	m_sprDanger.SetZoom( 2 );
	m_sprDanger.SetEffectWagging();
	m_sprDanger.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_DANGER_TEXT) );
	m_sprDanger.SetXY( CENTER_X, CENTER_Y );

	m_sprDangerBackground.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_DANGER_BACKGROUND) );
	m_sprDangerBackground.StretchTo( CRect((int)SCREEN_LEFT, (int)SCREEN_TOP, (int)SCREEN_RIGHT, (int)SCREEN_BOTTOM) );

	// Load background animations
	for( int i=0; i<NUM_BACKGROUND_ANIMATION_TYPES; i++ )
	{
		m_BackgroundAnimations.Add( new BackgroundAnimation( (BackgroundAnimationType)i ) );
	}

	m_pCurBackgroundAnimation = NULL;
}

bool Background::LoadFromSong( Song* pSong, bool bDisableVisualizations )
{
	// load song background

	CString sBackgroundMoviePath;
	if( pSong->HasBackgroundMovie() )
	{
		m_sprSongBackground.Load( pSong->GetBackgroundMoviePath() );
		m_sprSongBackground.SetZoomY( -1 );	// flip
	}
	else if( pSong->HasBackground() )
	{
		m_sprSongBackground.Load( pSong->GetBackgroundPath(), false, 2, 0, true );
	}
	else
	{
		m_sprSongBackground.Load( THEME->GetPathTo(GRAPHIC_FALLBACK_BACKGROUND), false, 2, 0, true );
	}
			
	m_sprSongBackground.StretchTo( CRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT ) );
	m_sprSongBackground.SetDiffuseColor( D3DXCOLOR(0,0,0,1) );


	if( PREFSMAN->m_visMode == VIS_MODE_MOVIE && !bDisableVisualizations )
	{
		// load a random visualization
		CStringArray sVisualizationNames;
		GetDirListing( VIS_DIR + "*.avi", sVisualizationNames );
		GetDirListing( VIS_DIR + "*.mpg", sVisualizationNames );
		GetDirListing( VIS_DIR + "*.mpeg", sVisualizationNames );
		if( sVisualizationNames.GetSize() > 0 )	// there is at least one visualization
		{
			int iIndexRandom = rand() % sVisualizationNames.GetSize();

			m_sprVisualizationOverlay.Load( VIS_DIR + sVisualizationNames[iIndexRandom] );
			m_sprVisualizationOverlay.StretchTo( CRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT ) );
			m_sprVisualizationOverlay.SetZoomY( m_sprVisualizationOverlay.GetZoomY()*-1 );
			m_sprVisualizationOverlay.SetBlendModeAdd();
			m_sprVisualizationOverlay.SetDiffuseColor( D3DXCOLOR(0,0,0,0) );
			this->AddActor( &m_sprVisualizationOverlay );
		}
	}

	return true;
}

void Background::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	if( m_fSongBeat < 8 )
		m_pCurBackgroundAnimation = NULL;
	else
	{
		int iIndexToSwitchTo = int(m_fSongBeat/BEATS_PER_MEASURE/2) % m_BackgroundAnimations.GetSize();
		m_pCurBackgroundAnimation = m_BackgroundAnimations[ iIndexToSwitchTo ];
	}


	if( DangerVisible() )
	{
		m_sprDangerBackground.Update( fDeltaTime );
		m_sprDanger.Update( fDeltaTime );
	}
	else
	{
		switch( PREFSMAN->m_visMode )
		{
		case VIS_MODE_ANIMATION:
			if( m_pCurBackgroundAnimation )
				m_pCurBackgroundAnimation->Update( fDeltaTime, m_fSongBeat );
			else
				m_sprSongBackground.Update( fDeltaTime );
			break;

		case VIS_MODE_MOVIE:
			m_sprVisualizationOverlay.Update( fDeltaTime );
			// fall through and draw background

		case VIS_MODE_NONE:
			m_sprSongBackground.Update( fDeltaTime );
			break;
		}
	}
	m_sprVisualizationOverlay.Update( fDeltaTime );
	m_sprSongBackground.Update( fDeltaTime );
	m_sprDanger.Update( fDeltaTime );
	m_sprDangerBackground.Update( fDeltaTime );
}

void Background::DrawPrimitives()
{
	ActorFrame::DrawPrimitives();

	if( DangerVisible() )
	{
		m_sprDangerBackground.Draw();
		m_sprDanger.Draw();
	}
	else
	{
		switch( PREFSMAN->m_visMode )
		{
		case VIS_MODE_ANIMATION:
			if( m_pCurBackgroundAnimation )
				m_pCurBackgroundAnimation->DrawPrimitives();
			else
				m_sprSongBackground.Draw();
			break;

		case VIS_MODE_MOVIE:
			m_sprVisualizationOverlay.Draw();
			// fall through and draw background

		case VIS_MODE_NONE:
			m_sprSongBackground.Draw();
			break;
		}
		m_sprSongBackground.Draw();
		m_sprVisualizationOverlay.Draw();
	}
}

bool Background::DangerVisible()
{
	return m_bShowDanger && (TIMER->GetTimeSinceStart() - (int)TIMER->GetTimeSinceStart()) > 0.5f;
}
