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


NoteDisplay::NoteDisplay()
{
	// the owner of the NoteDisplay must call load on the gray and color parts

//	m_sprHoldParts.Load( THEME->GetPathTo(GRAPHIC_COLOR_ARROW_GRAY_PART) );	
//	m_sprTapParts.Load( THEME->GetPathTo(GRAPHIC_COLOR_ARROW_COLOR_PART) );

}


void NoteDisplay::Load( int iColNum, PlayerNumber pn )
{
	m_PlayerNumber = pn;

	CString sPath;
	
	sPath = GAMEMAN->GetPathTo(iColNum, GRAPHIC_TAP_PARTS);
	m_sprTapParts.Load( sPath );
	if( m_sprTapParts.GetNumStates() % 2 != 0 )
		throw RageException( "Tap Parts '%s' must have an even number of frames.", sPath );

	sPath = GAMEMAN->GetPathTo(iColNum, GRAPHIC_HOLD_PARTS);
	m_sprHoldParts.Load( sPath );
	if( m_sprHoldParts.GetTexture()->GetFramesWide() != 4  ||  m_sprHoldParts.GetTexture()->GetFramesHigh() != 2 )
		throw RageException( "Hold Parts '%s' must have 4x2 frames.", sPath );

	m_sprTapParts.StopAnimating();
	m_sprTapParts.TurnShadowOff();
	m_sprHoldParts.StopAnimating();
	m_sprHoldParts.TurnShadowOff();
	m_colorTapTweens.RemoveAll();
	GAMEMAN->GetTapTweenColors( iColNum, m_colorTapTweens );	
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

void NoteDisplay::GetTapEdgeColors( const float fNoteBeat, D3DXCOLOR &colorLeadingOut, D3DXCOLOR &colorTrailingOut )
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
		D3DXCOLOR color = GetNoteColorFromBeat( fNoteBeat );
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

	float fLeadingColorIndex  = fPercentThroughColorsLeading * m_colorTapTweens.GetSize();
	float fTrailingColorIndex = fPercentThroughColorsTrailing* m_colorTapTweens.GetSize();

	float fLeadingColorWeightOf2 = fLeadingColorIndex - int(fLeadingColorIndex);
	int iLeadingColor1 = int(fLeadingColorIndex);
	int iLeadingColor2 = (iLeadingColor1 + 1) % m_colorTapTweens.GetSize();
	colorLeadingOut = m_colorTapTweens[iLeadingColor1] * (1-fLeadingColorWeightOf2) + m_colorTapTweens[iLeadingColor2] * fLeadingColorWeightOf2;

	float fTrailingColorWeightOf2 = fTrailingColorIndex - int(fTrailingColorIndex);
	int iTrailingColor1 = int(fTrailingColorIndex);
	int iTrailingColor2 = (iTrailingColor1 + 1) % m_colorTapTweens.GetSize();
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

	const float fYTailTop = fYTail-fFrameHeight/2;		
	const float fYTailBottom = fYTail+fFrameHeight/2;	

	const float fYBodyTop = fYHead;			// middle of head		
//	const float fYBodyBottom = fYTailTop;	// top of tail

	const int	fYStep = 8;		// draw a segment every 8 pixels	// this requires that the texture dimensions be a multiple of 8

	DISPLAY->SetBlendModeNormal();
	if( bDrawGlowOnly )
		DISPLAY->SetColorDiffuse();
	else
		DISPLAY->SetColorTextureMultDiffuse();
	DISPLAY->SetAlphaTextureMultDiffuse();
	DISPLAY->SetTexture( m_sprHoldParts.GetTexture() );

	//
	// Draw the tail
	//
	float fY;
	for( fY=max(fYTailTop,fYHead); fY<fYTailBottom; fY+=fYStep )	// don't draw the part of the tail that is before the middle of the head
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
		const float fTexCoordTop	= SCALE( fTopDistFromTailTop,    0, fFrameHeight, 0.5f, 1.0f );
		const float fTexCoordBottom = SCALE( fBottomDistFromTailTop, 0, fFrameHeight, 0.5f, 1.0f );
		ASSERT( fBottomDistFromTailTop-0.0001 <= fFrameHeight );
		const float fTexCoordLeft	= bActive ? 0.25f : 0.00f;
		const float fTexCoordRight	= bActive ? 0.50f : 0.25f;
		const float	fAlphaTop		= ArrowGetAlpha( m_PlayerNumber, fYTop, fPercentFadeToFail );
		const float	fAlphaBottom	= ArrowGetAlpha( m_PlayerNumber, fYBottom, fPercentFadeToFail );
		const float	fGlowTop		= ArrowGetGlow( m_PlayerNumber, fYTop, fPercentFadeToFail );
		const float	fGlowBottom		= ArrowGetGlow( m_PlayerNumber, fYBottom, fPercentFadeToFail );
		const float fColorScale		= SCALE(fLife,0,1,0.5f,1);
		const D3DXCOLOR colorDiffuseTop		= D3DXCOLOR(fColorScale,fColorScale,fColorScale,fAlphaTop);
		const D3DXCOLOR colorDiffuseBottom	= D3DXCOLOR(fColorScale,fColorScale,fColorScale,fAlphaBottom);
		const D3DXCOLOR colorGlowTop		= D3DXCOLOR(1,1,1,fGlowTop);
		const D3DXCOLOR colorGlowBottom		= D3DXCOLOR(1,1,1,fGlowBottom);

		// the shift by -0.5 is to align texels to pixels

		if( bDrawGlowOnly && colorGlowTop.a==0 && colorGlowBottom.a==0 )
			continue;
		if( !bDrawGlowOnly && colorDiffuseTop.a==0 && colorDiffuseBottom.a==0 )
			continue;

		DISPLAY->AddQuad( 
			D3DXVECTOR3(fXTopLeft-0.5f,    fYTop-0.5f,   0), bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop,    D3DXVECTOR2(fTexCoordLeft,  fTexCoordTop),   // colorGlowTop,			// top-left
			D3DXVECTOR3(fXTopRight-0.5f,   fYTop-0.5f,   0), bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop,    D3DXVECTOR2(fTexCoordRight, fTexCoordTop),   // colorGlowTop,			// top-right
			D3DXVECTOR3(fXBottomLeft-0.5f, fYBottom-0.5f,0), bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom, D3DXVECTOR2(fTexCoordLeft,  fTexCoordBottom),// colorGlowBottom,		// bottom-left
			D3DXVECTOR3(fXBottomRight-0.5f,fYBottom-0.5f,0), bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom, D3DXVECTOR2(fTexCoordRight, fTexCoordBottom) );//, colorGlowBottom );	// bottom-right
	}

	//
	// Draw the body
	//
	for( fY=fYBodyTop; fY<fYTailTop+1; fY+=fYStep )	// top to bottom
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
		const float fTexCoordTop	= SCALE( fTopDistFromTailTop,    0, fBodyHeight, 1, 0 );
		const float fTexCoordBottom = SCALE( fBottomDistFromTailTop, 0, fBodyHeight, 1, 0 );
		const float fTexCoordLeft	= bActive ? 0.75f : 0.50f;
		const float fTexCoordRight	= bActive ? 1.00f : 0.75f;
		const float	fAlphaTop		= ArrowGetAlpha( m_PlayerNumber, fYTop, fPercentFadeToFail );
		const float	fAlphaBottom	= ArrowGetAlpha( m_PlayerNumber, fYBottom, fPercentFadeToFail );
		const float	fGlowTop		= ArrowGetGlow( m_PlayerNumber, fYTop, fPercentFadeToFail );
		const float	fGlowBottom		= ArrowGetGlow( m_PlayerNumber, fYBottom, fPercentFadeToFail );
		const float fColorScale		= SCALE(fLife,0,1,0.5f,1);
		const D3DXCOLOR colorDiffuseTop		= D3DXCOLOR(fColorScale,fColorScale,fColorScale,fAlphaTop);
		const D3DXCOLOR colorDiffuseBottom	= D3DXCOLOR(fColorScale,fColorScale,fColorScale,fAlphaBottom);
		const D3DXCOLOR colorGlowTop		= D3DXCOLOR(1,1,1,fGlowTop);
		const D3DXCOLOR colorGlowBottom		= D3DXCOLOR(1,1,1,fGlowBottom);

		if( bDrawGlowOnly && colorGlowTop.a==0 && colorGlowBottom.a==0 )
			continue;
		if( !bDrawGlowOnly && colorDiffuseTop.a==0 && colorDiffuseBottom.a==0 )
			continue;

		DISPLAY->AddQuad( 
			D3DXVECTOR3(fXTopLeft-0.5f,    fYTop-0.5f,   0), bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop,    D3DXVECTOR2(fTexCoordLeft,  fTexCoordTop),    //colorGlowTop,			// top-left
			D3DXVECTOR3(fXTopRight-0.5f,   fYTop-0.5f,   0), bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop,    D3DXVECTOR2(fTexCoordRight, fTexCoordTop),    //colorGlowTop,			// top-right
			D3DXVECTOR3(fXBottomLeft-0.5f, fYBottom-0.5f,0), bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom, D3DXVECTOR2(fTexCoordLeft,  fTexCoordBottom), //colorGlowBottom,		// bottom-left
			D3DXVECTOR3(fXBottomRight-0.5f,fYBottom-0.5f,0), bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom, D3DXVECTOR2(fTexCoordRight, fTexCoordBottom) );//, colorGlowBottom );	// bottom-right
	}	

	DISPLAY->FlushQueue();


	//
	// Draw head
	//
	{
		fY							= fYHead;
		const float fX				= ArrowGetXPos( m_PlayerNumber, iCol, fY );
		const float	fAlpha			= ArrowGetAlpha( m_PlayerNumber, fY, fPercentFadeToFail );
		const float	fGlow			= ArrowGetGlow( m_PlayerNumber, fY, fPercentFadeToFail );
		const float fColorScale		= SCALE(fLife,0,1,0.5f,1);
		const D3DXCOLOR colorDiffuse= D3DXCOLOR(fColorScale,fColorScale,fColorScale,fAlpha);
		const D3DXCOLOR colorGlow	= D3DXCOLOR(1,1,1,fGlow);

		m_sprHoldParts.SetState( bActive?1:0 );
		m_sprHoldParts.SetXY( fX, fY );
		if( bDrawGlowOnly )
		{
			m_sprHoldParts.SetDiffuse( D3DXCOLOR(1,1,1,0) );
			m_sprHoldParts.SetGlow( colorGlow );
		}
		else
		{
			m_sprHoldParts.SetDiffuse( colorDiffuse );
			m_sprHoldParts.SetGlow( D3DXCOLOR(0,0,0,0) );
		}
		m_sprHoldParts.Draw();
	}

	// now, draw the glow pass
	if( !bDrawGlowOnly )
		DrawHold( hn, bActive, fLife, fPercentFadeToFail, true );
}

