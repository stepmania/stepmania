#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ListDisplay

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ListDisplay.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "Course.h"
#include "SongManager.h"
#include <math.h>
#include "ThemeManager.h"
#include "Steps.h"
#include "GameState.h"
#include "StyleDef.h"
#include "RageTexture.h"


ListDisplay::ListDisplay()
{
	m_iNumItemsToShow = 0;
	m_fItemWidth = 0;
	m_fItemHeight = 0;
	m_fItemAtTopOfList = 0;

	m_quadMask.SetBlendMode( BLEND_NO_EFFECT );	// don't change color values
	m_quadMask.SetUseZBuffer( true );	// we want to write to the Zbuffer
}


void ListDisplay::Load( 
	const vector<Actor*> &m_vpItems, 
	int iNumItemsToShow, 
	float fItemWidth, 
	float fItemHeight, 
	bool bLoop, 
	float fSecondsPerItem, 
	float fSecondsPauseBetweenItems,
	bool bSlide )
{
	ASSERT( iNumItemsToShow > 0 );
	ASSERT( fItemWidth > 0 );
	ASSERT( fItemHeight > 0 );
	ASSERT( fSecondsPerItem > 0 );
	ASSERT( fSecondsPauseBetweenItems >= 0 );

	m_SubActors = m_vpItems;
	m_iNumItemsToShow = iNumItemsToShow;
	m_fItemWidth = fItemWidth;
	m_fItemHeight = fItemHeight;
	m_bLoop = bLoop; 
	m_fSecondsPerItem = fSecondsPerItem; 
	m_fSecondsPauseBetweenItems = fSecondsPauseBetweenItems;
	m_bSlide = bSlide;
	m_fItemAtTopOfList = m_bLoop ? 0 : (float)-m_iNumItemsToShow;
	m_fSecondsPauseCountdown = 0;

	RectF rectBarSize(-m_fItemWidth/2, -m_fItemHeight/2,
						m_fItemWidth/2, m_fItemHeight/2);
	m_quadMask.StretchTo( rectBarSize );
	m_quadMask.SetZ( 1 );
}

float ListDisplay::GetSecondsForCompleteScrollThrough()
{
	int fTotalItems = m_iNumItemsToShow + m_SubActors.size();
	return fTotalItems * (m_fSecondsPerItem + m_fSecondsPauseBetweenItems );
}

void ListDisplay::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	/* If we have no children, the code below will busy loop. */
	if( !m_SubActors.size() )
		return;

	while( fDeltaTime > 0 )
	{
		// handle pause
		if( fDeltaTime > m_fSecondsPauseCountdown )
		{
			fDeltaTime -= m_fSecondsPauseCountdown;
			m_fSecondsPauseCountdown = 0;
		}
		else
		{
			m_fSecondsPauseCountdown -= fDeltaTime;
			fDeltaTime = 0;
			break;
		}

		// handle scrolling
		float fPercentUntilNextItem = (int)m_fItemAtTopOfList+1 - m_fItemAtTopOfList;
		float fSecsUntilNextItem = fPercentUntilNextItem * m_fSecondsPerItem;

		if( fDeltaTime > fSecsUntilNextItem )
		{
			fDeltaTime -= fSecsUntilNextItem;
			m_fItemAtTopOfList += fPercentUntilNextItem;
		}
		else
		{
			m_fItemAtTopOfList += fDeltaTime / m_fSecondsPerItem;
			fDeltaTime = 0;
			break;
		}

		if( m_bLoop )
			m_fItemAtTopOfList = fmodf( m_fItemAtTopOfList, (float) m_SubActors.size() );
	}
}

void ListDisplay::DrawPrimitives()
{
	if( m_SubActors.empty() )
		return;

	// write to z buffer so that top and bottom are clipped
	float fIndexFullyOnScreenTop = -(m_iNumItemsToShow-1)/2.f;
	float fIndexFullyOnScreenBottom = (m_iNumItemsToShow-1)/2.f;
	float fIndexCompletelyOffScreenTop = fIndexFullyOnScreenTop - 1;
	float fIndexCompletelyOffScreenBottom = fIndexFullyOnScreenBottom + 1;

	m_quadMask.SetY( fIndexCompletelyOffScreenTop * m_fItemHeight );
	m_quadMask.Draw();
	m_quadMask.SetY( fIndexCompletelyOffScreenBottom * m_fItemHeight );
	m_quadMask.Draw();

	int iItemToDraw = (int)m_fItemAtTopOfList;
	float fRemainder = m_fItemAtTopOfList - (int)m_fItemAtTopOfList;
	float fIndex = fIndexFullyOnScreenTop - fRemainder;
	float fY = (fIndex)*m_fItemHeight;

	for( int i=0; i<m_iNumItemsToShow+1; i++ )
	{
		if( iItemToDraw >= 0 && iItemToDraw < (int) m_SubActors.size() )
		{			
			float fX = 0;
			if( m_bSlide && fIndex>fIndexFullyOnScreenBottom-1 )
				fX = SCALE( fIndex, fIndexFullyOnScreenBottom, fIndexFullyOnScreenBottom-1.f, 640.f, 0.f );
			m_SubActors[iItemToDraw]->SetXY( roundf(fX), roundf(fY) );
			m_SubActors[iItemToDraw]->Draw();
		}
		
		iItemToDraw++;
		if( m_bLoop )
			wrap( iItemToDraw, m_SubActors.size() );
		fY += m_fItemHeight;
		fIndex += 1;
	}
}

