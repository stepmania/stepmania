#include "stdafx.h"
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

void FadingBanner::SetCroppedSize( float fWidth, float fHeight )
{
	m_BackBanner.SetCroppedSize( fWidth, fHeight );
	Banner::SetCroppedSize( fWidth, fHeight );
}

void FadingBanner::Update( float fDeltaTime )
{
	m_BackBanner.Update(fDeltaTime);
	Banner::Update( fDeltaTime );
}

void FadingBanner::DrawPrimitives()
{
	Banner::DrawPrimitives();
	m_BackBanner.Draw();
}

bool FadingBanner::Load( RageTextureID ID )
{
	BeforeChange();
	return Banner::Load(ID);
}

void FadingBanner::BeforeChange()
{
	// move the back banner to the front in preparation for a cross fade
	if( this->GetTexturePath() != "" )
	{
		m_BackBanner.Load( this->GetTexturePath() );
		m_BackBanner.SetScrolling( this->IsScrolling(), this->ScrollingPercent() );
	}

	m_BackBanner.SetDiffuse( RageColor(1,1,1,1) );
	m_BackBanner.StopTweening();
	m_BackBanner.BeginTweening( 0.25f );		// fade out
	m_BackBanner.SetTweenDiffuse( RageColor(1,1,1,0) );
}

