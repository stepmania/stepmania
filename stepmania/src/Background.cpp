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


const CString MOVIE_VIS_DIR = "Visualizations\\";
const CString BG_ANIMS_DIR = "BGAnimations\\";


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

	m_pCurBackgroundAnimation = NULL;
}

bool Background::LoadFromSong( Song* pSong, bool bDisableVisualizations )
{
	//
	// Load background animations
	//
	CStringArray asBGAnimNames;

	// We're going to try to classify songs as trance, pop, or techno based on some data about the song
	if( pSong->m_BPMSegments.GetSize() + pSong->m_FreezeSegments.GetSize() >= 3 )
		GetDirListing( BG_ANIMS_DIR+"techno*.*", asBGAnimNames, true );
	else if( pSong->m_BPMSegments[0].m_fBPM > 160 )
		GetDirListing( BG_ANIMS_DIR+"trance*.*", asBGAnimNames, true );
	else
		GetDirListing( BG_ANIMS_DIR+"pop*.*", asBGAnimNames, true );
	
	// pick 4 random animations from this array 
	for( int i=0; i<asBGAnimNames.GetSize(); i++ )
		m_BackgroundAnimations.Add( new BackgroundAnimation(BG_ANIMS_DIR + asBGAnimNames[i], pSong) );


	//
	// load song background
	//
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
		GetDirListing( MOVIE_VIS_DIR + "*.avi", sVisualizationNames );
		GetDirListing( MOVIE_VIS_DIR + "*.mpg", sVisualizationNames );
		GetDirListing( MOVIE_VIS_DIR + "*.mpeg", sVisualizationNames );
		if( sVisualizationNames.GetSize() > 0 )	// there is at least one visualization
		{
			int iIndexRandom = rand() % sVisualizationNames.GetSize();

			m_sprVisualizationOverlay.Load( MOVIE_VIS_DIR + sVisualizationNames[iIndexRandom] );
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
		if( m_BackgroundAnimations.GetSize() > 0 )
		{
			int iIndexToSwitchTo = int(m_fSongBeat/BEATS_PER_MEASURE/4) % m_BackgroundAnimations.GetSize();
			m_pCurBackgroundAnimation = m_BackgroundAnimations[ iIndexToSwitchTo ];
		}
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
			if( m_pCurBackgroundAnimation  &&  !m_bFreeze )
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
				m_pCurBackgroundAnimation->Draw();
			else
				m_sprSongBackground.Draw();
			break;

		case VIS_MODE_MOVIE:
			m_sprSongBackground.Draw();
			m_sprVisualizationOverlay.Draw();
			break;

		case VIS_MODE_NONE:
			m_sprSongBackground.Draw();
			break;
		}
	}
}

bool Background::DangerVisible()
{
	return m_bShowDanger && (TIMER->GetTimeSinceStart() - (int)TIMER->GetTimeSinceStart()) > 0.5f;
}
