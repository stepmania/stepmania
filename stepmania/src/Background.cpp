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
#include "RageLog.h"
#include "RageTextureManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "PrefsManager.h"
#include "NoteTypes.h"
#include <math.h>	// for fmodf
#include "DancingCharacters.h"
#include "arch/arch.h"


const float FADE_SECONDS = 1.0f;

#define LEFT_EDGE			THEME->GetMetricI("Background","LeftEdge")
#define TOP_EDGE			THEME->GetMetricI("Background","TopEdge")
#define RIGHT_EDGE			THEME->GetMetricI("Background","RightEdge")
#define BOTTOM_EDGE			THEME->GetMetricI("Background","BottomEdge")

#define RECT_BACKGROUND RectI(LEFT_EDGE,TOP_EDGE,RIGHT_EDGE,BOTTOM_EDGE)


const CString STATIC_BACKGROUND = "static background";

CString RandomBackground(int num) { return ssprintf("__random%i", num); }

Background::Background()
{
	m_bInDanger = false;

	m_iCurBGChangeIndex = -1;
	m_pCurrentBGA = NULL;
	m_pFadingBGA = NULL;
	m_fSecsLeftInFade = 0;

	m_BGADanger.LoadFromAniDir( THEME->GetPathToB("ScreenGameplay danger") );

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

	bool bOneOrMoreChars = false;
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsPlayerEnabled(p) )
			bOneOrMoreChars = true;
	if( bOneOrMoreChars )
		m_pDancingCharacters = new DancingCharacters;
	else
		m_pDancingCharacters = NULL;
}

Background::~Background()
{
	Unload();
	delete m_pDancingCharacters;
}

void Background::Unload()
{
    for( map<CString,BGAnimation*>::iterator iter = m_BGAnimations.begin();
		 iter != m_BGAnimations.end();
		 iter++ )
		delete iter->second;
	m_BGAnimations.clear();
	
	m_aBGChanges.clear();
	m_pCurrentBGA = NULL;
	m_pFadingBGA = NULL;
}

void Background::LoadFromAniDir( CString sAniDir )
{
	Unload();

	if( PREFSMAN->m_fBGBrightness == 0 )
		return;

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
	// Look for BG movies or static graphics in the song dir
	GetDirListing( pSong->GetSongDir()+sBGName, asFiles, false, true );
	if( !asFiles.empty() )
	{
		pTempBGA = new BGAnimation;
		CString sThrowAway, sExt;
		splitrelpath( asFiles[0], sThrowAway, sThrowAway, sExt );
		if( sExt.CompareNoCase("avi")==0 ||
			sExt.CompareNoCase("mpg")==0 ||
			sExt.CompareNoCase("mpeg")==0 )
			pTempBGA->LoadFromMovie( asFiles[0] );
		else
			pTempBGA->LoadFromStaticGraphic( asFiles[0] );
		return pTempBGA;
	}
	// Look for movies in the RandomMovies dir
	GetDirListing( RANDOMMOVIES_DIR+sBGName, asFiles, false, true );
	if( !asFiles.empty() )
	{
		pTempBGA = new BGAnimation;
		pTempBGA->LoadFromMovie( asFiles[0] );
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
			pTempBGA->LoadFromMovie( arrayPossibleMovies[index] );
			return pTempBGA;
		}
		break;
	}
}

