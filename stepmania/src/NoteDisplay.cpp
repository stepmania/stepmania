#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: NoteDisplay

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Brian Bugh
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "NoteDisplay.h"
#include "Notes.h"
#include "PrefsManager.h"
#include "GameState.h"
#include "GameManager.h"
#include "RageException.h"
#include "ArrowEffects.h"
#include "RageLog.h"
#include <math.h>
#include "RageDisplay.h"


// cache
bool g_bDrawHoldHeadForTapsOnSameRow, g_bDrawTapOnTopOfHoldHead, g_bDrawTapOnTopOfHoldTail;
float	g_fTapAnimationLengthInBeats;
float	g_bAllowVivid;

NoteDisplay::NoteDisplay()
{
	g_bDrawHoldHeadForTapsOnSameRow  = GAMEMAN->GetMetricB( "NoteDisplay", "DrawHoldHeadForTapsOnSameRow" );
	g_bDrawTapOnTopOfHoldHead  = GAMEMAN->GetMetricB( "NoteDisplay", "DrawTapOnTopOfHoldHead" );
	g_bDrawTapOnTopOfHoldTail  = GAMEMAN->GetMetricB( "NoteDisplay", "DrawTapOnTopOfHoldTail" );
	g_fTapAnimationLengthInBeats = GAMEMAN->GetMetricF( "NoteDisplay", "TapAnimationLengthInBeats" );
	g_bAllowVivid = GAMEMAN->GetMetricB( "NoteDisplay", "AllowVivid" );
}


void NoteDisplay::Load( int iColNum, PlayerNumber pn )
{
	m_PlayerNumber = pn;

	CString sTapPath = GAMEMAN->GetPathTo(iColNum, "tap");
	m_sprTap.Load( sTapPath );
	m_sprTap.StopAnimating();
	m_sprTap.TurnShadowOff();

	CString sHoldPartsPath = GAMEMAN->GetPathTo(iColNum, "hold parts 4x2");
	m_sprHoldParts.Load( sHoldPartsPath );
	if( m_sprHoldParts.GetTexture()->GetFramesWide() != 4  ||  m_sprHoldParts.GetTexture()->GetFramesHigh() != 2 )
		RageException::Throw( "Hold Parts '%s' must have 4x2 frames.", sHoldPartsPath.GetString() );
	m_sprHoldParts.StopAnimating();
	m_sprHoldParts.TurnShadowOff();

	return;
}


int NoteDisplay::GetTapFrameNo( const float fNoteBeat )
{
	const int iNumFrames = m_sprTap.GetNumStates();
	
	float fSongBeat = GAMESTATE->m_fSongBeat;
	float fPercentIntoMeasure = fmodf( fSongBeat, g_fTapAnimationLengthInBeats ) / g_fTapAnimationLengthInBeats;
	fPercentIntoMeasure += 1/(iNumFrames*2.f);	// fudge factor so that the "full white" frame shows when the arrow overlaps the receptors
	int iSongBeatFrameContrib = fPercentIntoMeasure * iNumFrames;

	float fBeatFraction = fmodf( fNoteBeat, 1.0f );
	fBeatFraction = froundf( fBeatFraction, 0.25f );
	int iBeatFractionContrib = fBeatFraction * iNumFrames;

	g_bAllowVivid = GAMEMAN->GetMetricB( "NoteDisplay", "AllowVivid" );

	int iFrameNo;
	if( g_bAllowVivid  &&  GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_ColorType == PlayerOptions::COLOR_VIVID )
		iFrameNo = (iSongBeatFrameContrib + iBeatFractionContrib) % iNumFrames;
	else
		iFrameNo = (iSongBeatFrameContrib) % iNumFrames;
	
	if( iFrameNo < 0 )
		iFrameNo += iNumFrames;

	return iFrameNo;
}

