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
const CString PARTICLE_DIR = "BGEffects\\particles\\";
const CString BACKGROUND_DIR = "BGEffects\\backgrounds\\";


// macro for handling CArrays of pointers
#define DELETE_CARRAY_CONTENTS( p )	{\
for( int d_ca_cI=0; d_ca_cI < (p).GetSize(); d_ca_cI++ ){ \
	if((p).GetAt( d_ca_cI )) \
		delete (p).GetAt( d_ca_cI ); \
}}

#define GET_RAND_ELEMENT( p	)		( (p).GetAt( rand() % (p).GetSize() ) )


Background::Background()
{
	m_totalTime = 0.0f;

	m_bShowDanger = false;

	m_sprDanger.SetZoom( 2 );
	m_sprDanger.SetEffectWagging();
	m_sprDanger.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_DANGER_TEXT) );
	m_sprDanger.SetXY( CENTER_X, CENTER_Y );

	m_sprDangerBackground.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_DANGER_BACKGROUND) );
	m_sprDangerBackground.StretchTo( CRect((int)SCREEN_LEFT, (int)SCREEN_TOP, (int)SCREEN_RIGHT, (int)SCREEN_BOTTOM) );

	// load particle sprites
	m_curParticleSprite = NULL;
	LoadParticleSprites( PARTICLE_DIR );

	// load particle systems
	m_curPS = NULL;
	LoadParticleSystems();

	// load background sprites
	m_curBackground = NULL;
	LoadBackgroundSprites( BACKGROUND_DIR );

	m_songBackground = NULL;

}


void Background::LoadParticleSprites( CString path )
{
	CStringArray filenames;

	// get full path filenames for particleSprites
	GetDirListing( path+"*.sprite", filenames, false, true );

	// load sprites
	for( int i=0; i < filenames.GetSize(); i++ )
	{
		Sprite * temp = new Sprite();

		temp->Load( filenames.GetAt(i) );
		temp->SetBlendModeAdd();
		
		m_particleSprites.Add( temp );
	}
}


void Background::LoadParticleSystems()
{
	m_particleSystems.Add( new DroppingParticleSystem );
	m_particleSystems.Add( new SpiralParticleSystem );
	m_particleSystems.Add( new ScrollingParticleSystem );
}


void Background::LoadBackgroundSprites( CString path )
{
	CStringArray filenames;

	// get full path filenames for particleSprites
	GetDirListing( path+"*.png", filenames, false, true );

	// load sprites
	for( int i=0; i < filenames.GetSize(); i++ )
	{
		Sprite * temp = new Sprite();

		temp->Load( filenames.GetAt(i) );

		float colTiles = SCREEN_WIDTH / temp->GetUnzoomedWidth();
		float rowTiles = SCREEN_HEIGHT / temp->GetUnzoomedHeight();
		
		temp->StretchTo( CRect( SCREEN_LEFT, SCREEN_TOP, SCREEN_RIGHT, SCREEN_BOTTOM ) );

		float textCoords[] = {
			0,rowTiles,					// bottom-left
			0,0,						// top-left
			colTiles, rowTiles,			// bottom-right
			colTiles, 0,				// top-right
		};
		temp->SetCustomTextureCoords( textCoords );

		m_backgroundSprites.Add( temp );
	}
}


Background::~Background()
{
	DELETE_CARRAY_CONTENTS( m_particleSprites );
	DELETE_CARRAY_CONTENTS( m_particleSystems );
	DELETE_CARRAY_CONTENTS( m_backgroundSprites );
}



bool Background::LoadFromSong( Song* pSong, bool bDisableVisualizations )
{
	Sprite * sprSongBackground = new Sprite();

	if( pSong->HasBackgroundMovie() )
	{
		// load the movie backgound, and don't load a visualization
		sprSongBackground->Load( pSong->GetBackgroundMoviePath() );
		sprSongBackground->StretchTo( CRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT ) );
		sprSongBackground->SetZoomY( -1 );
		sprSongBackground->SetDiffuseColor( D3DXCOLOR(0,0,0,1) );
		//this->AddActor( &m_sprSongBackground );

	}
	else
	{
		// load the static background (if available), and a visualization
		if( pSong->HasBackground() )	sprSongBackground->Load( pSong->GetBackgroundPath(), false, 2, 0, true );
		else							sprSongBackground->Load( THEME->GetPathTo(GRAPHIC_FALLBACK_BACKGROUND), false, 2, 0, true );

		sprSongBackground->StretchTo( CRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT ) );
		sprSongBackground->SetDiffuseColor( D3DXCOLOR(0,0,0,1) );
		//this->AddActor( &m_sprSongBackground );


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

	m_backgroundSprites.Add( sprSongBackground );
	m_songBackground = sprSongBackground;


	// get a effect to start with
	NextEffect();

	return true;
}

void Background::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	m_totalTime += fDeltaTime;

	m_sprVisualizationOverlay.Update( fDeltaTime );
	//m_sprSongBackground.Update( fDeltaTime );
	m_sprDanger.Update( fDeltaTime );
	m_sprDangerBackground.Update( fDeltaTime );

	if( m_curPS && m_curParticleSprite )
	{
		m_curParticleSprite->Update( fDeltaTime );
		m_curPS->update( fDeltaTime );
	}
	if( m_curBackground )
	{
		if( m_curBackground != m_songBackground )
		{
			float tweenPeriod = 2.0f;
			float rads = ( fmodf( m_totalTime, tweenPeriod ) / tweenPeriod ) * 2.0f * D3DX_PI;

			D3DXCOLOR color = D3DXCOLOR(
				cosf( rads ) * 0.5f + 0.5f,
				cosf( rads + D3DX_PI * 2.0f / 3.0f ) * 0.5f + 0.5f,
				cosf( rads + D3DX_PI * 4.0f / 3.0f) * 0.5f + 0.5f,
				1.0f
			);
			m_curBackground->SetDiffuseColor( color );

		}
		m_curBackground->Update( fDeltaTime );
	}
}

void Background::DrawPrimitives()
{
	ActorFrame::DrawPrimitives();

	if( m_bShowDanger  &&  (TIMER->GetTimeSinceStart() - (int)TIMER->GetTimeSinceStart()) > 0.5f )
	{
		m_sprDangerBackground.Draw();
		m_sprDanger.Draw();
	}
	else
	{
		//m_sprSongBackground.Draw();
		m_sprVisualizationOverlay.Draw();

		if( m_curBackground )
			m_curBackground->Draw();

		if( m_curPS && m_curParticleSprite )
			m_curPS->draw( m_curParticleSprite );
	}
}




void Background::NextEffect()
{
	// randomly choose a particle system
	if( m_particleSystems.GetSize() > 0 )
		m_curPS = GET_RAND_ELEMENT( m_particleSystems );

	// randomly choose a particle sprite
	if( m_particleSprites.GetSize() > 0 )
		m_curParticleSprite = GET_RAND_ELEMENT( m_particleSprites );

	// randomly choose a background sprite
	if( m_backgroundSprites.GetSize() > 0 )
	{
		m_curBackground = GET_RAND_ELEMENT( m_backgroundSprites );
	}
}

