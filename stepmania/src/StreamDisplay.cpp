#include "global.h"
#include "StreamDisplay.h"
#include "GameState.h"
#include <float.h>

StreamDisplay::StreamDisplay()
{
	m_fMeterHeight = 1;
	m_fMeterWidth = 1;
	m_iNumStrips = 1;
	m_iNumChambers = 1;
	m_fPercent = 0;
	m_fTrailingPercent = 0;
	m_fVelocity = 0;
	m_fPassingAlpha = 0;
	m_fHotAlpha = 0;
}

void StreamDisplay::Load( 
	float fMeterWidth, 
	float fMeterHeight,
	int iNumStrips,
	int iNumChambers, 
	const CString &sNormalPath, 
	const CString &sHotPath, 
	const CString &sPassingPath,
	const apActorCommands &acNormalOnCommand,
	const apActorCommands &acHotOnCommand,
	const apActorCommands &acPassingOnCommand
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
	m_sprStreamNormal.RunCommands( acNormalOnCommand );

	ID.filename = sHotPath;
	m_sprStreamHot.Load( ID );
	m_sprStreamHot.SetUseZBuffer( true );
	m_sprStreamHot.RunCommands( acHotOnCommand );
	
	ID.filename = sPassingPath;
	m_sprStreamPassing.Load( ID );
	m_sprStreamPassing.SetUseZBuffer( true );
	m_sprStreamPassing.RunCommands( acPassingOnCommand );
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
	float fStripWidthInPercent = 1.0f/m_iNumStrips;
	float fPercentBetweenStrips = 1.0f/m_iNumStrips;

	// round down so that the chamber overflows align
	if( m_iNumChambers > 10 )
		fPercentBetweenStrips = Quantize( fPercentBetweenStrips-fChamberWidthInPercent/2, fChamberWidthInPercent );
	
	
	if( m_iNumChambers > 3 )
		fPercentBetweenStrips -= 2*fChamberWidthInPercent;

	float fPercentOffset = fmodf( GAMESTATE->m_fSongBeat/4+1000, fPercentBetweenStrips );
	ASSERT( fPercentOffset >= 0  &&  fPercentOffset <= fPercentBetweenStrips );

	// "+fPercentBetweenStrips" so that the whole area is overdrawn 2x
	for( float f=fPercentOffset+1+fPercentBetweenStrips; f>=0; f-=fPercentBetweenStrips )
	{
		DrawMask( f );
		DrawStrip( f, fStripWidthInPercent );
	}
}

void StreamDisplay::SetPercent( float fPercent )
{
	DEBUG_ASSERT( fPercent >= 0.0f && fPercent <= 1.0f );
	if( _isnan(fPercent) )
	{
		DEBUG_ASSERT_M( 0, "fPercent is NaN" );
		fPercent = 1;
	}
	if( !_finite(fPercent) )
	{
		DEBUG_ASSERT_M( 0, "fPercent is infinite" );
		fPercent = 1;
	}

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

void StreamDisplay::DrawStrip( float fRightEdgePercent, float fStripWidthInPercent )
{
	m_sprStreamNormal.ZoomToWidth(		m_fMeterWidth*fStripWidthInPercent );
	m_sprStreamPassing.ZoomToWidth(		m_fMeterWidth*fStripWidthInPercent );
	m_sprStreamHot.ZoomToWidth(			m_fMeterWidth*fStripWidthInPercent );

	m_sprStreamNormal.ZoomToHeight(		m_fMeterHeight );
	m_sprStreamPassing.ZoomToHeight(	m_fMeterHeight );
	m_sprStreamHot.ZoomToHeight(		m_fMeterHeight );

	m_sprStreamNormal.SetHorizAlign(	Actor::align_right );
	m_sprStreamPassing.SetHorizAlign(	Actor::align_right );
	m_sprStreamHot.SetHorizAlign(		Actor::align_right );

	m_sprStreamNormal.SetX(		-m_fMeterWidth/2 + m_fMeterWidth*fRightEdgePercent );
	m_sprStreamPassing.SetX(	-m_fMeterWidth/2 + m_fMeterWidth*fRightEdgePercent );
	m_sprStreamHot.SetX(		-m_fMeterWidth/2 + m_fMeterWidth*fRightEdgePercent );

	const float fMeterLeftEdgePixels =  SCALE( 0.0f, 0.0f, 1.0f, -m_fMeterWidth/2, +m_fMeterWidth/2 );
	const float fMeterRightEdgePixels = SCALE( 1.0f, 0.0f, 1.0f, -m_fMeterWidth/2, +m_fMeterWidth/2 );

	const float fStreamRightEdgePixels = SCALE( fRightEdgePercent, 0.0f, 1.0f, -m_fMeterWidth/2, +m_fMeterWidth/2 );
	const float fStreamLeftEdgePixels = fStreamRightEdgePixels - m_sprStreamNormal.GetZoomedWidth();

	const float fLeftOverhangPixels = max( 0, fMeterLeftEdgePixels - fStreamLeftEdgePixels );
	const float fStreamCropLeftPercent = SCALE( fLeftOverhangPixels, 0.0f, m_sprStreamNormal.GetZoomedWidth(), 0.0f, 1.0f );

	const float fRightOverhangPixels = max( 0, fStreamRightEdgePixels - fMeterRightEdgePixels );
	const float fStreamCropRightPercent = SCALE( fRightOverhangPixels, 0.0f, m_sprStreamNormal.GetZoomedWidth(), 0.0f, 1.0f );

	m_sprStreamNormal.SetCropLeft(	fStreamCropLeftPercent );
	m_sprStreamPassing.SetCropLeft(	fStreamCropLeftPercent );
	m_sprStreamHot.SetCropLeft(		fStreamCropLeftPercent );
	
	m_sprStreamNormal.SetCropRight(	fStreamCropRightPercent );
	m_sprStreamPassing.SetCropRight(fStreamCropRightPercent );
	m_sprStreamHot.SetCropRight(	fStreamCropRightPercent );


	float fOrigPassingAlpha = m_sprStreamPassing.GetDiffuse().a;
	float fOrigHotAlpha = m_sprStreamHot.GetDiffuse().a;

	m_sprStreamPassing.SetDiffuseAlpha( m_fPassingAlpha * fOrigPassingAlpha );
	m_sprStreamHot.SetDiffuseAlpha( m_fHotAlpha * fOrigHotAlpha );

	if( m_fPassingAlpha < 1 && m_fHotAlpha < 1)
		m_sprStreamNormal.Draw();
	if( m_fHotAlpha < 1)
		m_sprStreamPassing.Draw();
	m_sprStreamHot.Draw();

	m_sprStreamPassing.SetDiffuseAlpha( fOrigPassingAlpha );
	m_sprStreamHot.SetDiffuseAlpha( fOrigHotAlpha );
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