void NoteDisplay::DrawHold( const HoldNote& hn, const bool bActive, const float fLife, const float fPercentFadeToFail, bool bDrawGlowOnly )
{
	// bDrawGlowOnly is a little hacky.  We need to draw the diffuse part and the glow part one pass at a time to minimize state changes

	const bool bReverse = GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_bReverseScroll;

	const int	iCol			= hn.m_iTrack;
	const float fStartYOffset	= ArrowGetYOffset(	m_PlayerNumber, hn.m_fStartBeat );
	const float fStartYPos		= ArrowGetYPos(		m_PlayerNumber, fStartYOffset );
	const float fEndYOffset		= ArrowGetYOffset(	m_PlayerNumber, hn.m_fEndBeat );
	const float fEndYPos		= ArrowGetYPos(		m_PlayerNumber, fEndYOffset );

	// draw from bottom to top
	const float fFrameWidth = m_sprHoldParts.GetUnzoomedWidth();
	const float fFrameHeight = m_sprHoldParts.GetUnzoomedHeight();
	const float fBodyHeight = fFrameHeight*2;

	const float fYHead = bReverse ? fEndYPos : fStartYPos;		// the center of the head
	const float fYTail = bReverse ? fStartYPos : fEndYPos;		// the center the tail

	const float fYHeadTop = fYHead-fFrameHeight/2;		
	const float fYHeadBottom = fYHead+fFrameHeight/2;	

	const float fYTailTop = fYTail-fFrameHeight/2;		
	const float fYTailBottom = fYTail+fFrameHeight/2;	

	const float fYBodyTop = g_bDrawTapOnTopOfHoldHead ? fYHeadBottom : fYHead;			// middle of head		
	const float fYBodyBottom = fYTailTop;	// top of tail

	const int	fYStep = fFrameHeight/4.f;	// 4 segments per frame

	DISPLAY->SetTexture( m_sprHoldParts.GetTexture() );
	DISPLAY->SetBlendModeNormal();
	if( bDrawGlowOnly )
		DISPLAY->SetTextureModeGlow();
	else
		DISPLAY->SetTextureModeModulate();
	DISPLAY->EnableTextureWrapping();

	static RageVertex queue[4096];
	RageVertex *v=&queue[0];

	//
	// Draw the tail
	//
	float fY;
	// HACK:  +0.05 below to try and get rid of ugly border at top of tail
	for( fY=max(fYTailTop-0.05f,fYHead); fY<fYTailBottom; fY+=fYStep )	// don't draw the part of the tail that is before the middle of the head
	{
		const float fYTop			= fY;
		const float fYBottom		= fY+min(fYStep, fYTailBottom-fY);
		const float fXTop			= ArrowGetXPos( m_PlayerNumber, iCol, fYTop );
		const float fXBottom		= ArrowGetXPos( m_PlayerNumber, iCol, fYBottom );
		const float fXTopLeft		= fXTop - fFrameWidth/2;
		const float fXTopRight		= fXTop + fFrameWidth/2;
		const float fXBottomLeft	= fXBottom - fFrameWidth/2;
		const float fXBottomRight	= fXBottom + fFrameWidth/2;
		const float fTopDistFromTailTop		= fYTop - fYTailTop;
		const float fBottomDistFromTailTop	= fYBottom - fYTailTop;
			  float fTexCoordTop	= SCALE( fTopDistFromTailTop,    0, fFrameHeight, 0.5f, 1.0f );
			  float fTexCoordBottom = SCALE( fBottomDistFromTailTop, 0, fFrameHeight, 0.5f, 1.0f );
		if( fTexCoordTop > 1 && fTexCoordBottom > 1 )
		{
			fTexCoordTop -= (int)fTexCoordTop;
			fTexCoordBottom -= (int)fTexCoordTop;
		}
		ASSERT( fBottomDistFromTailTop-0.001 <= fFrameHeight );
		const float fTexCoordLeft	= bActive ? 0.25f : 0.00f;
		const float fTexCoordRight	= bActive ? 0.50f : 0.25f;
		const float	fAlphaTop		= ArrowGetAlpha( m_PlayerNumber, fYTop, fPercentFadeToFail );
		const float	fAlphaBottom	= ArrowGetAlpha( m_PlayerNumber, fYBottom, fPercentFadeToFail );
		const float	fGlowTop		= ArrowGetGlow( m_PlayerNumber, fYTop, fPercentFadeToFail );
		const float	fGlowBottom		= ArrowGetGlow( m_PlayerNumber, fYBottom, fPercentFadeToFail );
		const float fColorScale		= SCALE(fLife,0,1,0.2f,1);
		const RageColor colorDiffuseTop		= RageColor(fColorScale,fColorScale,fColorScale,fAlphaTop);
		const RageColor colorDiffuseBottom	= RageColor(fColorScale,fColorScale,fColorScale,fAlphaBottom);
		const RageColor colorGlowTop		= RageColor(1,1,1,fGlowTop);
		const RageColor colorGlowBottom		= RageColor(1,1,1,fGlowBottom);

		if( bDrawGlowOnly && colorGlowTop.a==0 && colorGlowBottom.a==0 )
			continue;
		if( !bDrawGlowOnly && colorDiffuseTop.a==0 && colorDiffuseBottom.a==0 )
			continue;

		v[0].p = RageVector3(fXTopLeft,    fYTop,   0);	v[0].c = bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop;		v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop),
		v[1].p = RageVector3(fXTopRight,   fYTop,   0);	v[1].c = bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop;    	v[1].t = RageVector2(fTexCoordRight, fTexCoordTop);
		v[2].p = RageVector3(fXBottomRight,fYBottom,0);	v[2].c = bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom; 	v[2].t = RageVector2(fTexCoordRight, fTexCoordBottom);
		v[3].p = RageVector3(fXBottomLeft, fYBottom,0);	v[3].c = bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom; 	v[3].t = RageVector2(fTexCoordLeft,  fTexCoordBottom);
		v+=4;
	}

	//
	// Draw the body
	//
	for( fY=fYBodyTop; fY<=fYBodyBottom+1; fY+=fYStep )	// top to bottom
	{
		const float fYTop			= fY;
		const float fYBottom		= min( fY+fYStep, fYTailTop+1 );
		const float fXTop			= ArrowGetXPos( m_PlayerNumber, iCol, fYTop );
		const float fXBottom		= ArrowGetXPos( m_PlayerNumber, iCol, fYBottom );
		const float fXTopLeft		= fXTop - fFrameWidth/2;
		const float fXTopRight		= fXTop + fFrameWidth/2;
		const float fXBottomLeft	= fXBottom - fFrameWidth/2;
		const float fXBottomRight	= fXBottom + fFrameWidth/2;
		const float fTopDistFromTailTop		= fYTailTop - fYTop;
		const float fBottomDistFromTailTop	= fYTailTop - fYBottom;
			  float fTexCoordTop	= SCALE( fTopDistFromTailTop,    0, fBodyHeight, 1, 0 );
			  float fTexCoordBottom = SCALE( fBottomDistFromTailTop, 0, fBodyHeight, 1, 0 );
		if( fTexCoordTop > 1 && fTexCoordBottom > 1 )
		{
			fTexCoordTop -= (int)fTexCoordTop;
			fTexCoordBottom -= (int)fTexCoordTop;
		}
		const float fTexCoordLeft	= bActive ? 0.75f : 0.50f;
		const float fTexCoordRight	= bActive ? 1.00f : 0.75f;
		const float	fAlphaTop		= ArrowGetAlpha( m_PlayerNumber, fYTop, fPercentFadeToFail );
		const float	fAlphaBottom	= ArrowGetAlpha( m_PlayerNumber, fYBottom, fPercentFadeToFail );
		const float	fGlowTop		= ArrowGetGlow( m_PlayerNumber, fYTop, fPercentFadeToFail );
		const float	fGlowBottom		= ArrowGetGlow( m_PlayerNumber, fYBottom, fPercentFadeToFail );
		const float fColorScale		= SCALE(fLife,0,1,0.2f,1);
		const RageColor colorDiffuseTop		= RageColor(fColorScale,fColorScale,fColorScale,fAlphaTop);
		const RageColor colorDiffuseBottom	= RageColor(fColorScale,fColorScale,fColorScale,fAlphaBottom);
		const RageColor colorGlowTop		= RageColor(1,1,1,fGlowTop);
		const RageColor colorGlowBottom		= RageColor(1,1,1,fGlowBottom);

		if( bDrawGlowOnly && colorGlowTop.a==0 && colorGlowBottom.a==0 )
			continue;
		if( !bDrawGlowOnly && colorDiffuseTop.a==0 && colorDiffuseBottom.a==0 )
			continue;

		v[0].p = RageVector3(fXTopLeft,    fYTop,   0);	v[0].c = bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop;		v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop);
		v[1].p = RageVector3(fXTopRight,   fYTop,   0);	v[1].c = bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop;		v[1].t = RageVector2(fTexCoordRight, fTexCoordTop);
		v[2].p = RageVector3(fXBottomRight,fYBottom,0);	v[2].c = bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom;	v[2].t = RageVector2(fTexCoordRight, fTexCoordBottom);
		v[3].p = RageVector3(fXBottomLeft, fYBottom,0);	v[3].c = bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom;	v[3].t = RageVector2(fTexCoordLeft,  fTexCoordBottom);
		v+=4;
	}	
	DISPLAY->DrawQuads( queue, v-queue );
	v=&queue[0];

	if( g_bDrawTapOnTopOfHoldHead )
	{
		//
		// Draw the head
		//
		float fY;
		for( fY=fYHeadTop; fY<fYHeadBottom; fY+=fYStep )
		{
			const float fYTop			= fY;
			const float fYBottom		= fY+min(fYStep, fYHeadBottom-fY);
			const float fXTop			= ArrowGetXPos( m_PlayerNumber, iCol, fYTop );
			const float fXBottom		= ArrowGetXPos( m_PlayerNumber, iCol, fYBottom );
			const float fXTopLeft		= fXTop - fFrameWidth/2;
			const float fXTopRight		= fXTop + fFrameWidth/2;
			const float fXBottomLeft	= fXBottom - fFrameWidth/2;
			const float fXBottomRight	= fXBottom + fFrameWidth/2;
			const float fTopDistFromHeadTop		= fYTop - fYHeadTop;
			const float fBottomDistFromHeadTop	= fYBottom - fYHeadTop;
			const float fTexCoordTop	= SCALE( fTopDistFromHeadTop,    0, fFrameHeight, 0.0f, 0.499f );
			const float fTexCoordBottom = SCALE( fBottomDistFromHeadTop, 0, fFrameHeight, 0.0f, 0.499f );
			ASSERT( fBottomDistFromHeadTop-0.0001 <= fFrameHeight );
			const float fTexCoordLeft	= bActive ? 0.25f : 0.00f;
			const float fTexCoordRight	= bActive ? 0.50f : 0.25f;
			const float	fAlphaTop		= ArrowGetAlpha( m_PlayerNumber, fYTop, fPercentFadeToFail );
			const float	fAlphaBottom	= ArrowGetAlpha( m_PlayerNumber, fYBottom, fPercentFadeToFail );
			const float	fGlowTop		= ArrowGetGlow( m_PlayerNumber, fYTop, fPercentFadeToFail );
			const float	fGlowBottom		= ArrowGetGlow( m_PlayerNumber, fYBottom, fPercentFadeToFail );
			const float fColorScale		= SCALE(fLife,0,1,0.2f,1);
			const RageColor colorDiffuseTop		= RageColor(fColorScale,fColorScale,fColorScale,fAlphaTop);
			const RageColor colorDiffuseBottom	= RageColor(fColorScale,fColorScale,fColorScale,fAlphaBottom);
			const RageColor colorGlowTop		= RageColor(1,1,1,fGlowTop);
			const RageColor colorGlowBottom		= RageColor(1,1,1,fGlowBottom);

			if( bDrawGlowOnly && colorGlowTop.a==0 && colorGlowBottom.a==0 )
				continue;
			if( !bDrawGlowOnly && colorDiffuseTop.a==0 && colorDiffuseBottom.a==0 )
				continue;

			v[0].p = RageVector3(fXTopLeft,    fYTop,   0);	v[0].c = bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop;		v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop);
			v[1].p = RageVector3(fXTopRight,   fYTop,   0);	v[1].c = bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop;		v[1].t = RageVector2(fTexCoordRight, fTexCoordTop);
			v[2].p = RageVector3(fXBottomRight,fYBottom,0);	v[2].c = bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom;	v[2].t = RageVector2(fTexCoordRight, fTexCoordBottom);
			v[3].p = RageVector3(fXBottomLeft, fYBottom,0);	v[3].c = bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom;	v[3].t = RageVector2(fTexCoordLeft,  fTexCoordBottom);
			DISPLAY->DrawQuad( v );
		}
	}
	else
	{
		//
		// Draw head
		//
		{
			fY							= fYHead;
			const float fX				= ArrowGetXPos( m_PlayerNumber, iCol, fY );
			const float	fAlpha			= ArrowGetAlpha( m_PlayerNumber, fY, fPercentFadeToFail );
			const float	fGlow			= ArrowGetGlow( m_PlayerNumber, fY, fPercentFadeToFail );
			const float fColorScale		= SCALE(fLife,0,1,0.2f,1);
			const RageColor colorDiffuse= RageColor(fColorScale,fColorScale,fColorScale,fAlpha);
			const RageColor colorGlow	= RageColor(1,1,1,fGlow);

//			m_sprHoldParts.SetState( bActive?1:0 );
			// HACK:  the border around the edge of on this sprite is super-obvious.  
			m_sprHoldParts.SetCustomTextureRect( bActive ? RectF(0.251f,0.002f,0.499f,0.498f) : RectF(0.001f,0.002f,0.249f,0.498f) );
			m_sprHoldParts.SetXY( fX, fY );
			if( bDrawGlowOnly )
			{
				m_sprHoldParts.SetDiffuse( RageColor(1,1,1,0) );
				m_sprHoldParts.SetGlow( colorGlow );
			}
			else
			{
				m_sprHoldParts.SetDiffuse( colorDiffuse );
				m_sprHoldParts.SetGlow( RageColor(0,0,0,0) );
			}
			m_sprHoldParts.Draw();
		}
	}


	if( g_bDrawTapOnTopOfHoldHead )
		DrawTap( hn.m_iTrack, hn.m_fStartBeat, false, fPercentFadeToFail, fLife );
	if( g_bDrawTapOnTopOfHoldTail )
		DrawTap( hn.m_iTrack, hn.m_fEndBeat, false, fPercentFadeToFail, fLife );


	// now, draw the glow pass
	if( !bDrawGlowOnly )
		DrawHold( hn, bActive, fLife, fPercentFadeToFail, true );
}

