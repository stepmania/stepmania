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

const float FADE_SECONDS = 1.0f;

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

	m_pFadingBGA = NULL;
	m_fSecsLeftInFade = 0;

	m_BGADanger.LoadFromAniDir( THEME->GetPathTo("BGAnimations","gameplay danger") );

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
    for( int i=0; i<m_BGAnimations.GetSize(); i++ )
		delete m_BGAnimations[i];
	m_BGAnimations.RemoveAll();
	
	m_aBGSegments.RemoveAll();
	m_iCurBGSegment = 0;
}

void Background::LoadFromAniDir( CString sAniDir )
{
	Unload();

	BGAnimation* pTempBGA;
	pTempBGA = new BGAnimation;
	pTempBGA->LoadFromAniDir( sAniDir );
	m_BGAnimations.Add( pTempBGA );
}

void Background::LoadFromSong( Song* pSong )
{
	Unload();

	const float fXZoom = RECTWIDTH(RECT_BACKGROUND) / (float)SCREEN_WIDTH;
	const float fYZoom = RECTHEIGHT(RECT_BACKGROUND) / (float)SCREEN_HEIGHT;

	const CString sSongBackgroundPath = pSong->HasBackground() ? pSong->GetBackgroundPath() : THEME->GetPathTo("Graphics","fallback background");

	//
	// Load the static background that will before notes start and after notes end
	//
	BGAnimation* pTempBGA;

	
	// Tricky!  The song background looks terrible unless its loaded with no alpha 
	// and dithered.  Create a dummy sprite that loads the texture with the proper 
	// hints so we don't have to hack up BGAnimation to handle this special
	// case.
	Sprite sprDummy;
	sprDummy.Load( sSongBackgroundPath, true, 4, 0, true, false );
	

	pTempBGA = new BGAnimation;
	pTempBGA->LoadFromStaticGraphic( sSongBackgroundPath );
	m_BGAnimations.Add( pTempBGA );


	if( pSong->HasBGChanges() )
	{
		// start off showing the static song background
		m_aBGSegments.Add( BGSegment(-10000,0,false) );


		// Load the animations used by the song's pre-defined animation plan.
		// the song has a plan.  Use it.
		for( int i=0; i<pSong->m_BackgroundChanges.GetSize(); i++ )
		{
			const BackgroundChange& aniseg = pSong->m_BackgroundChanges[i];
			bool bFade = i==0;

			// Using aniseg.m_sBGName, search for the corresponding animation.
			// Look in this order:  movies in song dir, BGAnims in song dir
			//  movies in RandomMovies dir, BGAnims in BGAnimsDir.
			CStringArray asFiles;

			// Look for movies in the song dir
			GetDirListing( pSong->m_sSongDir+aniseg.m_sBGName+".avi", asFiles, false, true );
			GetDirListing( pSong->m_sSongDir+aniseg.m_sBGName+".mpg", asFiles, false, true );
			GetDirListing( pSong->m_sSongDir+aniseg.m_sBGName+".mpeg", asFiles, false, true );
			if( asFiles.GetSize() > 0 )
			{
				pTempBGA = new BGAnimation;
				pTempBGA->LoadFromMovie( asFiles[0], true, true );
				m_BGAnimations.Add( pTempBGA );

				m_aBGSegments.Add( BGSegment(aniseg.m_fStartBeat, m_BGAnimations.GetSize()-1, bFade) );	// add to the plan
				continue;	// stop looking for this background
			}

			// Look for BGAnims in the song dir
			GetDirListing( pSong->m_sSongDir+aniseg.m_sBGName, asFiles, true, true );
			if( asFiles.GetSize() > 0 )
			{
				pTempBGA = new BGAnimation;
				pTempBGA->LoadFromAniDir( asFiles[0], sSongBackgroundPath );
				m_BGAnimations.Add( pTempBGA );

				m_aBGSegments.Add( BGSegment(aniseg.m_fStartBeat, m_BGAnimations.GetSize()-1, bFade) );	// add to the plan
				continue;	// stop looking for this background
			}

			// Look for movies in the RandomMovies dir
			GetDirListing( RANDOMMOVIES_DIR+aniseg.m_sBGName+".avi", asFiles, false, true );
			GetDirListing( RANDOMMOVIES_DIR+aniseg.m_sBGName+".mpg", asFiles, false, true );
			GetDirListing( RANDOMMOVIES_DIR+aniseg.m_sBGName+".mpeg", asFiles, false, true );
			if( asFiles.GetSize() > 0 )
			{
				pTempBGA = new BGAnimation;
				pTempBGA->LoadFromMovie( asFiles[0], true, false );
				m_BGAnimations.Add( pTempBGA );

				m_aBGSegments.Add( BGSegment(aniseg.m_fStartBeat, m_BGAnimations.GetSize()-1, bFade) );	// add to the plan
				continue;	// stop looking for this background
			}

			// Look for BGAnims in the BGAnims dir
			GetDirListing( BG_ANIMS_DIR+aniseg.m_sBGName, asFiles, true, true );
			if( asFiles.GetSize() > 0 )
			{
				pTempBGA = new BGAnimation;
				pTempBGA->LoadFromAniDir( asFiles[0], sSongBackgroundPath  );
				m_BGAnimations.Add( pTempBGA );

				m_aBGSegments.Add( BGSegment(aniseg.m_fStartBeat, m_BGAnimations.GetSize()-1, bFade) );	// add to the plan
				continue;	// stop looking for this background
			}

			// if we make it here, ignore the background change
		}

		// end showing the static song background
		m_aBGSegments.Add( BGSegment(pSong->m_fLastBeat,0,false) );

		SortBGSegmentArray( m_aBGSegments );	// Need to sort in case there is a background change after the last beat (not likely)
	}
	else	// pSong doesn't have an animation plan
	{

		enum BackgroundMode { MODE_STATIC_BG, MODE_ANIMATIONS, MODE_MOVIE_VIS, MODE_RANDOMMOVIES } backgroundMode;

		//
		// figure out what BackgroundMode to use
		//
		if( PREFSMAN->m_BackgroundMode == PrefsManager::BGMODE_OFF )
			backgroundMode = MODE_STATIC_BG;
		else if( PREFSMAN->m_BackgroundMode == PrefsManager::BGMODE_MOVIEVIS )
			backgroundMode = MODE_MOVIE_VIS;
		else if( PREFSMAN->m_BackgroundMode == PrefsManager::BGMODE_RANDOMMOVIES )
			backgroundMode = MODE_RANDOMMOVIES;
		else
			backgroundMode = MODE_ANIMATIONS;

		bool bFade = backgroundMode==MODE_RANDOMMOVIES;

		//
		// Load animations that will play while notes are active
		//
		switch( backgroundMode )
		{
		case MODE_STATIC_BG:
			break;
		case MODE_MOVIE_VIS:
			{
				CStringArray arrayPossibleMovies;
				GetDirListing( VISUALIZATIONS_DIR + "*.avi", arrayPossibleMovies, false, true );
				GetDirListing( VISUALIZATIONS_DIR + "*.mpg", arrayPossibleMovies, false, true );
				GetDirListing( VISUALIZATIONS_DIR + "*.mpeg", arrayPossibleMovies, false, true );

				if( arrayPossibleMovies.GetSize() > 0 )
				{
					int index = rand() % arrayPossibleMovies.GetSize();
					pTempBGA = new BGAnimation;
					pTempBGA->LoadFromVisualization( arrayPossibleMovies[index], sSongBackgroundPath );
					m_BGAnimations.Add( pTempBGA );
				}
				else
				{
					pTempBGA = new BGAnimation;
					pTempBGA->LoadFromStaticGraphic( sSongBackgroundPath );
					m_BGAnimations.Add( pTempBGA );
				}
			}
			break;
		case MODE_ANIMATIONS:
			{
				CStringArray arrayPossibleAnims;
				GetDirListing( BG_ANIMS_DIR+"*.*", arrayPossibleAnims, true, true );
				// strip out "cvs" and "danger
				for( int i=arrayPossibleAnims.GetSize()-1; i>=0; i-- )
					if( 0==stricmp(arrayPossibleAnims[i].Right(3),"cvs") || 0==stricmp(arrayPossibleAnims[i].Right(3),"danger") )
						arrayPossibleAnims.RemoveAt(i);
				for( i=0; i<4 && arrayPossibleAnims.GetSize()>0; i++ )
				{
					int index = rand() % arrayPossibleAnims.GetSize();
					pTempBGA = new BGAnimation;
					pTempBGA->LoadFromAniDir( arrayPossibleAnims[index], sSongBackgroundPath );
					m_BGAnimations.Add( pTempBGA );
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
				for( int i=0; i<4 && arrayPossibleMovies.GetSize()>0; i++ )
				{
					int index = rand() % arrayPossibleMovies.GetSize();
					pTempBGA = new BGAnimation;
					pTempBGA->LoadFromMovie( arrayPossibleMovies[index], true, false );
					m_BGAnimations.Add( pTempBGA );
					arrayPossibleMovies.RemoveAt( index );
				}	
			}
			break;
		default:
			ASSERT(0);
		}


		// At this point, m_BGAnimations[0] is the song background, and everything else
		// in m_BGAnimations should be cycled through randomly while notes are playing.	
		//
		// Generate an animation plan
		//
		if( backgroundMode == MODE_MOVIE_VIS )
		{
			m_aBGSegments.Add( BGSegment(-10000,1,false) );
			return;
		}
		else
		{
			// start off showing the static song background
			m_aBGSegments.Add( BGSegment(-10000,0,false) );
		}

		/* If we have only 2, only generate a single animation segment for for the
		 * whole song.  Otherwise, if it's a movie., it'll loop every four bars; we
		 * want it to play continuously. */
		const float fFirstBeat = pSong->m_fFirstBeat;
		const float fLastBeat = pSong->m_fLastBeat;

		if( m_BGAnimations.GetSize() == 2) {
			m_aBGSegments.Add( BGSegment(fFirstBeat,1,bFade) );
		} else {
			// change BG every 4 bars
			for( float f=fFirstBeat; f<fLastBeat; f+=16 )
			{
				int index;
				if( m_BGAnimations.GetSize()==1 )
					index = 0;
				else if( f == fFirstBeat )
					index = 1;	// force the first random background to play first
				else
					index = 1 + rand()%(m_BGAnimations.GetSize()-1);
				m_aBGSegments.Add( BGSegment(f,index,bFade || f==fFirstBeat) );
			}

			// change BG every BPM change
			for( int i=0; i<pSong->m_BPMSegments.GetSize(); i++ )
			{
				const BPMSegment& bpmseg = pSong->m_BPMSegments[i];

				if( bpmseg.m_fStartBeat < fFirstBeat  || bpmseg.m_fStartBeat > fLastBeat )
					continue;	// skip]

				int index;
				if( m_BGAnimations.GetSize()==1 )
					index = 0;
				else
					index = 1 + int(bpmseg.m_fBPM)%(m_BGAnimations.GetSize()-1);
				m_aBGSegments.Add( BGSegment(bpmseg.m_fStartBeat,index,bFade) );
			}
		}

		// end showing the static song background
		m_aBGSegments.Add( BGSegment(pSong->m_fLastBeat,0,bFade) );
		
		// sort segments
		SortBGSegmentArray( m_aBGSegments );
	}

	for( int i=0; i<m_BGAnimations.GetSize(); i++ )
	{
		m_BGAnimations[i]->SetXY( (float)LEFT_EDGE, (float)TOP_EDGE );
		m_BGAnimations[i]->SetZoomX( fXZoom );
		m_BGAnimations[i]->SetZoomY( fYZoom );
	}
		
	m_BGADanger.SetXY( (float)LEFT_EDGE, (float)TOP_EDGE );
	m_BGADanger.SetZoomX( fXZoom );
	m_BGADanger.SetZoomY( fYZoom );	
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
			BGAnimation* pOld = GetCurBGA();
			m_iCurBGSegment = i;
			BGAnimation* pNew = GetCurBGA();

			if( pOld != pNew )
			{
				pOld->LosingFocus();
				pNew->GainingFocus();

				m_pFadingBGA = pOld;
				m_fSecsLeftInFade = m_aBGSegments[m_iCurBGSegment].m_bFade ? FADE_SECONDS : 0;
			}
		}

		GetCurBGA()->Update( fDeltaTime );
		if( m_pFadingBGA )
		{
			m_pFadingBGA->Update( fDeltaTime );
			m_fSecsLeftInFade -= fDeltaTime;
			float fPercentOpaque = m_fSecsLeftInFade / FADE_SECONDS;
			m_pFadingBGA->SetDiffuse( D3DXCOLOR(1,1,1,fPercentOpaque) );
			if( fPercentOpaque <= 0 )
				m_pFadingBGA = NULL;
		}
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
		if( m_pFadingBGA )
			m_pFadingBGA->Draw();
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
