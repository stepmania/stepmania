#include "global.h"
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
#include "NoteTypes.h"
#include <math.h>	// for fmodf


const float FADE_SECONDS = 1.0f;

#define LEFT_EDGE			THEME->GetMetricI("Background","LeftEdge")
#define TOP_EDGE			THEME->GetMetricI("Background","TopEdge")
#define RIGHT_EDGE			THEME->GetMetricI("Background","RightEdge")
#define BOTTOM_EDGE			THEME->GetMetricI("Background","BottomEdge")

#define RECT_BACKGROUND RectI(LEFT_EDGE,TOP_EDGE,RIGHT_EDGE,BOTTOM_EDGE)


const CString STATIC_BACKGROUND = "static background";
const int MAX_RANDOM_BACKGROUNDS = 5;
const CString RANDOM_BACKGROUND[MAX_RANDOM_BACKGROUNDS] = 
{
	"__random1",
	"__random2",
	"__random3",
	"__random4",
	"__random5"
};


Background::Background()
{
	m_iCurBGChange = 0;
	m_bInDanger = false;

	m_pFadingBGA = NULL;
	m_fSecsLeftInFade = 0;

	m_BGADanger.LoadFromAniDir( THEME->GetPathTo("BGAnimations","ScreenGameplay danger") );

	m_quadBGBrightness.StretchTo( RECT_BACKGROUND );
	m_quadBGBrightness.SetDiffuse( RageColor(0,0,0,1-PREFSMAN->m_fBGBrightness) );

	m_quadBorder[0].StretchTo( RectI(SCREEN_LEFT,SCREEN_TOP,LEFT_EDGE,SCREEN_BOTTOM) );
	m_quadBorder[0].SetDiffuse( RageColor(0,0,0,1) );
	m_quadBorder[1].StretchTo( RectI(LEFT_EDGE,SCREEN_TOP,RIGHT_EDGE,TOP_EDGE) );
	m_quadBorder[1].SetDiffuse( RageColor(0,0,0,1) );
	m_quadBorder[2].StretchTo( RectI(RIGHT_EDGE,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );
	m_quadBorder[2].SetDiffuse( RageColor(0,0,0,1) );
	m_quadBorder[3].StretchTo( RectI(LEFT_EDGE,BOTTOM_EDGE,RIGHT_EDGE,SCREEN_BOTTOM) );
	m_quadBorder[3].SetDiffuse( RageColor(0,0,0,1) );
}

Background::~Background()
{
	Unload();
}

void Background::Unload()
{
    for( map<CString,BGAnimation*>::iterator iter = m_BGAnimations.begin();
		 iter != m_BGAnimations.end();
		 iter++ )
		delete iter->second;
	m_BGAnimations.clear();
	
	m_aBGChanges.clear();
	m_iCurBGChange = 0;
}

void Background::LoadFromAniDir( CString sAniDir )
{
	Unload();

	BGAnimation* pTempBGA;
	pTempBGA = new BGAnimation;
	pTempBGA->LoadFromAniDir( sAniDir );
	m_BGAnimations[STATIC_BACKGROUND] = pTempBGA;
}

BGAnimation *Background::CreateSongBGA(const Song *pSong, CString sBGName) const
{
	BGAnimation *pTempBGA;

	// Using aniseg.m_sBGName, search for the corresponding animation.
	// Look in this order:  movies in song dir, BGAnims in song dir
	//  movies in RandomMovies dir, BGAnims in BGAnimsDir.
	CStringArray asFiles;

	// Look for BGAnims in the song dir
	GetDirListing( pSong->GetSongDir()+sBGName, asFiles, true, true );
	if( !asFiles.empty() )
	{
		pTempBGA = new BGAnimation;
		pTempBGA->LoadFromAniDir( asFiles[0] );
		return pTempBGA;
	}
	// Look for BG movies in the song dir
	GetDirListing( pSong->GetSongDir()+sBGName, asFiles, false, true );
	if( !asFiles.empty() )
	{
		pTempBGA = new BGAnimation;
		pTempBGA->LoadFromMovie( asFiles[0], true, true );
		return pTempBGA;
	}
	// Look for movies in the RandomMovies dir
	GetDirListing( RANDOMMOVIES_DIR+sBGName, asFiles, false, true );
	if( !asFiles.empty() )
	{
		pTempBGA = new BGAnimation;
		pTempBGA->LoadFromMovie( asFiles[0], true, false );
		return pTempBGA;
	}

	// Look for BGAnims in the BGAnims dir
	GetDirListing( BG_ANIMS_DIR+sBGName, asFiles, true, true );
	if( !asFiles.empty() )
	{
		pTempBGA = new BGAnimation;
		pTempBGA->LoadFromAniDir( asFiles[0] );
		return pTempBGA;
	}

	// Look for BGAnims in the BGAnims dir
	GetDirListing( VISUALIZATIONS_DIR+sBGName, asFiles, false, true );
	if( !asFiles.empty() )
	{
		pTempBGA = new BGAnimation;
		pTempBGA->LoadFromVisualization( asFiles[0] );
		return pTempBGA;
	}

	// There is no background by this name.  
	return NULL;
}

BGAnimation* Background::CreateRandomBGA() const
{
	switch( PREFSMAN->m_BackgroundMode )
	{
	default:
		ASSERT(0);
		// fall through
	case PrefsManager::BGMODE_OFF:
		return NULL;
	case PrefsManager::BGMODE_ANIMATIONS:
		{
			CStringArray arrayPossibleAnims;
			GetDirListing( BG_ANIMS_DIR+"*", arrayPossibleAnims, true, true );
			// strip out "cvs" and "danger
			int i;
			for( i=arrayPossibleAnims.size()-1; i>=0; i-- )
				if( 0==stricmp(arrayPossibleAnims[i].Right(3),"cvs") || 0==stricmp(arrayPossibleAnims[i].Right(3),"danger") )
					arrayPossibleAnims.erase(arrayPossibleAnims.begin()+i,
												arrayPossibleAnims.begin()+i+1);

			if( arrayPossibleAnims.empty() )
				return NULL;
			unsigned index = rand() % arrayPossibleAnims.size();
			BGAnimation *pTempBGA = new BGAnimation;
			pTempBGA->LoadFromAniDir( arrayPossibleAnims[index] );
			return pTempBGA;
		}
	case PrefsManager::BGMODE_MOVIEVIS:
		{
			CStringArray arrayPossibleMovies;
			GetDirListing( VISUALIZATIONS_DIR + "*.avi", arrayPossibleMovies, false, true );
			GetDirListing( VISUALIZATIONS_DIR + "*.mpg", arrayPossibleMovies, false, true );
			GetDirListing( VISUALIZATIONS_DIR + "*.mpeg", arrayPossibleMovies, false, true );

			if( arrayPossibleMovies.empty() )
				return NULL;
			unsigned index = rand() % arrayPossibleMovies.size();
			BGAnimation* pTempBGA = new BGAnimation;
			pTempBGA->LoadFromVisualization( arrayPossibleMovies[index] );
			return pTempBGA;
		}
		break;
	case PrefsManager::BGMODE_RANDOMMOVIES:
		{
			CStringArray arrayPossibleMovies;
			GetDirListing( RANDOMMOVIES_DIR + "*.avi", arrayPossibleMovies, false, true );
			GetDirListing( RANDOMMOVIES_DIR + "*.mpg", arrayPossibleMovies, false, true );
			GetDirListing( RANDOMMOVIES_DIR + "*.mpeg", arrayPossibleMovies, false, true );

			if( arrayPossibleMovies.empty() )
				return NULL;
			unsigned index = rand() % arrayPossibleMovies.size();
			BGAnimation *pTempBGA = new BGAnimation;
			pTempBGA->LoadFromMovie( arrayPossibleMovies[index], true, false );
			return pTempBGA;
		}
		break;
	}
}

void Background::LoadFromSong( Song* pSong )
{
	Unload();

	const float fXZoom = RECT_BACKGROUND.GetWidth() / (float)SCREEN_WIDTH;
	const float fYZoom = RECT_BACKGROUND.GetHeight() / (float)SCREEN_HEIGHT;

	CString sSongBGPath = pSong && pSong->HasBackground() ? pSong->GetBackgroundPath() : THEME->GetPathTo("Graphics","Common fallback background");

	// Load the static background that will before notes start and after notes end
	{
		BGAnimation *pTempBGA = new BGAnimation;
		pTempBGA->LoadFromStaticGraphic( sSongBGPath );
		m_BGAnimations[STATIC_BACKGROUND] = pTempBGA;
	}


	// Load random backgrounds
	{
		for( int i=0; i<MAX_RANDOM_BACKGROUNDS; i++ )
		{
			CString sBGName = RANDOM_BACKGROUND[ i ];
			m_BGAnimations[sBGName] = CreateRandomBGA();
		}
	}

	// start off showing the static song background
	m_aBGChanges.push_back( BackgroundChange(-10000,STATIC_BACKGROUND) );


	if( pSong->HasBGChanges() )
	{
		// Load all song-specified backgrounds
		for( unsigned i=0; i<pSong->m_BackgroundChanges.size(); i++ )
		{
			float fStartBeat = pSong->m_BackgroundChanges[i].m_fStartBeat;
			CString sBGName = pSong->m_BackgroundChanges[i].m_sBGName;
			
			bool bIsAlreadyLoaded = m_BGAnimations.find(sBGName) != m_BGAnimations.end();

			if( !bIsAlreadyLoaded )
			{
				BGAnimation *pTempBGA = CreateSongBGA(pSong, sBGName);
				if( pTempBGA )
					m_BGAnimations[sBGName] = pTempBGA;
				else // the background was not found.  Use a random one instead
					sBGName = RANDOM_BACKGROUND[ rand()%MAX_RANDOM_BACKGROUNDS ];
			}
			
			m_aBGChanges.push_back( BackgroundChange(fStartBeat, sBGName) );
		}
	}
	else	// pSong doesn't have an animation plan
	{
		const float fFirstBeat = pSong->m_fFirstBeat;
		const float fLastBeat = pSong->m_fLastBeat;

		// change BG every 4 bars
		for( float f=fFirstBeat; f<fLastBeat; f+=BEATS_PER_MEASURE*4 )
		{
			CString sBGName = RANDOM_BACKGROUND[ rand()%MAX_RANDOM_BACKGROUNDS ];
			m_aBGChanges.push_back( BackgroundChange(f,sBGName) );
		}

		// change BG every BPM change that is at the beginning of a measure
		for( unsigned i=0; i<pSong->m_BPMSegments.size(); i++ )
		{
			const BPMSegment& bpmseg = pSong->m_BPMSegments[i];

			if( fmodf(bpmseg.m_fStartBeat, (float)BEATS_PER_MEASURE) != 0 )
				continue;	// skip

			if( bpmseg.m_fStartBeat < fFirstBeat  || bpmseg.m_fStartBeat > fLastBeat )
				continue;	// skip

			CString sBGName = RANDOM_BACKGROUND[ rand()%MAX_RANDOM_BACKGROUNDS ];
			m_aBGChanges.push_back( BackgroundChange(bpmseg.m_fStartBeat,sBGName) );
		}
	}


	// end showing the static song background
	m_aBGChanges.push_back( BackgroundChange(pSong->m_fLastBeat,STATIC_BACKGROUND) );

		
	// sort segments
	SortBackgroundChangesArray( m_aBGChanges );



    for( map<CString,BGAnimation*>::iterator iter = m_BGAnimations.begin();
		 iter != m_BGAnimations.end();
		 iter++ )
	{
		iter->second->SetXY( (float)LEFT_EDGE, (float)TOP_EDGE );
		iter->second->SetZoomX( fXZoom );
		iter->second->SetZoomY( fYZoom );
	}
		
	m_BGADanger.SetXY( (float)LEFT_EDGE, (float)TOP_EDGE );
	m_BGADanger.SetZoomX( fXZoom );
	m_BGADanger.SetZoomY( fYZoom );	
}


void Background::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	if( IsDangerVisible() )
	{
		m_BGADanger.Update( fDeltaTime );
	}
	else
	{			
		if( GAMESTATE->m_fMusicSeconds == GameState::MUSIC_SECONDS_INVALID )
			return; /* hasn't been updated yet */

		/* If we're in a freeze, hold all animations (don't animate by calling Update). */
		if( GAMESTATE->m_bFreeze )
			return;

		// Find the BGSegment we're in
		unsigned i;
		for( i=0; i<m_aBGChanges.size()-1; i++ )
			if( GAMESTATE->m_fSongBeat < m_aBGChanges[i+1].m_fStartBeat )
				break;
		ASSERT( i >= 0  &&  i<m_aBGChanges.size() );

		if( int(i) > m_iCurBGChange )
		{
//			LOG->Trace( "%d, %d, %f, %f\n", m_iCurBGSegment, i, m_aBGSegments[i].m_fStartBeat, GAMESTATE->m_fSongBeat );
			BGAnimation* pOld = GetCurrentBGA();
			m_iCurBGChange = i;
			BGAnimation* pNew = GetCurrentBGA();

			if( pOld != pNew )
			{
				pOld->LosingFocus();
				pNew->GainingFocus();

				m_pFadingBGA = pOld;
				bool bBGAnimsMode = PREFSMAN->m_BackgroundMode == PrefsManager::BGMODE_ANIMATIONS;
				m_fSecsLeftInFade = bBGAnimsMode ? 0 : FADE_SECONDS;
			}
		}

		GetCurrentBGA()->Update( fDeltaTime );
		if( m_pFadingBGA )
		{
			m_pFadingBGA->Update( fDeltaTime );
			m_fSecsLeftInFade -= fDeltaTime;
			float fPercentOpaque = m_fSecsLeftInFade / FADE_SECONDS;
			m_pFadingBGA->SetDiffuse( RageColor(1,1,1,fPercentOpaque) );
			if( fPercentOpaque <= 0 )
				m_pFadingBGA = NULL;
		}
	}
	
	m_quadBGBrightness.Update( fDeltaTime );
}

void Background::DrawPrimitives()
{
	ActorFrame::DrawPrimitives();
	
	if( IsDangerVisible() )
	{
		m_BGADanger.Draw();
	}
	else
	{			
		GetCurrentBGA()->Draw();
		if( m_pFadingBGA )
			m_pFadingBGA->Draw();
	}

	m_quadBGBrightness.Draw();
	for( int i=0; i<4; i++ )
		m_quadBorder[i].Draw();
}

bool Background::IsDangerVisible()
{
	return m_bInDanger  &&  PREFSMAN->m_bShowDanger  &&  (RageTimer::GetTimeSinceStart() - (int)RageTimer::GetTimeSinceStart()) < 0.5f;
}


void Background::FadeIn()
{
	m_quadBGBrightness.BeginTweening( 0.5f );
	m_quadBGBrightness.SetTweenDiffuse( RageColor(0,0,0,1-PREFSMAN->m_fBGBrightness) );
}

void Background::FadeOut()
{
	m_quadBGBrightness.BeginTweening( 0.5f );
	m_quadBGBrightness.SetTweenDiffuse( RageColor(0,0,0,1-0.5f) );

}
