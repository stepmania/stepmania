#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: Background

 Desc: Background behind arrows while dancing

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Background.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "RageBitmapTexture.h"
#include "RageException.h"
#include "RageTimer.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "PrefsManager.h"


const CString BG_ANIMS_DIR = "BGAnimations\\";
const CString VISUALIZATIONS_DIR = "Visualizations\\";
const CString RANDOMMOVIES_DIR = "RandomMovies\\";

// TODO: Move these into theme metrics
const int BACKGROUND_LEFT	= 0;
const int BACKGROUND_TOP	= 0;
const int BACKGROUND_RIGHT	= 640;
const int BACKGROUND_BOTTOM	= 480;

#define RECT_BACKGROUND CRect(BACKGROUND_LEFT,BACKGROUND_TOP,BACKGROUND_RIGHT,BACKGROUND_BOTTOM)

int CompareAnimSegs(const void *arg1, const void *arg2)
{
	// arg1 and arg2 are of type Step**
	AnimSeg* seg1 = (AnimSeg*)arg1;
	AnimSeg* seg2 = (AnimSeg*)arg2;

	float score1 = seg1->m_fStartBeat;
	float score2 = seg2->m_fStartBeat;

	if( score1 == score2 )
		return 0;
	else if( score1 < score2 )
		return -1;
	else
		return 1;
}

void SortAnimSegArray( CArray<AnimSeg,AnimSeg&> &arrayAnimSegs )
{
	qsort( arrayAnimSegs.GetData(), arrayAnimSegs.GetSize(), sizeof(AnimSeg), CompareAnimSegs );
}


