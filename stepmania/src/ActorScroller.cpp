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
	m_fSecsIntoScroll = 0;
}

void ActorScroller::Load( vector<Actor*> vActors, float fBaseX, float fBaseY, float fVelX, float fVelY, float fSpacingX, float fSpacingY )
{
	m_vActors = vActors;
	m_vBase.x = fBaseX;
	m_vBase.y = fBaseY;
	m_vVelocity.x = fVelX;
	m_vVelocity.y = fVelY;
	m_vSpacing.x = fSpacingX;
	m_vSpacing.y = fSpacingY;

	// adjust base
	float t = 10000;	// inf
	if( m_vSpacing.x != 0 )
		t = min( t, m_vBase.x / fabsf(m_vSpacing.x) );
	if( m_vSpacing.y != 0 )
		t = min( t, m_vBase.y / fabsf(m_vSpacing.y) );
	m_vBase.x -= t * fabsf(m_vSpacing.x);
	m_vBase.y -= t * fabsf(m_vSpacing.y);
	if( m_vVelocity.x < 0 )
		m_vBase.x += SCREEN_WIDTH;
	if( m_vVelocity.y < 0 )
		m_vBase.y += SCREEN_HEIGHT;
	
	// push off screen
	if( m_vVelocity.x!=0 )	m_vBase.x += m_vSpacing.x/2;
	if( m_vVelocity.y!=0 )	m_vBase.y += m_vSpacing.y/2;
}

float ActorScroller::GetTotalSecsToScroll()
{
	float fMaxSecToScroll = 0;
	if( m_vVelocity.x != 0 )
	{
		float fSecToScroll = fabsf(m_vSpacing.x*(m_vActors.size()+1) + SCREEN_WIDTH) / fabsf(m_vVelocity.x);
		fMaxSecToScroll = max( fMaxSecToScroll, fSecToScroll );
	}
	if( m_vVelocity.y != 0 )
	{
		float fSecToScroll = fabsf(m_vSpacing.y*(m_vActors.size()+1) + SCREEN_HEIGHT) / fabsf(m_vVelocity.y);
		fMaxSecToScroll = max( fMaxSecToScroll, fSecToScroll );
	}
	return fMaxSecToScroll;
}

void ActorScroller::Update( float fDeltaTime )
{
	m_fSecsIntoScroll += fDeltaTime;

	for( unsigned i=0; i<m_vActors.size(); i++ )
	{
		float fX = m_vBase.x + m_vSpacing.x * i + m_vVelocity.x * m_fSecsIntoScroll;
		float fY = m_vBase.y + m_vSpacing.y * i + m_vVelocity.y * m_fSecsIntoScroll;
		fX = roundf( fX );
		fY = roundf( fY );
		m_vActors[i]->SetXY( fX, fY );
	}
}

void ActorScroller::DrawPrimitives()
{
	for( unsigned i=0; i<m_vActors.size(); i++ )
		if( !IsOffScreen(m_vActors[i]) )
			m_vActors[i]->Draw();
}