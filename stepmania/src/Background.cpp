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
	m_bShowDanger = false;

	m_sprDanger.SetZoom( 2 );
	m_sprDanger.SetEffectWagging();
	m_sprDanger.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_DANGER_TEXT) );
	m_sprDanger.SetXY( CENTER_X, CENTER_Y );

	m_sprDangerBackground.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_DANGER_BACKGROUND) );
	m_sprDangerBackground.StretchTo( CRect((int)SCREEN_LEFT, (int)SCREEN_TOP, (int)SCREEN_RIGHT, (int)SCREEN_BOTTOM) );
}

bool Background::LoadFromSong( Song* pSong, bool bDisableVisualizations )
{
	if( pSong->HasBackgroundMovie() )
	{
		// load the movie backgound, and don't load a visualization
		m_sprSongBackground.Load( pSong->GetBackgroundMoviePath() );
		m_sprSongBackground.StretchTo( CRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT ) );
		m_sprSongBackground.SetZoomY( -1 );
		m_sprSongBackground.SetDiffuseColor( D3DXCOLOR(0,0,0,1) );
		this->AddActor( &m_sprSongBackground );
	}
	else
	{
		// load the static background (if available), and a visualization
		if( pSong->HasBackground() )	m_sprSongBackground.Load( pSong->GetBackgroundPath(), false, 2, 0, true );
		else							m_sprSongBackground.Load( THEME->GetPathTo(GRAPHIC_FALLBACK_BACKGROUND), false, 2, 0, true );

		m_sprSongBackground.StretchTo( CRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT ) );
		m_sprSongBackground.SetDiffuseColor( D3DXCOLOR(0,0,0,1) );
		this->AddActor( &m_sprSongBackground );


		if( PREFSMAN->m_bUseRandomVis && !bDisableVisualizations )
		{
			// load a random visualization
			CStringArray sVisualizationPaths;
			GetDirListing( VIS_DIR + "*.avi", sVisualizationPaths );
			GetDirListing( VIS_DIR + "*.mpg", sVisualizationPaths );
			GetDirListing( VIS_DIR + "*.mpeg", sVisualizationPaths );
			if( sVisualizationPaths.GetSize() > 0 )	// there is at least one visualization
			{
				int iIndexRandom = rand() % sVisualizationPaths.GetSize();

				m_sprVisualizationOverlay.Load( VIS_DIR + sVisualizationPaths[iIndexRandom] );

				
				m_sprVisualizationOverlay.StretchTo( CRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT ) );
				m_sprVisualizationOverlay.SetZoomY( m_sprVisualizationOverlay.GetZoomY()*-1 );
				m_sprVisualizationOverlay.SetBlendModeAdd();
				m_sprVisualizationOverlay.SetDiffuseColor( D3DXCOLOR(0,0,0,0) );
				this->AddActor( &m_sprVisualizationOverlay );
			}
		}		
	}

	return true;
}

void Background::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	m_sprVisualizationOverlay.Update( fDeltaTime );
	m_sprSongBackground.Update( fDeltaTime );
	m_sprDanger.Update( fDeltaTime );
	m_sprDangerBackground.Update( fDeltaTime );
}

void Background::DrawPrimitives()
{
	ActorFrame::DrawPrimitives();

	if( m_bShowDanger  &&  (GetTickCount() % 1000) > 500 )
	{
		m_sprDangerBackground.Draw();
		m_sprDanger.Draw();
	}
	else
	{
		m_sprSongBackground.Draw();
		m_sprVisualizationOverlay.Draw();
	}
}