void Background::LoadFromSong( Song* pSong )
{
	Unload();

	if( PREFSMAN->m_fBGBrightness == 0 )
		return;

	TEXTUREMAN->DisableOddDimensionWarning();

	const float fXZoom = RECT_BACKGROUND.GetWidth() / (float)SCREEN_WIDTH;
	const float fYZoom = RECT_BACKGROUND.GetHeight() / (float)SCREEN_HEIGHT;

	CString sSongBGPath = pSong && pSong->HasBackground() ? pSong->GetBackgroundPath() : THEME->GetPathToG("Common fallback background");

	// Load the static background that will before notes start and after notes end
	{
		BGAnimation *pTempBGA = new BGAnimation;
		pTempBGA->LoadFromStaticGraphic( sSongBGPath );
		m_BGAnimations[STATIC_BACKGROUND] = pTempBGA;
	}


	// Load random backgrounds
	bool bLoadedAnyRandomBackgrounds = false;
	{
		for( int i=0; i<PREFSMAN->m_iNumBackgrounds; i++ )
		{
			CString sBGName = RandomBackground(i);
			BGAnimation *pTempBGA = CreateRandomBGA();
			if( pTempBGA )
			{
				m_BGAnimations[sBGName] = pTempBGA;
				bLoadedAnyRandomBackgrounds = true;
			}
		}
	}

	// start off showing the static song background
	m_aBGChanges.push_back( BackgroundChange(-10000,STATIC_BACKGROUND) );


	if( pSong->HasBGChanges() )
	{
		// Load all song-specified backgrounds
		for( unsigned i=0; i<pSong->m_BackgroundChanges.size(); i++ )
		{
			BackgroundChange change = pSong->m_BackgroundChanges[i];
			CString sBGName = change.m_sBGName;
			
			bool bIsAlreadyLoaded = m_BGAnimations.find(sBGName) != m_BGAnimations.end();

			if( !bIsAlreadyLoaded )
			{
				BGAnimation *pTempBGA = CreateSongBGA(pSong, sBGName);
				if( pTempBGA )
					m_BGAnimations[sBGName] = pTempBGA;
				else // the background was not found.  Use a random one instead
					if( bLoadedAnyRandomBackgrounds )
						sBGName = RandomBackground( rand()%PREFSMAN->m_iNumBackgrounds );
					else
						sBGName = STATIC_BACKGROUND;
			}
			
			m_aBGChanges.push_back( change );
		}
	}
	else	// pSong doesn't have an animation plan
	{
		const float fFirstBeat = pSong->m_fFirstBeat;
		const float fLastBeat = pSong->m_fLastBeat;

	//	const int iSequenceLength = 8;
	//	const int iSequence[iSequenceLength] = {0,1,0,1,2,3,2,3};
		int ctr=0;

		// change BG every 4 bars
		for( float f=fFirstBeat; f<fLastBeat; f+=BEATS_PER_MEASURE*4 )
		{
			if( bLoadedAnyRandomBackgrounds )
			{
				CString sBGName = RandomBackground( ctr );
				
				// Don't fade.  It causes frame rate dip, especially on 
				// slower machines.
				bool bFade = false;
				//bool bFade = PREFSMAN->m_BackgroundMode==PrefsManager::BGMODE_RANDOMMOVIES || 
				//	PREFSMAN->m_BackgroundMode==PrefsManager::BGMODE_MOVIEVIS;
				
				m_aBGChanges.push_back( BackgroundChange(f,sBGName,1.f,bFade) );
				ctr = (ctr+1)%PREFSMAN->m_iNumBackgrounds;
			}
		}

		// change BG every BPM change that is at the beginning of a measure
		for( unsigned i=0; i<pSong->m_BPMSegments.size(); i++ )
		{
			const BPMSegment& bpmseg = pSong->m_BPMSegments[i];

			if( fmodf(bpmseg.m_fStartBeat, (float)BEATS_PER_MEASURE) != 0 )
				continue;	// skip

			if( bpmseg.m_fStartBeat < fFirstBeat  || bpmseg.m_fStartBeat > fLastBeat )
				continue;	// skip

			if( bLoadedAnyRandomBackgrounds )
			{
				CString sBGName = RandomBackground( rand()%PREFSMAN->m_iNumBackgrounds );
				m_aBGChanges.push_back( BackgroundChange(bpmseg.m_fStartBeat,sBGName) );
			}
		}
	}


	// end showing the static song background
	m_aBGChanges.push_back( BackgroundChange(pSong->m_fLastBeat,STATIC_BACKGROUND) );

		
	// sort segments
	SortBackgroundChangesArray( m_aBGChanges );

	// scale all rates by the current music rate
	for( unsigned i=0; i<m_aBGChanges.size(); i++ )
		m_aBGChanges[i].m_fRate *= GAMESTATE->m_SongOptions.m_fMusicRate;


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

	TEXTUREMAN->EnableOddDimensionWarning();
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

		if( m_aBGChanges.size() == 0 )
			return;

		/* Only update BGAnimations if we're not in the middle of a stop. */
		if( !GAMESTATE->m_bFreeze )
		{
			// Find the BGSegment we're in
			int i;
			int size = (int)(m_aBGChanges.size()) - 1;
			for( i=0; i<size; i++ )
				if( GAMESTATE->m_fSongBeat < m_aBGChanges[i+1].m_fStartBeat )
					break;

			if( i != m_iCurBGChangeIndex )
			{
				LOG->Trace( "old bga %d -> new bga %d, %f, %f", i, m_iCurBGChangeIndex, m_aBGChanges[i].m_fStartBeat, GAMESTATE->m_fSongBeat );

				m_iCurBGChangeIndex = i;

				const BackgroundChange& change = m_aBGChanges[i];

				BGAnimation* pOld = m_pCurrentBGA;

				if( change.m_bFadeLast )
					m_pFadingBGA = m_pCurrentBGA;
				else
					m_pFadingBGA = NULL;

				m_pCurrentBGA = m_BGAnimations[ change.m_sBGName ];

				if( pOld )
					pOld->LosingFocus();
				if( m_pCurrentBGA )
					m_pCurrentBGA->GainingFocus( change.m_fRate, change.m_bRewindMovie, change.m_bLoop );

				m_fSecsLeftInFade = m_pFadingBGA!=NULL ? FADE_SECONDS : 0;
			}

			if( m_pCurrentBGA )
				m_pCurrentBGA->Update( fDeltaTime );
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
	}
	
	if( m_pDancingCharacters )
		m_pDancingCharacters->Update( fDeltaTime );

	m_quadBGBrightness.Update( fDeltaTime );
}

void Background::DrawPrimitives()
{
	if( PREFSMAN->m_fBGBrightness == 0 )
		return;

	ActorFrame::DrawPrimitives();
	
	if( IsDangerVisible() )
	{
		m_BGADanger.Draw();
	}
	else
	{	
		if( m_pCurrentBGA )
			m_pCurrentBGA->Draw();
		if( m_pFadingBGA )
			m_pFadingBGA->Draw();
	}

	if( m_pDancingCharacters )
		m_pDancingCharacters->Draw();

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
	m_quadBGBrightness.SetDiffuse( RageColor(0,0,0,1-PREFSMAN->m_fBGBrightness) );
}

void Background::FadeOut()
{
	m_quadBGBrightness.BeginTweening( 0.5f );
	m_quadBGBrightness.SetDiffuse( RageColor(0,0,0,1-0.5f) );

}
