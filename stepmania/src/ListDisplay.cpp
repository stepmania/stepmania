#include "global.h"
#include "ListDisplay.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "Course.h"
#include "SongManager.h"
#include "ThemeManager.h"
#include "Steps.h"
#include "GameState.h"
#include "Style.h"
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
	CLAMP( iNumItemsToShow, 1, 10000 );
	CLAMP( fItemWidth, 1, 10000 );
	CLAMP( fItemHeight, 1, 10000 );
	CLAMP( fSecondsPerItem, 0.01f, 10000 );
	CLAMP( fSecondsPauseBetweenItems, 0, 10000 );

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

/*
 * (c) 2003 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
