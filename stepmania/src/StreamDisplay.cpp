#include "global.h"
#include "StreamDisplay.h"
#include "GameState.h"


void StreamDisplay::Load( 
	float fMeterWidth, 
	float fMeterHeight,
	int iNumStrips,
	int iNumChambers, 
	const CString &sNormalPath, 
	const CString &sHotPath, 
	const CString &sPassingPath
	)
{
	m_fMeterWidth = fMeterWidth;
	m_fMeterHeight = fMeterHeight;
	m_iNumStrips = iNumStrips;
	m_iNumChambers = iNumChambers;

	m_quadMask.SetDiffuse( RageColor(0,0,0,1) );
	m_quadMask.SetZ( 1 );
	m_quadMask.SetBlendMode( BLEND_NO_EFFECT );
	m_quadMask.SetUseZBuffer( true );

	CString sGraphicPath;
	RageTextureID ID;
	ID.bStretch = true;

	ID.filename = sNormalPath;
	m_sprStreamNormal.Load( ID );
	m_sprStreamNormal.SetUseZBuffer( true );

	ID.filename = sHotPath;
	m_sprStreamHot.Load( ID );
	m_sprStreamHot.SetUseZBuffer( true );
	
	ID.filename = sPassingPath;
	m_sprStreamPassing.Load( ID );
	m_sprStreamPassing.SetUseZBuffer( true );
}

void StreamDisplay::Update( float fDeltaSecs )
{
	// HACK:  Tweaking these values is very difficulty.  Update the
	// "physics" many times so that the spring motion appears faster
	for( int i=0; i<10; i++ )
	{
		const float fDelta = m_fPercent - m_fTrailingPercent;

		// Don't apply spring and viscous forces if we're full or empty.
		// Just move straight to either full or empty.
		if( m_fPercent <= 0 || m_fPercent >= 1 )
		{
			if( fabsf(fDelta) < 0.00001f )
				m_fVelocity = 0; // prevent div/0
			else
				m_fVelocity = (fDelta / fabsf(fDelta)) * 4;
		}
		else
		{
			const float fSpringForce = fDelta * 2.0f;
			m_fVelocity += fSpringForce * fDeltaSecs;

			const float fViscousForce = -m_fVelocity * 0.2f;
			m_fVelocity += fViscousForce * fDeltaSecs;
		}

		CLAMP( m_fVelocity, -.06f, +.02f );

		m_fTrailingPercent += m_fVelocity * fDeltaSecs;
	}

	// Don't clamp life percentage a little outside the visible range so
	// that the clamp doesn't dampen the "jiggle" of the meter.
	CLAMP( m_fTrailingPercent, -0.1f, 1.1f );
}

void StreamDisplay::DrawPrimitives()
{
	DrawMask( m_fTrailingPercent );		// this is the "right endcap" to the life
	
	const float fChamberWidthInPercent = 1.0f/m_iNumChambers;
	float fPercentBetweenStrips = 1.0f/m_iNumStrips;
	// round this so that the chamber overflows align
	if( m_iNumChambers > 10 )
		fPercentBetweenStrips = Quantize( fPercentBetweenStrips, fChamberWidthInPercent );

	float fPercentOffset = fmodf( GAMESTATE->m_fSongBeat/4+1000, fPercentBetweenStrips );
	ASSERT( fPercentOffset >= 0  &&  fPercentOffset <= fPercentBetweenStrips );

	for( float f=fPercentOffset+1; f>=0; f-=fPercentBetweenStrips )
	{
		DrawMask( f );
		DrawStrip( f );
	}
}

void StreamDisplay::SetPercent( float fPercent )
{
	float fDeltaPercent = fPercent - m_fPercent;
	m_fVelocity += fDeltaPercent;
	m_fPercent = fPercent;
}

void StreamDisplay::SetPassingAlpha( float fPassingAlpha )
{
	m_fPassingAlpha = fPassingAlpha;
}

void StreamDisplay::SetHotAlpha( float fHotAlpha )
{
	m_fHotAlpha = fHotAlpha;
}

void StreamDisplay::GetChamberIndexAndOverslow( float fPercent, int& iChamberOut, float& fChamberOverflowPercentOut )
{
	iChamberOut = (int)(fPercent*m_iNumChambers);
	fChamberOverflowPercentOut = fPercent*m_iNumChambers - iChamberOut;
}

float StreamDisplay::GetChamberLeftPercent( int iChamber )
{
	return (iChamber+0) / (float)m_iNumChambers;
}

float StreamDisplay::GetChamberRightPercent( int iChamber )
{
	return (iChamber+1) / (float)m_iNumChambers;
}

