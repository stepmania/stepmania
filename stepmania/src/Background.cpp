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
#define LEFT_EDGE			THEME->GetMetricI("Background","LeftEdge")
#define TOP_EDGE			THEME->GetMetricI("Background","TopEdge")
#define RIGHT_EDGE			THEME->GetMetricI("Background","RightEdge")
#define BOTTOM_EDGE			THEME->GetMetricI("Background","BottomEdge")

#define RECT_BACKGROUND CRect(LEFT_EDGE,TOP_EDGE,RIGHT_EDGE,BOTTOM_EDGE)

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

	CStringArray asPossibleDangerMovies;
	GetDirListing( RANDOMMOVIES_DIR+"danger.avi", asPossibleDangerMovies, false, true );
	GetDirListing( RANDOMMOVIES_DIR+"danger.mpg", asPossibleDangerMovies, false, true );
	GetDirListing( RANDOMMOVIES_DIR+"danger.mpeg", asPossibleDangerMovies, false, true );

	if( asPossibleDangerMovies.GetSize()>0 )
		m_BGADanger.LoadFromMovie( asPossibleDangerMovies[0], true, false );
	else if( DoesFileExist(BG_ANIMS_DIR+"danger\\") )
		m_BGADanger.LoadFromAniDir( BG_ANIMS_DIR+"danger\\", "" );
	else
		PREFSMAN->m_bShowDanger = false;

	m_quadBGBrightness.StretchTo( RECT_BACKGROUND );
	m_quadBGBrightness.SetDiffuse( D3DXCOLOR(0,0,0,1-PREFSMAN->m_fBGBrightness) );

	m_quadBorder[0].StretchTo( CRect(SCREEN_LEFT,SCREEN_TOP,LEFT_EDGE,SCREEN_BOTTOM) );
	m_quadBorder[0].SetDiffuse( D3DXCOLOR(0,0,0,1) );
	m_quadBorder[1].StretchTo( CRect(LEFT_EDGE,SCREEN_TOP,RIGHT_EDGE,TOP_EDGE) );
	m_quadBorder[1].SetDiffuse( D3DXCOLOR(0,0,0,1) );
	m_quadBorder[2].StretchTo( CRect(RIGHT_EDGE,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );
	m_quadBorder[2].SetDiffuse( D3DXCOLOR(0,0,0,1) );
	m_quadBorder[3].StretchTo( CRect(LEFT_EDGE,BOTTOM_EDGE,RIGHT_EDGE,SCREEN_BOTTOM) );
	m_quadBorder[3].SetDiffuse( D3DXCOLOR(0,0,0,1) );
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
	
	m_aBGSegments.RemoveAll();
	m_iCurBGSegment = 0;
}