void NoteDisplay::DrawTap( const int iCol, const float fBeat, const bool bOnSameRowAsHoldStart, const float fPercentFadeToFail, const float fLife )
{
	const float fYOffset		= ArrowGetYOffset(	m_PlayerNumber, fBeat );
	const float fYPos			= ArrowGetYPos(		m_PlayerNumber, fYOffset );
	const float fRotation		= ArrowGetRotation(	m_PlayerNumber, iCol, fYOffset );
	const float fXPos			= ArrowGetXPos(		m_PlayerNumber, iCol, fYPos );
	const float fAlpha			= ArrowGetAlpha(	m_PlayerNumber, fYPos, fPercentFadeToFail );
	const float fGlow			= ArrowGetGlow(		m_PlayerNumber, fYPos, fPercentFadeToFail );
	const int iTapFrameNo		= GetTapFrameNo( fBeat );
	const float fColorScale		= SCALE(fLife,0,1,0.2f,1);
	RageColor diffuse = RageColor(fColorScale,fColorScale,fColorScale,fAlpha);
	RageColor glow = RageColor(1,1,1,fGlow);

	if( GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_ColorType == PlayerOptions::COLOR_NOTE )
	{
		RageColor noteColor = GetNoteColorFromBeat(fBeat);
		diffuse.r *= noteColor.r;
		diffuse.g *= noteColor.g;
		diffuse.b *= noteColor.b;

		glow = RageColor(1,1,1,1)*fGlow + noteColor*(1-fGlow)*0.7f*fAlpha;
	}

	if( bOnSameRowAsHoldStart  &&  g_bDrawHoldHeadForTapsOnSameRow )
	{
		//
		// draw hold head
		//
		m_sprHoldParts.SetXY( fXPos, fYPos );
		m_sprHoldParts.SetDiffuse( diffuse );
		m_sprHoldParts.SetGlow( glow );
		m_sprHoldParts.StopUsingCustomCoords();
		m_sprHoldParts.SetState( 0 );
		m_sprHoldParts.Draw();
	}
	else	
	{
		m_sprTap.SetXY( fXPos, fYPos );
		m_sprTap.SetRotation( fRotation );
		m_sprTap.SetGlow( glow );
		m_sprTap.SetDiffuse( diffuse );
		m_sprTap.SetState( iTapFrameNo );
		m_sprTap.Draw();
	}
}


//	if( ct == PlayerOptions::COLOR_NOTE )
//	{
//		RageColor color = GetNoteColorFromBeat( fNoteBeat );
//		colorLeadingOut = color;
//		colorTrailingOut = color;
//
//		// add a little bit of white so the note doesn't look so plain
//		colorLeadingOut.r += 0.3f * fabsf( fPercentThroughColorsLeading - 0.5f );
//		colorLeadingOut.g += 0.3f * fabsf( fPercentThroughColorsLeading - 0.5f );
//		colorLeadingOut.b += 0.3f * fabsf( fPercentThroughColorsLeading - 0.5f );
//		colorTrailingOut.r += 0.3f * fabsf( fPercentThroughColorsTrailing - 0.5f );
//		colorTrailingOut.g += 0.3f * fabsf( fPercentThroughColorsTrailing - 0.5f );
//		colorTrailingOut.b += 0.3f * fabsf( fPercentThroughColorsTrailing - 0.5f );
//		return;
//	}