float StreamDisplay::GetRightEdgePercent( int iChamber, float fChamberOverflowPercent )
{
	if( (iChamber%2) == 0 )
		return (iChamber+fChamberOverflowPercent) / (float)m_iNumChambers;
	else
		return (iChamber+1) / (float)m_iNumChambers;
}

float StreamDisplay::GetHeightPercent( int iChamber, float fChamberOverflowPercent )
{
	if( (iChamber%2) == 1 )
		return 1-fChamberOverflowPercent;
	else
		return 0;
}

void StreamDisplay::DrawStrip( float fRightEdgePercent )
{
	RectF rect;

	const float fChamberWidthInPercent = 1.0f/m_iNumChambers;
	const float fStripWidthInPercent = 1.0f/m_iNumStrips;
	
	const float fCorrectedRightEdgePercent = fRightEdgePercent + fChamberWidthInPercent;
	const float fCorrectedStripWidthInPercent = fStripWidthInPercent + 2*fChamberWidthInPercent;
	const float fCorrectedLeftEdgePercent = fCorrectedRightEdgePercent - fCorrectedStripWidthInPercent;


	// set size of streams
	rect.left	= -m_fMeterWidth/2 + m_fMeterWidth*max(0,fCorrectedLeftEdgePercent);
	rect.top	= -m_fMeterHeight/2;
	rect.right	= -m_fMeterWidth/2 + m_fMeterWidth*min(1,fCorrectedRightEdgePercent);
	rect.bottom	= +m_fMeterHeight/2;

	ASSERT( rect.left <= m_fMeterWidth/2  &&  rect.right <= m_fMeterWidth/2 );  

	float fPercentCroppedFromLeft = max( 0, -fCorrectedLeftEdgePercent );
	float fPercentCroppedFromRight = max( 0, fCorrectedRightEdgePercent-1 );


	m_sprStreamNormal.StretchTo( rect );
	m_sprStreamPassing.StretchTo( rect );
	m_sprStreamHot.StretchTo( rect );


	// set custom texture coords
//		float fPrecentOffset = fRightEdgePercent;

	RectF frectCustomTexRect(
		fPercentCroppedFromLeft,
		0,
		1-fPercentCroppedFromRight,
		1);

	m_sprStreamNormal.SetCustomTextureRect( frectCustomTexRect );
	m_sprStreamPassing.SetCustomTextureRect( frectCustomTexRect );
	m_sprStreamHot.SetCustomTextureRect( frectCustomTexRect );

	m_sprStreamPassing.SetDiffuse( RageColor(1,1,1,m_fPassingAlpha) );
	m_sprStreamHot.SetDiffuse( RageColor(1,1,1,m_fHotAlpha) );

	if( m_fPassingAlpha < 1 && m_fHotAlpha < 1)
		m_sprStreamNormal.Draw();
	if( m_fHotAlpha < 1)
		m_sprStreamPassing.Draw();
	m_sprStreamHot.Draw();
}

void StreamDisplay::DrawMask( float fPercent )
{
	RectF rect;

	int iChamber;
	float fChamberOverflowPercent;
	GetChamberIndexAndOverslow( fPercent, iChamber, fChamberOverflowPercent );
	float fRightPercent = GetRightEdgePercent( iChamber, fChamberOverflowPercent );
	float fHeightPercent = GetHeightPercent( iChamber, fChamberOverflowPercent );
	float fChamberLeftPercent = GetChamberLeftPercent( iChamber );
	float fChamberRightPercent = GetChamberRightPercent( iChamber );

	// draw mask for vertical chambers
	rect.left	= -m_fMeterWidth/2 + fChamberLeftPercent*m_fMeterWidth-1;
	rect.top	= -m_fMeterHeight/2;
	rect.right	= -m_fMeterWidth/2 + fChamberRightPercent*m_fMeterWidth+1;
	rect.bottom	= -m_fMeterHeight/2 + fHeightPercent*m_fMeterHeight;

	rect.left  = MIN( rect.left,  + m_fMeterWidth/2 );
	rect.right = MIN( rect.right, + m_fMeterWidth/2 );

	m_quadMask.StretchTo( rect );
	m_quadMask.Draw();

	// draw mask for horizontal chambers
	rect.left	= -m_fMeterWidth/2 + fRightPercent*m_fMeterWidth; 
	rect.top	= -m_fMeterHeight/2;
	rect.right	= +m_fMeterWidth/2;
	rect.bottom	= +m_fMeterHeight/2;

	rect.left  = MIN( rect.left,  + m_fMeterWidth/2 );
	rect.right = MIN( rect.right, + m_fMeterWidth/2 );

	m_quadMask.StretchTo( rect );
	m_quadMask.Draw();
}


/*
 * (c) 2003-2004 Chris Danford
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