void Background::LoadFromSong( Song* pSong )
{
	Unload();

	const float fXZoom = RECTWIDTH(RECT_BACKGROUND) / (float)SCREEN_WIDTH;
	const float fYZoom = RECTHEIGHT(RECT_BACKGROUND) / (float)SCREEN_HEIGHT;
	const float fZoom = max(fXZoom,fYZoom);


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


	const CString sSongBackgroundPath = pSong->HasBackground() ? pSong->GetBackgroundPath() : THEME->GetPathTo("Graphics","fallback background");

	//
	// Load the static background that will before notes start and after notes end
	//
	BackgroundAnimation* pTempBGA;

	pTempBGA = new BackgroundAnimation;
	pTempBGA->LoadFromStaticGraphic( sSongBackgroundPath );
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
				pTempBGA->LoadFromMovie( asFiles[0], true, true, i==0/*first BGChange*/, sSongBackgroundPath );
				m_BackgroundAnimations.Add( pTempBGA );

				m_aBGSegments.Add( BGSegment(aniseg.m_fStartBeat, m_BackgroundAnimations.GetSize()-1) );	// add to the plan
				continue;	// stop looking for this background
			}

			// Look for BGAnims in the song dir
			GetDirListing( pSong->m_sSongDir+aniseg.m_sBGName, asFiles, true, true );
			if( asFiles.GetSize() > 0 )
			{
				pTempBGA = new BackgroundAnimation;
				pTempBGA->LoadFromAniDir( asFiles[0], sSongBackgroundPath );
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
				pTempBGA->LoadFromMovie( asFiles[0], true, false, i==0/*first BGChange*/, sSongBackgroundPath );
				m_BackgroundAnimations.Add( pTempBGA );

				m_aBGSegments.Add( BGSegment(aniseg.m_fStartBeat, m_BackgroundAnimations.GetSize()-1) );	// add to the plan
				continue;	// stop looking for this background
			}

			// Look for BGAnims in the BGAnims dir
			GetDirListing( BG_ANIMS_DIR+aniseg.m_sBGName, asFiles, true, true );
			if( asFiles.GetSize() > 0 )
			{
				pTempBGA = new BackgroundAnimation;
				pTempBGA->LoadFromAniDir( asFiles[0], sSongBackgroundPath  );
				m_BackgroundAnimations.Add( pTempBGA );

				m_aBGSegments.Add( BGSegment(aniseg.m_fStartBeat, m_BackgroundAnimations.GetSize()-1) );	// add to the plan
				continue;	// stop looking for this background
			}

			// if we make it here, ignore the background change
		}

		// end showing the static song background
		m_aBGSegments.Add( BGSegment(pSong->m_fLastBeat,0) );

		SortBGSegmentArray( m_aBGSegments );	// Need to sort in case there is a background change after the last beat (not likely)
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
				pTempBGA->LoadFromMovie( pSong->GetMovieBackgroundPath(), false, true, true, sSongBackgroundPath );
				m_BackgroundAnimations.Add( pTempBGA );
			}
			break;
		case MODE_MOVIE_VIS:
			{
				CStringArray arrayPossibleMovies;
				GetDirListing( VISUALIZATIONS_DIR + "*.avi", arrayPossibleMovies, false, true );
				GetDirListing( VISUALIZATIONS_DIR + "*.mpg", arrayPossibleMovies, false, true );
				GetDirListing( VISUALIZATIONS_DIR + "*.mpeg", arrayPossibleMovies, false, true );

				int index = rand() % arrayPossibleMovies.GetSize();
				pTempBGA = new BackgroundAnimation;
				pTempBGA->LoadFromVisualization( arrayPossibleMovies[index], sSongBackgroundPath );
				m_BackgroundAnimations.Add( pTempBGA );
				arrayPossibleMovies.RemoveAt( index );
			}
			break;
		case MODE_ANIMATIONS:
			{
				CStringArray arrayPossibleAnims;
				GetDirListing( BG_ANIMS_DIR+"*.*", arrayPossibleAnims, true, true );
				// strip out "cvs" and "danger"
				for( int i=arrayPossibleAnims.GetSize()-1; i>=0; i-- )
					if( 0==stricmp(arrayPossibleAnims[i].Right(3),"cvs")  ||  -1!=arrayPossibleAnims[i].Find("anger") )
						arrayPossibleAnims.RemoveAt(i);
				for( i=0; i<4 && arrayPossibleAnims.GetSize()>0; i++ )
				{
					int index = rand() % arrayPossibleAnims.GetSize();
					pTempBGA = new BackgroundAnimation;
					pTempBGA->LoadFromAniDir( arrayPossibleAnims[index], sSongBackgroundPath );
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
				for( int i=arrayPossibleMovies.GetSize()-1; i>=0; i-- )
					if( -1!=arrayPossibleMovies[i].Find("anger") )
						arrayPossibleMovies.RemoveAt(i);
				for( i=0; i<4 && arrayPossibleMovies.GetSize()>0; i++ )
				{
					int index = rand() % arrayPossibleMovies.GetSize();
					pTempBGA = new BackgroundAnimation;
					pTempBGA->LoadFromMovie( arrayPossibleMovies[index], true, false, i==0, sSongBackgroundPath );
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

		/* If we have only 2, only generate a single animation segment for for the
		 * whole song.  Otherwise, if it's a movie., it'll loop every four bars; we
		 * want it to play continuously. */
		const float fFirstBeat = (m_BackgroundMode==MODE_MOVIE_BG) ? 0 : pSong->m_fFirstBeat;
		const float fLastBeat = pSong->m_fLastBeat;

		if( m_BackgroundAnimations.GetSize() == 2) {
			m_aBGSegments.Add( BGSegment(fFirstBeat,1) );
		} else {
			// change BG every 4 bars
			for( float f=fFirstBeat; f<fLastBeat; f+=16 )
			{
				int index;
				if( m_BackgroundAnimations.GetSize()==1 )
					index = 0;
				else if( f == fFirstBeat )
					index = 1;	// force the first random background to play first
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
		}

		// end showing the static song background
		m_aBGSegments.Add( BGSegment(pSong->m_fLastBeat,0) );
		
		// sort segments
		SortBGSegmentArray( m_aBGSegments );
	}

	for( int i=0; i<m_BackgroundAnimations.GetSize(); i++ )
	{
		m_BackgroundAnimations[i]->SetXY( (float)LEFT_EDGE, (float)TOP_EDGE );
		m_BackgroundAnimations[i]->SetZoom( fZoom );
	}
		
	m_BGADanger.SetXY( (float)LEFT_EDGE, (float)TOP_EDGE );
	m_BGADanger.SetZoom( fZoom );
	
}


void Background::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	if( DangerVisible() )
	{
		m_BGADanger.Update( fDeltaTime );
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
		m_BGADanger.Draw();
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
	m_quadBGBrightness.SetTweenDiffuse( D3DXCOLOR(0,0,0,1-PREFSMAN->m_fBGBrightness) );
}

void Background::FadeOut()
{
	m_quadBGBrightness.BeginTweening( 0.5f );
	m_quadBGBrightness.SetTweenDiffuse( D3DXCOLOR(0,0,0,1-0.5f) );

}
