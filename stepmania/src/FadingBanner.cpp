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

void FadingBanner::SetCroppedSize( float fWidth, float fHeight )
{
	m_FrontBanner.SetCroppedSize( fWidth, fHeight );
	Banner::SetCroppedSize( fWidth, fHeight );
}

void FadingBanner::Update( float fDeltaTime )
{
	m_FrontBanner.Update(fDeltaTime);
	Banner::Update( fDeltaTime );
}

void FadingBanner::DrawPrimitives()
{
	Banner::DrawPrimitives();
	m_FrontBanner.Draw();
}

bool FadingBanner::Load( RageTextureID ID )
{
	BeforeChange();
	
	if(!Banner::Load(ID))
		return false;

	Update(0);
	return true;
}

void FadingBanner::BeforeChange()
{
	// move the back banner to the front in preparation for a cross fade
	if( this->GetTexture() )
	{
		m_FrontBanner.Load( this->GetTexture()->GetID() );
		m_FrontBanner.SetScrolling( this->IsScrolling(), this->ScrollingPercent() );
	}

	m_FrontBanner.SetDiffuse( RageColor(1,1,1,1) );
	m_FrontBanner.StopTweening();
	m_FrontBanner.BeginTweening( 0.25f );		// fade out
	m_FrontBanner.SetTweenDiffuse( RageColor(1,1,1,0) );
}

