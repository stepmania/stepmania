#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: FadingBanner

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "FadingBanner.h"
#include "RageTextureManager.h"
#include "BannerCache.h"
#include "song.h"
#include "RageLog.h"

/* XXX: metric */
static const float FadeTime = 0.25;

FadingBanner::FadingBanner()
{
	m_iIndexFront = 0;
	for( int i=0; i<2; i++ )
		this->AddChild( &m_Banner[i] );
}

void FadingBanner::SetCroppedSize( float fWidth, float fHeight )
{
	for( int i=0; i<2; i++ )
		m_Banner[i].SetCroppedSize( fWidth, fHeight );
}

#include "PrefsManager.h"
#include "ThemeManager.h"
void FadingBanner::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	/* Don't fade to the full banner until we finish fading. */
	float HighQualTime = FadeTime;

	/* Hacky: also don't fade until the music wheel has a chance to settle down. */
	HighQualTime = max( HighQualTime, 1.0f / PREFSMAN->m_iMusicWheelSwitchSpeed );
	HighQualTime = max( HighQualTime, THEME->GetMetricF("MusicWheel","SwitchSeconds") );
	
	if( m_sPendingBanner == "" || m_PendingTimer.PeekDeltaTime() < HighQualTime )
		return;

	/* Load the high quality banner. */
	CString banner = m_sPendingBanner;
	BeforeChange();
	m_Banner[GetBackIndex()].Load( banner );
}

void FadingBanner::DrawPrimitives()
{
	// draw manually
//	ActorFrame::DrawPrimitives();
	m_Banner[GetBackIndex()].Draw();
	m_Banner[m_iIndexFront].Draw();
}

bool FadingBanner::Load( RageTextureID ID )
{
	BeforeChange();
	return m_Banner[m_iIndexFront].Load(ID);
}

void FadingBanner::BeforeChange()
{
	m_Banner[m_iIndexFront].SetDiffuse( RageColor(1,1,1,1) );

	m_iIndexFront = GetBackIndex();

	m_Banner[m_iIndexFront].SetDiffuse( RageColor(1,1,1,1) );
	m_Banner[m_iIndexFront].StopTweening();
	m_Banner[m_iIndexFront].BeginTweening( FadeTime );		// fade out
	m_Banner[m_iIndexFront].SetDiffuse( RageColor(1,1,1,0) );

	m_sPendingBanner = "";
}

void FadingBanner::LoadFromSong( Song* pSong )
{
	BeforeChange();

	if( !pSong->HasBanner() || 
		TEXTUREMAN->IsTextureRegistered( Banner::BannerTex( pSong->GetBannerPath() ) ) )
	{
		/* We either don't have a regular banner at all, or we have a banner and
		 * it's cached; just use Banner's logic. */
		m_Banner[GetBackIndex()].LoadFromSong( pSong );
		return;
	}

	/* We have a banner, but it's not loaded.  Try to load the low quality version. */
	RageTextureID ID = BANNERCACHE->LoadCachedSongBanner( pSong->GetBannerPath() );
	
	if( !TEXTUREMAN->IsTextureRegistered(ID) )
	{
		LOG->Warn("Load of reduced banner for '%s' failed", pSong->GetSongDir().c_str() );
		m_Banner[GetBackIndex()].LoadFromSong( pSong );
		return;
	}

	m_Banner[GetBackIndex()].Load( ID );
	m_sPendingBanner = pSong->GetBannerPath();
	m_PendingTimer.GetDeltaTime(); /* reset */
}

void FadingBanner::LoadAllMusic()
{
	BeforeChange();
	m_Banner[GetBackIndex()].LoadAllMusic();
}

void FadingBanner::LoadFromGroup( CString sGroupName )
{
	BeforeChange();
	m_Banner[GetBackIndex()].LoadFromGroup( sGroupName );
}

void FadingBanner::LoadFromCourse( Course* pCourse )
{
	BeforeChange();
	m_Banner[GetBackIndex()].LoadFromCourse( pCourse );
}

void FadingBanner::LoadRoulette()
{
	BeforeChange();
	m_Banner[GetBackIndex()].LoadRoulette();
}

void FadingBanner::LoadRandom()
{
	BeforeChange();
	m_Banner[GetBackIndex()].LoadRandom();
}

void FadingBanner::LoadFallback()
{
	BeforeChange();
	m_Banner[GetBackIndex()].LoadFallback();
}
