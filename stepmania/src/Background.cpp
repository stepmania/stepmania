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
#include "PrefsManager.h"
#include "PrefsManager.h"
#include "RageBitmapTexture.h"
#include "RageException.h"
#include "RageTimer.h"


const CString BG_ANIMS_DIR = "BGAnimations\\";
const CString VISUALIZATIONS_DIR = "Visualizations\\";


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
	m_fSongBeat = 0;
	m_bFreeze = false;

	m_bInDanger = false;

	m_BackgroundMode = MODE_STATIC_BG;

	m_sprDanger.SetZoom( 2 );
	m_sprDanger.SetEffectWagging();
	m_sprDanger.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_DANGER_TEXT) );
	m_sprDanger.SetXY( CENTER_X, CENTER_Y );

	m_sprDangerBackground.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_DANGER_BACKGROUND) );
	m_sprDangerBackground.StretchTo( CRect(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );

	m_quadBGBrightness.StretchTo( CRect(SCREEN_LEFT, SCREEN_TOP, SCREEN_RIGHT, SCREEN_BOTTOM) );
	m_quadBGBrightness.SetDiffuseColor( D3DXCOLOR(0,0,0,1-0.5f) );
}

Background::~Background()
{
    for( int i=0; i<m_BackgroundAnimations.GetSize(); i++ )
		delete m_BackgroundAnimations[i];
}

bool Background::LoadFromSong( Song* pSong, bool bDisableVisualizations )
{
	m_fMusicSeconds = 0;
	m_bStartedBGMovie = false;


	m_pSong = pSong;

	//
	// figure out what BackgroundMode to use
	//
	if( PREFSMAN->m_BackgroundMode == PrefsManager::BGMODE_OFF )
		m_BackgroundMode = MODE_STATIC_BG;
	else if( pSong->HasMovieBackground() )
		m_BackgroundMode = MODE_MOVIE_BG;
	else if( PREFSMAN->m_BackgroundMode == PrefsManager::BGMODE_MOVIEVIS )
		m_BackgroundMode = MODE_MOVIE_VIS;
	else
		m_BackgroundMode = MODE_ANIMATIONS;

	m_sprSongBackground.Load( pSong->HasBackground() ? pSong->GetBackgroundPath() : THEME->GetPathTo(GRAPHIC_FALLBACK_BACKGROUND), true, 4, 0, true );
	
	m_sprSongBackground.StretchTo( CRect(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );

	switch( m_BackgroundMode )
	{
	case MODE_STATIC_BG:
		// do nothing
		break;
	case MODE_MOVIE_BG:
		m_sprMovieBackground.Load( pSong->GetMovieBackgroundPath() );
		m_sprMovieBackground.StretchTo( CRect(SCREEN_LEFT+10,SCREEN_TOP+16,SCREEN_RIGHT-10,SCREEN_BOTTOM-16) );
		m_sprMovieBackground.SetZoomY( m_sprMovieBackground.GetZoomY()*-1 );
		m_sprMovieBackground.StopAnimating( );
		break;
	case MODE_MOVIE_VIS:
		{
			CStringArray arrayPossibleMovies;
			GetDirListing( VISUALIZATIONS_DIR + CString("*.avi"), arrayPossibleMovies );
			GetDirListing( VISUALIZATIONS_DIR + CString("*.mpg"), arrayPossibleMovies );
			GetDirListing( VISUALIZATIONS_DIR + CString("*.mpeg"), arrayPossibleMovies );
			if( arrayPossibleMovies.GetSize() > 0 )
			{
				int index = rand() % arrayPossibleMovies.GetSize();
				m_sprMovieVis.Load( VISUALIZATIONS_DIR + arrayPossibleMovies[index] );
				m_sprMovieVis.StretchTo( CRect(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );
				m_sprMovieVis.SetZoomY( m_sprMovieVis.GetZoomY()*-1 );
				m_sprMovieVis.SetBlendModeAdd();
			}		
		}
		break;
	case MODE_ANIMATIONS:
		{
			//
			// Load background animations
			//
			CStringArray asBGAnimNames;

			// First look in the song folder for animations
			GetDirListing( pSong->m_sSongDir+"BGAnimations\\*.*", asBGAnimNames, true );
			
			if( asBGAnimNames.GetSize() > 0 )
			{
				for( int i=0; i<asBGAnimNames.GetSize(); i++ )
					m_BackgroundAnimations.Add( new BackgroundAnimation(pSong->m_sSongDir+"BGAnimations\\"+asBGAnimNames[i], pSong) );

			}
			else
			{
				// We're going to try to classify songs as trance, pop, or techno based on some data about the song
				if( pSong->m_BPMSegments.GetSize() + pSong->m_StopSegments.GetSize() >= 3 )
					GetDirListing( BG_ANIMS_DIR+"techno*.*", asBGAnimNames, true );
				else if( pSong->m_BPMSegments[0].m_fBPM > 160 )
					GetDirListing( BG_ANIMS_DIR+"trance*.*", asBGAnimNames, true );
				else
					GetDirListing( BG_ANIMS_DIR+"pop*.*", asBGAnimNames, true );
				
				// load different animations 
				for( int i=0; i<asBGAnimNames.GetSize(); i++ )
					m_BackgroundAnimations.Add( new BackgroundAnimation(BG_ANIMS_DIR + asBGAnimNames[i], pSong) );
			}

			if( m_BackgroundAnimations.GetSize() == 0 ) 
				break;

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
			}
			else
			{
				// Generate a plan.
				for( int i=0; i<pSong->m_fLastBeat; i+=16 )
					m_aAnimSegs.Add( AnimSeg((float)i,rand()%m_BackgroundAnimations.GetSize()) );	// change BG every 4 bars

				for( i=0; i<pSong->m_BPMSegments.GetSize(); i++ )
				{
					const BPMSegment& bpmseg = pSong->m_BPMSegments[i];
					m_aAnimSegs.Add( AnimSeg(bpmseg.m_fStartBeat,int(bpmseg.m_fBPM)%m_BackgroundAnimations.GetSize()) );	// change BG every BPM change
				}
				SortAnimSegArray( m_aAnimSegs );
			}
		}
		break;
	default:
		ASSERT(0);
	}
	

	return true;
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
		if( m_bFreeze )
			return;

		m_sprSongBackground.Update( fDeltaTime );

		switch( m_BackgroundMode )
		{
		case MODE_STATIC_BG:
			// do nothing
			break;
		case MODE_MOVIE_BG:
			m_sprMovieBackground.Update( fDeltaTime );
			break;
		case MODE_MOVIE_VIS:
			m_sprMovieVis.Update( fDeltaTime );
			break;
		case MODE_ANIMATIONS:
			if( GetCurBGA() )
				GetCurBGA()->Update( fDeltaTime, m_fSongBeat );
			break;
		default:
			ASSERT(0);
		}

		m_quadBGBrightness.Update( fDeltaTime );
	}
}

