#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: Background.cpp

 Desc: Background behind arrows while dancing

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "Background.h"
#include "RageUtil.h"
#include "ScreenDimensions.h"
#include "ThemeManager.h"


const CString VIS_DIR = "Visualizations\\";


Background::Background()
{
	m_bShowDanger = false;

	m_sprDanger.Load( THEME->GetPathTo(GRAPHIC_DANGER) );
	m_sprDanger.SetXY( CENTER_X, CENTER_Y );

	m_sprDangerBackground.Load( THEME->GetPathTo(GRAPHIC_DANGER_BACKGROUND) );
	m_sprDangerBackground.StretchTo( CRect(SCREEN_LEFT, SCREEN_TOP, SCREEN_RIGHT, SCREEN_BOTTOM) );
}

bool Background::LoadFromSong( Song &song )
{
	if( song.HasBackgroundMovie() )
	{
		// load the movie backgound, and don't load a visualization
		m_sprSongBackground.Load( song.HasBackground() );
		m_sprSongBackground.StretchTo( CRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT ) );
		m_sprSongBackground.SetZoomY( -1 );
		m_sprSongBackground.SetDiffuseColor( D3DXCOLOR(0,0,0,1) );
		this->AddActor( &m_sprSongBackground );
	}
	else
	{
		// load the static background (if available), and a visualization
		if( song.HasBackground() )
			m_sprSongBackground.Load( song.GetBackgroundPath() );
		else
			m_sprSongBackground.Load( THEME->GetPathTo(GRAPHIC_FALLBACK_BACKGROUND) );

		m_sprSongBackground.StretchTo( CRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT ) );
		m_sprSongBackground.SetDiffuseColor( D3DXCOLOR(0,0,0,1) );
		this->AddActor( &m_sprSongBackground );


		if( GAMEINFO->m_GameOptions.m_bRandomVis )
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
				m_sprVisualizationOverlay.SetZoomY( -1 );
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

void Background::RenderPrimitives()
{
	ActorFrame::RenderPrimitives();

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



