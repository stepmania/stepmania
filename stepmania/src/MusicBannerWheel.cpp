#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: MusicBannerWheel

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Andrew Livy
-----------------------------------------------------------------------------
*/

#include "MusicBannerWheel.h"
#include "RageUtil.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "SongManager.h"
#include "ThemeManager.h"
#include "RageMusic.h"

#define MAXBANNERS 5
#define BANNERSPACING 200


MusicBannerWheel::MusicBannerWheel() 
{ 
	currentPos=0;

	m_ScrollingList.SetXY( 0, 0 );
	m_ScrollingList.SetSpacing( BANNERSPACING );
	this->AddChild( &m_ScrollingList );

	SetNewPos(currentPos);
	PlayMusicSample();
}

void MusicBannerWheel::SetNewPos(int NewPos)
{
	CArray<Song*, Song*> arraySongs = SONGMAN->m_pSongs;
	Song* pSong;

	// TODO: OPTIMIZE THIS. (SO IT LOADS THE ENTIRE SONG LIST)
	CStringArray asGraphicPaths;
	for( unsigned j=0; j<arraySongs.size() && j < MAXBANNERS; j++ )
	{
		pSong = arraySongs[j];

		if( pSong == NULL ) asGraphicPaths.push_back(THEME->GetPathTo("Graphics","fallback banner"));
		else if (pSong->HasBanner()) asGraphicPaths.push_back(pSong->GetBannerPath());
		else if (PREFSMAN->m_bUseBGIfNoBanner && pSong->HasBackground() ) asGraphicPaths.push_back(pSong->GetBannerPath());
		else asGraphicPaths.push_back(THEME->GetPathTo("Graphics","fallback banner"));
	}	
	m_ScrollingList.Load( asGraphicPaths );
}

Song* MusicBannerWheel::GetSelectedSong()
{
	CArray<Song*, Song*> arraySongs = SONGMAN->m_pSongs;
	Song* pSong;
	pSong=arraySongs[m_ScrollingList.GetSelection()];
	return(pSong);
}

void MusicBannerWheel::PlayMusicSample()
{
	Song* pSong = GetSelectedSong();
	if( pSong  &&  pSong->HasMusic() )
	{
		MUSIC->Stop();
		MUSIC->Load( pSong->GetMusicPath() );
		MUSIC->Play( true, pSong->m_fMusicSampleStartSeconds, pSong->m_fMusicSampleLengthSeconds );
	}
}

void MusicBannerWheel::BannersLeft()
{
	m_ScrollingList.Left();
	PlayMusicSample();
}

void MusicBannerWheel::BannersRight()
{
	m_ScrollingList.Right();
	PlayMusicSample();
}

MusicBannerWheel::~MusicBannerWheel()
{
}
