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

void FadingBanner::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );
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
	m_Banner[m_iIndexFront].BeginTweening( 0.25f );		// fade out
	m_Banner[m_iIndexFront].SetDiffuse( RageColor(1,1,1,0) );
}

void FadingBanner::LoadFromSong( Song* pSong )
{
	BeforeChange();
	m_Banner[GetBackIndex()].LoadFromSong( pSong );
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
