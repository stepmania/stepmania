#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: FadingBanner

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "FadingBanner.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "GameState.h"


FadingBanner::FadingBanner()
{
	// these guys get loaded on the Set* methods
	this->AddChild( &m_Banner[0] );
	this->AddChild( &m_Banner[1] );
}

void FadingBanner::SetCroppedSize( float fWidth, float fHeight )
{
	m_Banner[0].SetCroppedSize( fWidth, fHeight );
	m_Banner[1].SetCroppedSize( fWidth, fHeight );
}

void FadingBanner::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

}

void FadingBanner::BeforeChange()
{
	// move the back banner to the front in preparation for a cross fade
	if( m_Banner[0].GetTexturePath() != "" )
	{
		m_Banner[1].Load( m_Banner[0].GetTexturePath() );
		m_Banner[1].SetScrolling( m_Banner[0].IsScrolling(), m_Banner[0].ScrollingPercent() );
	}

	m_Banner[1].SetDiffuse( RageColor(1,1,1,1) );
	m_Banner[1].StopTweening();
	m_Banner[1].BeginTweening( 0.25f );		// fade out
	m_Banner[1].SetTweenDiffuse( RageColor(1,1,1,0) );
}

void FadingBanner::SetFromSong( Song* pSong )
{
	ASSERT( pSong != NULL );
	BeforeChange();
	m_Banner[0].LoadFromSong( pSong );
}

void FadingBanner::SetFromGroup( const CString &sGroupName )
{
	BeforeChange();
	m_Banner[0].LoadFromGroup( sGroupName );
}

void FadingBanner::SetRoulette()
{
	BeforeChange();
	m_Banner[0].LoadRoulette();
}

void FadingBanner::SetFromCourse( Course* pCourse )
{
	BeforeChange();
	m_Banner[0].LoadFromCourse( pCourse );
}
