#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: BannerWithFrame

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "BannerWithFrame.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"



BannerWithFrame::BannerWithFrame()
{
	m_sprBannerFrame.Load( THEME->GetPathTo("Graphics","evaluation banner frame") );
	m_Banner.SetCroppedSize( m_sprBannerFrame.GetUnzoomedWidth()-6, m_sprBannerFrame.GetUnzoomedHeight()-6 );

	this->AddChild( &m_Banner );
	this->AddChild( &m_sprBannerFrame );
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		float fX = (m_sprBannerFrame.GetUnzoomedWidth()/2-26) * (p==PLAYER_1 ? -1 : 1 );
		float fY = m_sprBannerFrame.GetUnzoomedHeight()/2-26;
		m_Icon[p].SetX( fX );
		m_Icon[p].SetY( fY );
		this->AddChild( &m_Icon[p] );
	}
}

void BannerWithFrame::LoadFromSongAndNotes( Song* pSong, Notes* pNotes[NUM_PLAYERS] )
{
	LoadFromSong( pSong );

	for( int p=0; p<NUM_PLAYERS; p++ )
		m_Icon[p].SetFromNotes( pNotes[p] );	// NULL pNotes menas icon is hidden
}

void BannerWithFrame::LoadFromSong( Song* pSong )
{
	m_Banner.LoadFromSong( pSong );
}

void BannerWithFrame::LoadFromGroup( CString sGroupName )
{
	m_Banner.LoadFromGroup( sGroupName );
}

void BannerWithFrame::LoadFromCourse( Course* pCourse )
{
	m_Banner.LoadFromCourse( pCourse );
}