void Background::SetSongBeat( const float fSongBeat, const bool bFreeze, const float fMusicSeconds )
{
	m_fSongBeat = fSongBeat;
	m_bFreeze = bFreeze;
	m_fMusicSeconds = fMusicSeconds;
	if( m_BackgroundMode == MODE_MOVIE_BG  &&  !m_bStartedBGMovie  &&  m_fMusicSeconds > 0 )
		m_sprMovieBackground.StartAnimating();
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

		switch( m_BackgroundMode )
		{
		case MODE_STATIC_BG:
			m_sprSongBackground.Draw();
			break;
		case MODE_MOVIE_BG:
			::Sleep(4);	// let the movie decode a frame
			m_sprMovieBackground.Draw();
			break;
		case MODE_MOVIE_VIS:
			m_sprSongBackground.Draw();
			::Sleep(4);	// let the movie decode a frame
			m_sprMovieVis.Draw();
			break;
		case MODE_ANIMATIONS:
			if( GetCurBGA() )
				GetCurBGA()->Draw();
			else
				m_sprSongBackground.Draw();
			break;
		default:
			ASSERT(0);
		}

		m_quadBGBrightness.Draw();
	}
}

bool Background::DangerVisible()
{
	return m_bInDanger && (TIMER->GetTimeSinceStart() - (int)TIMER->GetTimeSinceStart()) < 0.5f;
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
