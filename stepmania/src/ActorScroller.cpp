#include "global.h"	// testing updates
/*
-----------------------------------------------------------------------------
 Class: ActorScroller

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorScroller.h"
#include "ActorCollision.h"
#include <math.h>
#include "RageUtil.h"


ActorScroller::ActorScroller()
{
	m_fCurrentItem = 0;
	m_fDestinationItem = 0;
}

void ActorScroller::Load( float fScrollSecondsPerItem, float fSpacingX, float fSpacingY )
{
	ASSERT( fScrollSecondsPerItem > 0 );
	m_fScrollSecondsPerItem = fScrollSecondsPerItem;
	m_vSpacing.x = fSpacingX;
	m_vSpacing.y = fSpacingY;
}

void ActorScroller::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	fapproach( m_fCurrentItem, m_fDestinationItem, fDeltaTime/m_fScrollSecondsPerItem );

	for( unsigned i=0; i<m_SubActors.size(); i++ )
	{
		Actor* pActor = m_SubActors[i];

		float fItemOffset = i - m_fCurrentItem;
		float fX = m_vSpacing.x * fItemOffset;
		float fY = m_vSpacing.y * fItemOffset;
		fX = roundf( fX );
		fY = roundf( fY );
		pActor->SetXY( fX, fY );
	}
}
