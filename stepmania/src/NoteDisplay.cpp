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


bool g_bDrawHoldHeadForTapsOnSameRow, g_bDrawTapOnTopOfHoldHead, g_bDrawTapOnTopOfHoldTail;	// cache

NoteDisplay::NoteDisplay()
{
	g_bDrawHoldHeadForTapsOnSameRow  = GAMEMAN->GetMetricB( "NoteDisplay", "DrawHoldHeadForTapsOnSameRow" );
	g_bDrawTapOnTopOfHoldHead  = GAMEMAN->GetMetricB( "NoteDisplay", "DrawTapOnTopOfHoldHead" );
	g_bDrawTapOnTopOfHoldTail  = GAMEMAN->GetMetricB( "NoteDisplay", "DrawTapOnTopOfHoldTail" );

	// the owner of the NoteDisplay must call load on the gray and color parts
//	m_sprHoldParts.Load( THEME->GetPathTo(GRAPHIC_COLOR_ARROW_GRAY_PART) );	
//	m_sprTapParts.Load( THEME->GetPathTo(GRAPHIC_COLOR_ARROW_COLOR_PART) );
}


void NoteDisplay::Load( int iColNum, PlayerNumber pn )
{
	m_PlayerNumber = pn;

	CString sTapPartsPath = GAMEMAN->GetPathTo(iColNum, "tap parts");
	m_sprTapParts.Load( sTapPartsPath );
	if( m_sprTapParts.GetNumStates() % 2 != 0 )
		throw RageException( "Tap Parts '%s' must have an even number of frames.", sTapPartsPath.GetString() );
	m_sprTapParts.StopAnimating();
	m_sprTapParts.TurnShadowOff();

	CString sHoldPartsPath = GAMEMAN->GetPathTo(iColNum, "hold parts");
	m_sprHoldParts.Load( sHoldPartsPath );
	if( m_sprHoldParts.GetTexture()->GetFramesWide() != 4  ||  m_sprHoldParts.GetTexture()->GetFramesHigh() != 2 )
		throw RageException( "Hold Parts '%s' must have 4x2 frames.", sHoldPartsPath.GetString() );
	m_sprHoldParts.StopAnimating();
	m_sprHoldParts.TurnShadowOff();


	m_colorTapTweens.clear();
	const CString sColorsFilePath = GAMEMAN->GetPathTo( iColNum, "Tap.colors" );
	FILE* fp = fopen( sColorsFilePath, "r" );
	if( fp == NULL )
		throw RageException( "Couldn't open .colors file '%s'", sColorsFilePath.GetString() );

	bool bSuccess;
	do
	{
		RageColor color;
		int retval = fscanf( fp, "%f,%f,%f,%f\n", &color.r, &color.g, &color.b, &color.a );
		bSuccess = (retval == 4);
		if( bSuccess )
			m_colorTapTweens.push_back( color );
	} while( bSuccess );

	if( m_colorTapTweens.empty() )
		m_colorTapTweens.push_back( RageColor(1,1,1,1) );

	fclose( fp );
	return;
}


int NoteDisplay::GetTapGrayFrameNo( const float fNoteBeat )
{
	const int iNumGrayPartFrames = m_sprTapParts.GetNumStates()/2;
	
	const float fSongBeat = GAMESTATE->m_fSongBeat;
	const float fPercentIntoBeat = fSongBeat - (int)fSongBeat;
	int iFrameNo = int(fPercentIntoBeat * iNumGrayPartFrames) % iNumGrayPartFrames;
	if( iFrameNo < 0 )
		iFrameNo += iNumGrayPartFrames;

	return iFrameNo * 2;
}

int NoteDisplay::GetTapColorFrameNo( const float fNoteBeat )
{
	return GetTapGrayFrameNo(fNoteBeat) + 1;
}