Background::Background()
{
	m_iCurAnimSegment = 0;
	m_bInDanger = false;

	m_BackgroundMode = MODE_STATIC_BG;

	m_sprDanger.SetZoom( 2 );
	m_sprDanger.SetEffectWagging();
	m_sprDanger.Load( THEME->GetPathTo("Graphics","gameplay danger text") );
	m_sprDanger.SetX( (float)RECTCENTERX(RECT_BACKGROUND) );
	m_sprDanger.SetY( (float)RECTCENTERY(RECT_BACKGROUND) );

	m_sprDangerBackground.Load( THEME->GetPathTo("Graphics","gameplay danger background") );
	m_sprDangerBackground.StretchTo( RECT_BACKGROUND );

	m_quadBGBrightness.StretchTo( RECT_BACKGROUND );
	m_quadBGBrightness.SetDiffuseColor( D3DXCOLOR(0,0,0,1-0.5f) );

	m_quadBorder[0].StretchTo( CRect(SCREEN_LEFT,SCREEN_TOP,BACKGROUND_LEFT,SCREEN_BOTTOM) );
	m_quadBorder[0].SetDiffuseColor( D3DXCOLOR(0,0,0,0) );
	m_quadBorder[1].StretchTo( CRect(BACKGROUND_LEFT,SCREEN_TOP,BACKGROUND_RIGHT,BACKGROUND_TOP) );
	m_quadBorder[1].SetDiffuseColor( D3DXCOLOR(0,0,0,0) );
	m_quadBorder[2].StretchTo( CRect(BACKGROUND_RIGHT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );
	m_quadBorder[2].SetDiffuseColor( D3DXCOLOR(0,0,0,0) );
	m_quadBorder[3].StretchTo( CRect(BACKGROUND_LEFT,SCREEN_TOP,BACKGROUND_RIGHT,BACKGROUND_TOP) );
	m_quadBorder[3].SetDiffuseColor( D3DXCOLOR(0,0,0,0) );
}

Background::~Background()
{
	Unload();
}

void Background::Unload()
{
    for( int i=0; i<m_BackgroundAnimations.GetSize(); i++ )
		delete m_BackgroundAnimations[i];
	m_BackgroundAnimations.RemoveAll();
}

void Background::LoadFromSong( Song* pSong )
{
	/* Endless was crashing due to this; is there any reason not to
	 * fix it this way? -glenn */
	 /* Correct.  I added the same chage.    -Chris */
	Unload();

	//
	// figure out what BackgroundMode to use
	//
	if( PREFSMAN->m_BackgroundMode == PrefsManager::BGMODE_OFF )
		m_BackgroundMode = MODE_STATIC_BG;
	else if( pSong->HasMovieBackground() )
		m_BackgroundMode = MODE_MOVIE_BG;
	else if( PREFSMAN->m_BackgroundMode == PrefsManager::BGMODE_MOVIEVIS )
		m_BackgroundMode = MODE_MOVIE_VIS;
	else if( PREFSMAN->m_BackgroundMode == PrefsManager::BGMODE_RANDOMMOVIES )
		m_BackgroundMode = MODE_RANDOMMOVIES;
	else
		m_BackgroundMode = MODE_ANIMATIONS;


	//
	// Load the static background that will before notes start and after notes end
	//
	BackgroundAnimation* pTempBGA;
	pTempBGA = new BackgroundAnimation;
	pTempBGA->LoadFromStaticGraphic( pSong->HasBackground() ? pSong->GetBackgroundPath() : THEME->GetPathTo("Graphics","fallback background") );
	m_BackgroundAnimations.Add( pTempBGA );


	//
	// Load animations that will play while notes are active
	//
	switch( m_BackgroundMode )
	{
	case MODE_STATIC_BG:
		break;
	case MODE_MOVIE_BG:
		{
			pTempBGA = new BackgroundAnimation;
			pTempBGA->LoadFromMovieBG( pSong->GetMovieBackgroundPath() );
			m_BackgroundAnimations.Add( pTempBGA );
		}
		break;
	case MODE_MOVIE_VIS:
		{
			CStringArray arrayPossibleMovies;
			GetDirListing( VISUALIZATIONS_DIR + "*.avi", arrayPossibleMovies, false, true );
			GetDirListing( VISUALIZATIONS_DIR + "*.mpg", arrayPossibleMovies, false, true );
			GetDirListing( VISUALIZATIONS_DIR + "*.mpeg", arrayPossibleMovies, false, true );
			while( arrayPossibleMovies.GetSize() > 0  &&  m_BackgroundAnimations.GetSize() < 2 )
			{
				int index = rand() % arrayPossibleMovies.GetSize();
				pTempBGA = new BackgroundAnimation;
				pTempBGA->LoadFromVisualization( arrayPossibleMovies[index], pSong->HasBackground() ? pSong->GetBackgroundPath() : THEME->GetPathTo("Graphics","fallback background") );
				m_BackgroundAnimations.Add( pTempBGA );
				arrayPossibleMovies.RemoveAt( index );
			}	
		}
		break;
	case MODE_ANIMATIONS:
		{
			CStringArray arrayPossibleAnims;
			GetDirListing( BG_ANIMS_DIR+"*.*", arrayPossibleAnims, true, true );
			while( arrayPossibleAnims.GetSize() > 0  &&  m_BackgroundAnimations.GetSize() < 5 )
			{
				int index = rand() % arrayPossibleAnims.GetSize();
				pTempBGA = new BackgroundAnimation;
				pTempBGA->LoadFromAniDir( arrayPossibleAnims[index], pSong->HasBackground() ? pSong->GetBackgroundPath() : THEME->GetPathTo("Graphics","fallback background") );
				m_BackgroundAnimations.Add( pTempBGA );
				arrayPossibleAnims.RemoveAt( index );
			}
		}
		break;
	case MODE_RANDOMMOVIES:
		{
			CStringArray arrayPossibleMovies;
			GetDirListing( RANDOMMOVIES_DIR + "*.avi", arrayPossibleMovies, false, true );
			GetDirListing( RANDOMMOVIES_DIR + "*.mpg", arrayPossibleMovies, false, true );
			GetDirListing( RANDOMMOVIES_DIR + "*.mpeg", arrayPossibleMovies, false, true );
			while( arrayPossibleMovies.GetSize() > 0  &&  m_BackgroundAnimations.GetSize() < 5 )
			{
				int index = rand() % arrayPossibleMovies.GetSize();
				pTempBGA = new BackgroundAnimation;
				pTempBGA->LoadFromRandomMovie( arrayPossibleMovies[index] );
				m_BackgroundAnimations.Add( pTempBGA );
				arrayPossibleMovies.RemoveAt( index );
			}	
		}
		break;
	default:
		ASSERT(0);
	}

	// At this point, m_BackgroundAnimations[0] is the song background, and everything else
	// in m_BackgroundAnimations should be cycled through randomly while notes are playing.	
	//
	// Generate an animation plan
	//
	if( m_BackgroundMode == MODE_MOVIE_VIS )
	{
		m_aAnimSegs.Add( AnimSeg(-10000,1) );
		return;
	}

	// start off showing the static song background
	m_aAnimSegs.Add( AnimSeg(-10000,0) );

	// change BG every 4 bars
	const float fFirstBeat = m_BackgroundMode==MODE_MOVIE_BG ? 0 : pSong->m_fFirstBeat;
	const float fLastBeat = pSong->m_fLastBeat;
	for( float f=fFirstBeat; f<fLastBeat; f+=16 )
	{
		int index;
		if( m_BackgroundAnimations.GetSize()==1 )
			index = 0;
		else
			index = 1 + rand()%(m_BackgroundAnimations.GetSize()-1);
		m_aAnimSegs.Add( AnimSeg(f,index) );
	}

	// change BG every BPM change
	for( int i=0; i<pSong->m_BPMSegments.GetSize(); i++ )
	{
		const BPMSegment& bpmseg = pSong->m_BPMSegments[i];

		if( bpmseg.m_fStartBeat < fFirstBeat  || bpmseg.m_fStartBeat > fLastBeat )
			continue;	// skip]

		int index;
		if( m_BackgroundAnimations.GetSize()==1 )
			index = 0;
		else
			index = 1 + int(bpmseg.m_fBPM)%(m_BackgroundAnimations.GetSize()-1);
		m_aAnimSegs.Add( AnimSeg(bpmseg.m_fStartBeat,index) );
	}

	// end showing the static song background
	m_aAnimSegs.Add( AnimSeg(pSong->m_fLastBeat,0) );

	// sort segments
	SortAnimSegArray( m_aAnimSegs );
//	for( int i=0; i<m_aAnimSegs.GetSize(); i++ )
//		printf("AnimSeg %d: %f, %i\n", i, m_aAnimSegs[i].m_fStartBeat, m_aAnimSegs[i].m_iAnimationIndex);


/*
// Load the song's pre-defined animation plan.
// TODO:  Fix this

			// Load a background animation plan into m_aAnimSegs
			if( pSong->m_AnimationSegments.GetSize() > 0 )
			{
				// the song has a plan.  Use it.
				for( int i=0; i<pSong->m_AnimationSegments.GetSize(); i++ )
				{
					const AnimationSegment& aniseg = pSong->m_AnimationSegments[i];

					int iIndex = -1;
					for( int j=0; j<asBGAnimNames.GetSize(); j++ )
					{
						if( asBGAnimNames[j] == aniseg.m_sAnimationName )
						{
							iIndex = j;
							break;
						}
					}
					if( iIndex == -1 )	// this animation doesn't exist in the song dir
						iIndex = rand() % m_BackgroundAnimations.GetSize();	// use a random one instead

					m_aAnimSegs.Add( AnimSeg(aniseg.m_fStartBeat, iIndex) );
				}
				SortAnimSegArray( m_aAnimSegs );	// this should already be sorted
*/
}


void Background::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	if( DangerVisible() )
	{
		m_sprDangerBackground.Update( fDeltaTime );
		m_sprDanger.Update( fDeltaTime );
	}
	else
	{			
		if( GAMESTATE->m_bFreeze )
			return;

		// Find the AnimSeg we're in
		for( int i=0; i<m_aAnimSegs.GetSize()-1; i++ )
			if( GAMESTATE->m_fSongBeat < m_aAnimSegs[i+1].m_fStartBeat )
				break;
		ASSERT( i >= 0  &&  i<m_aAnimSegs.GetSize() );

		if( i > m_iCurAnimSegment )
		{
//			printf( "%d, %d, %f, %f\n", m_iCurAnimSegment, i, m_aAnimSegs[i].m_fStartBeat, GAMESTATE->m_fSongBeat );
			GetCurBGA()->LosingFocus();
			m_iCurAnimSegment = i;
			GetCurBGA()->GainingFocus();

		}

		GetCurBGA()->Update( fDeltaTime );

	}
	
	m_quadBGBrightness.Update( fDeltaTime );
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
		GetCurBGA()->Draw();
	}

	m_quadBGBrightness.Draw();
	for( int i=0; i<4; i++ )
		m_quadBorder[i].Draw();
}

bool Background::DangerVisible()
{
	return m_bInDanger  &&  PREFSMAN->m_bShowDanger  &&  (TIMER->GetTimeSinceStart() - (int)TIMER->GetTimeSinceStart()) < 0.5f;
}


void Background::FadeIn()
{
	m_quadBGBrightness.BeginTweening( 0.5f );
	m_quadBGBrightness.SetTweenDiffuseColor( D3DXCOLOR(0,0,0,1-PREFSMAN->m_fBGBrightness) );
}

void Background::FadeOut()
{
	m_quadBGBrightness.BeginTweening( 0.5f );
	m_quadBGBrightness.SetTweenDiffuseColor( D3DXCOLOR(0,0,0,1-0.5f) );

}