void NoteDisplay::DrawTap( const int iCol, const float fBeat, const bool bUseHoldColor, const float fPercentFadeToFail )
{
	const float fYOffset		= ArrowGetYOffset(	m_PlayerNumber, fBeat );
	const float fYPos			= ArrowGetYPos(		m_PlayerNumber, fYOffset );
	const float fRotation		= ArrowGetRotation(	m_PlayerNumber, iCol, fYOffset );
	const float fXPos			= ArrowGetXPos(		m_PlayerNumber, iCol, fYPos );
	const float fAlpha			= ArrowGetAlpha(	m_PlayerNumber, fYPos, fPercentFadeToFail );
	const float fGlow			= ArrowGetGlow(		m_PlayerNumber, fYPos, fPercentFadeToFail );
	const int iGrayPartFrameNo	= GetTapGrayFrameNo( fBeat );
	const int iColorPartFrameNo	= GetTapColorFrameNo( fBeat );

	D3DXCOLOR colorGrayPart = D3DXCOLOR(1,1,1,1);
	D3DXCOLOR colorLeadingEdge;
	D3DXCOLOR colorTrailingEdge;
	GetTapEdgeColors( fBeat, colorLeadingEdge, colorTrailingEdge );
	colorGrayPart.a		*= fAlpha;
	colorLeadingEdge.a	*= fAlpha;
	colorTrailingEdge.a *= fAlpha;

	if( bUseHoldColor )
	{
		//
		// draw hold head
		//
		m_sprHoldParts.SetXY( fXPos, fYPos );
		m_sprHoldParts.SetDiffuse( colorGrayPart );
		m_sprHoldParts.SetGlow( D3DXCOLOR(1,1,1,fGlow) );
		m_sprHoldParts.SetState( 0 );
		m_sprHoldParts.Draw();
	}
	else	
	{
		m_sprTapParts.SetXY( fXPos, fYPos );
		m_sprTapParts.SetRotation( fRotation );
		m_sprTapParts.SetGlow( D3DXCOLOR(1,1,1,fGlow) );

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