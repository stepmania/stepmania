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
#include "RageDisplay.h"


ActorScroller::ActorScroller()
{
	m_bLoaded = false;
	m_fCurrentItem = 0;
	m_fDestinationItem = 0;
	m_fSecondsPerItem = 1;
	m_iNumItemsToDraw = 7;

	m_vRotationDegrees = RageVector3(0,0,0);
	m_vTranslateTerm0 = RageVector3(0,0,0);
	m_vTranslateTerm1 = RageVector3(0,0,0);
	m_vTranslateTerm2 = RageVector3(0,0,0);
}

void ActorScroller::Load( 
	float fSecondsPerItem, 
	int iNumItemsToDraw, 
	const RageVector3	&vRotationDegrees,
	const RageVector3	&vTranslateTerm0,
	const RageVector3	&vTranslateTerm1,
	const RageVector3	&vTranslateTerm2
	)
{
	ASSERT( fSecondsPerItem > 0 );
	m_fSecondsPerItem = fSecondsPerItem;
	m_iNumItemsToDraw = iNumItemsToDraw;
	m_vRotationDegrees = vRotationDegrees;
	m_vTranslateTerm0 = vTranslateTerm0;
	m_vTranslateTerm1 = vTranslateTerm1;
	m_vTranslateTerm2 = vTranslateTerm2;

	m_bLoaded = true;
}

void ActorScroller::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	if( m_fHibernateSecondsLeft > 0 )
		return;	// early abort

	fapproach( m_fCurrentItem, m_fDestinationItem, fDeltaTime/m_fSecondsPerItem );
}

void ActorScroller::DrawPrimitives()
{
	// Optimization:  If we weren't loaded, then fall back to the ActorFrame logic
	if( !m_bLoaded )
	{
		ActorFrame::DrawPrimitives();
	}
	else
	{
		int iFirstItemToDraw = (int)roundf( m_fCurrentItem - m_iNumItemsToDraw/2.f );
		for( int i=iFirstItemToDraw; i<iFirstItemToDraw+m_iNumItemsToDraw; i++ )
		{
			if( i < 0  ||  i >= (int)m_SubActors.size() )
				continue;	// skip

			float fItemOffset = i - m_fCurrentItem;

			DISPLAY->PushMatrix();

			if( m_vRotationDegrees[0] )
				DISPLAY->RotateX( m_vRotationDegrees[0]*fItemOffset );
			if( m_vRotationDegrees[1] )
				DISPLAY->RotateY( m_vRotationDegrees[1]*fItemOffset );
			if( m_vRotationDegrees[2] )
				DISPLAY->RotateZ( m_vRotationDegrees[2]*fItemOffset );
			
			RageVector3 vTranslation = 
				m_vTranslateTerm0 +								// m_vTranslateTerm0*itemOffset^0
				m_vTranslateTerm1 * fItemOffset +				// m_vTranslateTerm1*itemOffset^1
				m_vTranslateTerm2 * fItemOffset*fItemOffset;	// m_vTranslateTerm2*itemOffset^2
			DISPLAY->Translate( 
				vTranslation[0],
				vTranslation[1],
				vTranslation[2]
				);

			m_SubActors[i]->Draw();
			
			DISPLAY->PopMatrix();
		}
	}
}
