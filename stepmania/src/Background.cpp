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

int CompareBGSegments(const void *arg1, const void *arg2)
{
	// arg1 and arg2 are of type Step**
	BGSegment* seg1 = (BGSegment*)arg1;
	BGSegment* seg2 = (BGSegment*)arg2;

	float score1 = seg1->m_fStartBeat;
	float score2 = seg2->m_fStartBeat;

	if( score1 == score2 )
		return 0;
	else if( score1 < score2 )
		return -1;
	else
		return 1;
}

void SortBGSegmentArray( CArray<BGSegment,BGSegment&> &arrayBGSegments )
{
	qsort( arrayBGSegments.GetData(), arrayBGSegments.GetSize(), sizeof(BGSegment), CompareBGSegments );
}


Background::Background()
{
	m_iCurBGSegment = 0;
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


	if( pSong->HasBGChanges() )
	{
		// start off showing the static song background
		m_aBGSegments.Add( BGSegment(-10000,0) );


		// Load the animations used by the song's pre-defined animation plan.
		// the song has a plan.  Use it.
		for( int i=0; i<pSong->m_BackgroundChanges.GetSize(); i++ )
		{
			const BackgroundChange& aniseg = pSong->m_BackgroundChanges[i];

			// Using aniseg.m_sBGName, search for the corresponding animation.
			// Look in this order:  movies in song dir, BGAnims in song dir
			//  movies in RandomMovies dir, BGAnims in BGAnimsDir.
			CStringArray asFiles;

			// Look for movies in the song dir
			GetDirListing( pSong->m_sSongDir+"movies\\"+aniseg.m_sBGName+".avi", asFiles, false, true );
			GetDirListing( pSong->m_sSongDir+"movies\\"+aniseg.m_sBGName+".mpg", asFiles, false, true );
			GetDirListing( pSong->m_sSongDir+"movies\\"+aniseg.m_sBGName+".mpeg", asFiles, false, true );
			if( asFiles.GetSize() > 0 )
			{
				pTempBGA = new BackgroundAnimation;
				pTempBGA->LoadFromMovie( asFiles[0], true, true );
				m_BackgroundAnimations.Add( pTempBGA );

				m_aBGSegments.Add( BGSegment(aniseg.m_fStartBeat, m_BackgroundAnimations.GetSize()-1) );	// add to the plan
				continue;	// stop looking for this background
			}

			// Look for BGAnims in the song dir
			GetDirListing( pSong->m_sSongDir+aniseg.m_sBGName, asFiles, true, true );
			if( asFiles.GetSize() > 0 )
			{
				pTempBGA = new BackgroundAnimation;
				pTempBGA->LoadFromAniDir( asFiles[0], pSong->HasBackground() ? pSong->GetBackgroundPath() : THEME->GetPathTo("Graphics","fallback background") );
				m_BackgroundAnimations.Add( pTempBGA );

				m_aBGSegments.Add( BGSegment(aniseg.m_fStartBeat, m_BackgroundAnimations.GetSize()-1) );	// add to the plan
				continue;	// stop looking for this background
			}

			// Look for movies in the RandomMovies dir
			GetDirListing( RANDOMMOVIES_DIR+aniseg.m_sBGName+".avi", asFiles, false, true );
			GetDirListing( RANDOMMOVIES_DIR+aniseg.m_sBGName+".mpg", asFiles, false, true );
			GetDirListing( RANDOMMOVIES_DIR+aniseg.m_sBGName+".mpeg", asFiles, false, true );
			if( asFiles.GetSize() > 0 )
			{
				pTempBGA = new BackgroundAnimation;
				pTempBGA->LoadFromMovie( asFiles[0], true, false );
				m_BackgroundAnimations.Add( pTempBGA );

				m_aBGSegments.Add( BGSegment(aniseg.m_fStartBeat, m_BackgroundAnimations.GetSize()-1) );	// add to the plan
				continue;	// stop looking for this background
			}

			// Look for BGAnims in the BGAnims dir
			GetDirListing( BG_ANIMS_DIR+aniseg.m_sBGName, asFiles, true, true );
			if( asFiles.GetSize() > 0 )
			{
				pTempBGA = new BackgroundAnimation;
				pTempBGA->LoadFromAniDir( asFiles[0], pSong->HasBackground() ? pSong->GetBackgroundPath() : THEME->GetPathTo("Graphics","fallback background") );
				m_BackgroundAnimations.Add( pTempBGA );

				m_aBGSegments.Add( BGSegment(aniseg.m_fStartBeat, m_BackgroundAnimations.GetSize()-1) );	// add to the plan
				continue;	// stop looking for this background
			}
		}

		// end showing the static song background
		m_aBGSegments.Add( BGSegment(pSong->m_fLastBeat,0) );

		SortBGSegmentArray( m_aBGSegments );	// Need to sort in case the song has a background change after the last beat
	}
	else	// pSong doesn't have an animation plan
	{
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
				pTempBGA->LoadFromMovie( pSong->GetMovieBackgroundPath(), false, true );
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
					pTempBGA->LoadFromMovie( arrayPossibleMovies[index], true, false );
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
			m_aBGSegments.Add( BGSegment(-10000,1) );
			return;
		}

		// start off showing the static song background
		m_aBGSegments.Add( BGSegment(-10000,0) );

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
			m_aBGSegments.Add( BGSegment(f,index) );
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
			m_aBGSegments.Add( BGSegment(bpmseg.m_fStartBeat,index) );
		}

		// end showing the static song background
		m_aBGSegments.Add( BGSegment(pSong->m_fLastBeat,0) );

		// sort segments
		SortBGSegmentArray( m_aBGSegments );


	}


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

		// Find the BGSegment we're in
		for( int i=0; i<m_aBGSegments.GetSize()-1; i++ )
			if( GAMESTATE->m_fSongBeat < m_aBGSegments[i+1].m_fStartBeat )
				break;
		ASSERT( i >= 0  &&  i<m_aBGSegments.GetSize() );

		if( i > m_iCurBGSegment )
		{
//			printf( "%d, %d, %f, %f\n", m_iCurBGSegment, i, m_aBGSegments[i].m_fStartBeat, GAMESTATE->m_fSongBeat );
			GetCurBGA()->LosingFocus();
			m_iCurBGSegment = i;
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