void NoteDisplay::GetTapEdgeColors( const float fNoteBeat, RageColor &colorLeadingOut, RageColor &colorTrailingOut )
{
// Chris: If EZ2 doesn't use a color part, leave that part of the NoteSkin graphic empty
//
// Andy:
//	if (GAMESTATE->m_CurGame == GAME_EZ2)
//	{
//		return; // Get out of this, as it breaks EZ2, and besides, Ez2 doesn't use COLOR ARROWS
//	}

	////////////////////////////////////////////////////////////
	// ADDED 2-14-2002 - BrianB 
	//
	// This section has been added for the different arrow
	// display types:
	//
	// Note: Each note has a different color, but does not tween
	// Flat: Each note has the same color, and does tween
	// Plain: Each note has the same color, but does not tween
	//
	////////////////////////////////////////////////////////////

	const PlayerOptions::ColorType ct = GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_ColorType;
	const float fNotePercentIntoBeat = fNoteBeat - (int)fNoteBeat;
	const float fSongBeat = GAMESTATE->m_fSongBeat;
	float fPercentThroughColors = fmodf( fSongBeat, (float)BEATS_PER_MEASURE ) / BEATS_PER_MEASURE;

	switch( ct )
	{
	case PlayerOptions::COLOR_VIVID:
	case PlayerOptions::COLOR_NOTE:
		fPercentThroughColors += fNotePercentIntoBeat;	
		break;
	}
	if( fPercentThroughColors < 0 )
		fPercentThroughColors += 1;
	else
		fPercentThroughColors -= (int)fPercentThroughColors;



	float fPercentThroughColorsLeading=0.f;		// fill these in below
	float fPercentThroughColorsTrailing=0.f;
	switch( ct )
	{
	case PlayerOptions::COLOR_VIVID:
	case PlayerOptions::COLOR_NOTE:
	case PlayerOptions::COLOR_FLAT:
		// use different edge colors so that the arrows look more colorful
		fPercentThroughColorsLeading = fPercentThroughColors;	
		fPercentThroughColorsTrailing = fmodf( fPercentThroughColors + 0.20f, 1 );
		break;
	case PlayerOptions::COLOR_PLAIN:
		// use same colors for both edges, making it look "plain"
		fPercentThroughColorsLeading = fPercentThroughColors;	
		fPercentThroughColorsTrailing = fPercentThroughColors;
		break;
	default:
		ASSERT(0);
	}
	

	if( ct == PlayerOptions::COLOR_NOTE )
	{
		RageColor color = GetNoteColorFromBeat( fNoteBeat );
		colorLeadingOut = color;
		colorTrailingOut = color;

		// add a little bit of white so the note doesn't look so plain
		colorLeadingOut.r += 0.3f * fabsf( fPercentThroughColorsLeading - 0.5f );
		colorLeadingOut.g += 0.3f * fabsf( fPercentThroughColorsLeading - 0.5f );
		colorLeadingOut.b += 0.3f * fabsf( fPercentThroughColorsLeading - 0.5f );
		colorTrailingOut.r += 0.3f * fabsf( fPercentThroughColorsTrailing - 0.5f );
		colorTrailingOut.g += 0.3f * fabsf( fPercentThroughColorsTrailing - 0.5f );
		colorTrailingOut.b += 0.3f * fabsf( fPercentThroughColorsTrailing - 0.5f );
		return;
	}

	float fLeadingColorIndex  = fPercentThroughColorsLeading * m_colorTapTweens.size();
	float fTrailingColorIndex = fPercentThroughColorsTrailing* m_colorTapTweens.size();

	float fLeadingColorWeightOf2 = fLeadingColorIndex - int(fLeadingColorIndex);
	int iLeadingColor1 = int(fLeadingColorIndex);
	int iLeadingColor2 = (iLeadingColor1 + 1) % m_colorTapTweens.size();
	colorLeadingOut = m_colorTapTweens[iLeadingColor1] * (1-fLeadingColorWeightOf2) + m_colorTapTweens[iLeadingColor2] * fLeadingColorWeightOf2;

	float fTrailingColorWeightOf2 = fTrailingColorIndex - int(fTrailingColorIndex);
	int iTrailingColor1 = int(fTrailingColorIndex);
	int iTrailingColor2 = (iTrailingColor1 + 1) % m_colorTapTweens.size();
	colorTrailingOut = m_colorTapTweens[iTrailingColor1] * (1-fTrailingColorWeightOf2) + m_colorTapTweens[iTrailingColor2] * fTrailingColorWeightOf2;
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

	const int	fYStep = 8;		// draw a segment every 8 pixels	// this requires that the texture dimensions be a multiple of 8

	DISPLAY->SetTexture( m_sprHoldParts.GetTexture() );
	DISPLAY->SetBlendModeNormal();
	if( bDrawGlowOnly )
		DISPLAY->SetTextureModeGlow();
	else
		DISPLAY->SetTextureModeModulate();
	DISPLAY->EnableTextureWrapping();

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

		// the shift by -0.5 is to align texels to pixels

		if( bDrawGlowOnly && colorGlowTop.a==0 && colorGlowBottom.a==0 )
			continue;
		if( !bDrawGlowOnly && colorDiffuseTop.a==0 && colorDiffuseBottom.a==0 )
			continue;

		static RageVertex v[4];
		v[0].p = RageVector3(fXTopLeft-0.5f,    fYTop-0.5f,   0);	v[0].c = bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop;		v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop),
		v[1].p = RageVector3(fXTopRight-0.5f,   fYTop-0.5f,   0);	v[1].c = bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop;    	v[1].t = RageVector2(fTexCoordRight, fTexCoordTop);
		v[2].p = RageVector3(fXBottomRight-0.5f,fYBottom-0.5f,0);	v[2].c = bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom; 	v[2].t = RageVector2(fTexCoordRight, fTexCoordBottom);
		v[3].p = RageVector3(fXBottomLeft-0.5f, fYBottom-0.5f,0);	v[3].c = bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom; 	v[3].t = RageVector2(fTexCoordLeft,  fTexCoordBottom);

		DISPLAY->DrawQuad( v );
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

		static RageVertex v[4];
		v[0].p = RageVector3(fXTopLeft-0.5f,    fYTop-0.5f,   0);	v[0].c = bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop;		v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop);
		v[1].p = RageVector3(fXTopRight-0.5f,   fYTop-0.5f,   0);	v[1].c = bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop;		v[1].t = RageVector2(fTexCoordRight, fTexCoordTop);
		v[2].p = RageVector3(fXBottomRight-0.5f,fYBottom-0.5f,0);	v[2].c = bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom;	v[2].t = RageVector2(fTexCoordRight, fTexCoordBottom);
		v[3].p = RageVector3(fXBottomLeft-0.5f, fYBottom-0.5f,0);	v[3].c = bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom;	v[3].t = RageVector2(fTexCoordLeft,  fTexCoordBottom);
		DISPLAY->DrawQuad( v );
	}	

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

			// the shift by -0.5 is to align texels to pixels

			if( bDrawGlowOnly && colorGlowTop.a==0 && colorGlowBottom.a==0 )
				continue;
			if( !bDrawGlowOnly && colorDiffuseTop.a==0 && colorDiffuseBottom.a==0 )
				continue;

			static RageVertex v[4];
			v[0].p = RageVector3(fXTopLeft-0.5f,    fYTop-0.5f,   0);	v[0].c = bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop;		v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop);
			v[1].p = RageVector3(fXTopRight-0.5f,   fYTop-0.5f,   0);	v[1].c = bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop;		v[1].t = RageVector2(fTexCoordRight, fTexCoordTop);
			v[2].p = RageVector3(fXBottomRight-0.5f,fYBottom-0.5f,0);	v[2].c = bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom;	v[2].t = RageVector2(fTexCoordRight, fTexCoordBottom);
			v[3].p = RageVector3(fXBottomLeft-0.5f, fYBottom-0.5f,0);	v[3].c = bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom;	v[3].t = RageVector2(fTexCoordLeft,  fTexCoordBottom);
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
	const int iGrayPartFrameNo	= GetTapGrayFrameNo( fBeat );
	const int iColorPartFrameNo	= GetTapColorFrameNo( fBeat );
	const float fColorScale		= SCALE(fLife,0,1,0.2f,1);

	RageColor colorGrayPart = RageColor(fColorScale,fColorScale,fColorScale,1);
	RageColor colorLeadingEdge;
	RageColor colorTrailingEdge;
	GetTapEdgeColors( fBeat, colorLeadingEdge, colorTrailingEdge );
	colorGrayPart.a		*= fAlpha;
	colorLeadingEdge.a	*= fAlpha;
	colorTrailingEdge.a *= fAlpha;

	colorLeadingEdge.r *= fColorScale;
	colorLeadingEdge.g *= fColorScale;
	colorLeadingEdge.b *= fColorScale;
	colorTrailingEdge.r *= fColorScale;
	colorTrailingEdge.g *= fColorScale;
	colorTrailingEdge.b *= fColorScale;

	if( bOnSameRowAsHoldStart  &&  g_bDrawHoldHeadForTapsOnSameRow )
	{
		//
		// draw hold head
		//
		m_sprHoldParts.SetXY( fXPos, fYPos );
		m_sprHoldParts.SetDiffuse( colorGrayPart );
		m_sprHoldParts.SetGlow( RageColor(1,1,1,fGlow) );
		m_sprHoldParts.StopUsingCustomCoords();
		m_sprHoldParts.SetState( 0 );
		m_sprHoldParts.Draw();
	}
	else	
	{
		m_sprTapParts.SetXY( fXPos, fYPos );
		m_sprTapParts.SetRotation( fRotation );
		m_sprTapParts.SetGlow( RageColor(1,1,1,fGlow) );

		//
		// draw gray part
		//
		m_sprTapParts.SetState( iGrayPartFrameNo );
		m_sprTapParts.SetDiffuse( colorGrayPart );
		m_sprTapParts.Draw();

		//
		// draw color part
		//
		m_sprTapParts.SetState( iColorPartFrameNo );
		m_sprTapParts.SetDiffuseTopEdge( colorLeadingEdge );
		m_sprTapParts.SetDiffuseBottomEdge( colorTrailingEdge );
		m_sprTapParts.Draw();
	}
}