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
	if( m_sprHoldParts.GetNumStates() != 16 )
		throw RageException( "Hold Parts '%s' must have 16 frames.", sPath );

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
// Chris: If EZ2 doesn't use these values, it can just ignore them.
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



	float fPercentThroughColorsLeading;		// fill these in below
	float fPercentThroughColorsTrailing;
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

/*
D3DXCOLOR NoteDisplay::GetHoldColor( const float fY, const float fYTop, const float fYBottom )
{
	if( fY == fYTop )
		return m_colorHoldTweens[0];

	float fDistFromEnd = fYBottom - fY;
	float fArrowSizesFromEnd = fDistFromEnd / (float)ARROW_SIZE;
	float fPercentThroughColors = fmodf( fArrowSizesFromEnd, 2 ) / 2.0f; 
	float fIndex = fPercentThroughColors * (m_colorHoldTweens.GetSize()-1);
	float fWeightOf2 = fIndex - int(fIndex);
	int iColor1 = int(fIndex);
	int iColor2 = (iColor1+1) % m_colorHoldTweens.GetSize();
	return m_colorHoldTweens[iColor1] * (1-fWeightOf2) + m_colorHoldTweens[iColor2] * fWeightOf2;	
}
*/

float NoteDisplay::GetAddAlpha( float fDiffuseAlpha, float fPercentFadeToFail )
{
	if( fPercentFadeToFail != -1 )
		return 1-fPercentFadeToFail;

	float fDistFromCenter = fabsf( 0.5f - fDiffuseAlpha );
	return clamp( 1-fDistFromCenter*2, 0, 1 );
}


/*
void NoteDisplay::DrawList( int iCount, NoteDisplayInstance cni[], bool bDrawAddPass )
{
	int iNumV;

	LPDIRECT3DVERTEXBUFFER8 pVB = DISPLAY->GetVertexBuffer();
	RAGEVERTEX* v;

	LPDIRECT3DDEVICE8 pd3dDevice = DISPLAY->GetDevice();

	pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
	pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	pd3dDevice->SetVertexShader( D3DFVF_RAGEVERTEX );
	pd3dDevice->SetStreamSource( 0, pVB, sizeof(RAGEVERTEX) );


	const float fGrayWidth		= m_sprHoldParts.GetUnzoomedWidth();
	const float fGrayHeight		= m_sprHoldParts.GetUnzoomedHeight();
	const float fColorWidth		= m_sprTapParts.GetUnzoomedWidth();
	const float fColorHeight	= m_sprTapParts.GetUnzoomedHeight();

	RageTexture*	pGrayTexture	= m_sprHoldParts.GetTexture();
	RageTexture*	pColorTexture	= m_sprTapParts.GetTexture();

	//
	//	Draw gray parts diffuse pass
	//
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );

	pd3dDevice->SetTexture( 0, pGrayTexture->GetD3DTexture() );
	
	pVB->Lock( 0, 0, (BYTE**)&v, 0 );

	iNumV = 0;
	for( int i=0; i<iCount; i++ )
	{
		NoteDisplayInstance* pCNI = &cni[i];
		const float fX = pCNI->fX;
		const float fY = pCNI->fY;
		const float fRotation = pCNI->fRotation;
		D3DXVECTOR3 vecPoints[4];	// top left, bottom left, top right, bottom right

		
		//
		// Set vertex positions
		//
		vecPoints[0] = D3DXVECTOR3( -fGrayWidth/2, -fGrayHeight/2,	0 );	// top left
		vecPoints[1] = D3DXVECTOR3( -fGrayWidth/2, +fGrayHeight/2,	0 );	// bottom left
		vecPoints[2] = D3DXVECTOR3( +fGrayWidth/2, -fGrayHeight/2,	0 );	// top right
		vecPoints[3] = D3DXVECTOR3( +fGrayWidth/2, +fGrayHeight/2,	0 );	// bottom right

		if( fRotation != 0 )
		{
			D3DXMATRIX matRotation;
			D3DXMatrixRotationZ( &matRotation, fRotation );

			for( int j=0; j<4; j++ )
				D3DXVec3TransformCoord( &vecPoints[j], &vecPoints[j], &matRotation ); 
		}
		for( int j=0; j<4; j++ )
		{
			vecPoints[j].x += fX;
			vecPoints[j].y += fY;
		}

		// first triangle
		v[iNumV++].p = vecPoints[0];	// top left
		v[iNumV++].p = vecPoints[1];	// bottom left
		v[iNumV++].p = vecPoints[2];	// top right
		// 2nd triangle
		v[iNumV++].p = vecPoints[2];	// top right
		v[iNumV++].p = vecPoints[1];	// bottom left
		v[iNumV++].p = vecPoints[3];	// bottom right

		//
		// set texture coordinates
		//
		iNumV -= 6;

		FRECT* pTexCoordRect = pGrayTexture->GetTextureCoordRect( pCNI->iGrayPartFrameNo );

		v[iNumV++].t = D3DXVECTOR2( pTexCoordRect->left,	pTexCoordRect->top );		// top left
		v[iNumV++].t = D3DXVECTOR2( pTexCoordRect->left,	pTexCoordRect->bottom );	// bottom left
		v[iNumV++].t = D3DXVECTOR2( pTexCoordRect->right,	pTexCoordRect->top );		// top right
		v[iNumV++].t = v[iNumV-1].t;													// top right
		v[iNumV++].t = v[iNumV-3].t;													// bottom left
		v[iNumV++].t = D3DXVECTOR2( pTexCoordRect->right,	pTexCoordRect->bottom );	// bottom right

		//
		// set vertex colors
		//
		iNumV -= 6;

		D3DXCOLOR	colorGrayPart(1,1,1,pCNI->fAlpha);

		v[iNumV++].color = colorGrayPart;	// top left
		v[iNumV++].color = colorGrayPart;	// bottom left
		v[iNumV++].color = colorGrayPart;	// top right
		v[iNumV++].color = colorGrayPart;	// top right
		v[iNumV++].color = colorGrayPart;	// bottom left
		v[iNumV++].color = colorGrayPart;	// bottom right

	}
	ASSERT( iNumV == iCount*6 );	// two triangles for each note

	pVB->Unlock();

	pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, iNumV/3 );


	//
	//	Draw gray parts add pass
	//
	if( bDrawAddPass )
	{
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );

		pVB->Lock( 0, 0, (BYTE**)&v, 0 );


		iNumV = 0;
		for( int i=0; i<iCount; i++ )
		{
			NoteDisplayInstance* pCNI = &cni[i];

			const D3DXCOLOR colorGlow = D3DXCOLOR(1,1,1,pCNI->fAddAlpha);

			//
			// set vertex colors
			//
			v[iNumV++].color = colorGlow;	// top left
			v[iNumV++].color = colorGlow;	// bottom left
			v[iNumV++].color = colorGlow;	// top right
			v[iNumV++].color = colorGlow;	// top right
			v[iNumV++].color = colorGlow;	// bottom left
			v[iNumV++].color = colorGlow;	// bottom right

		}
		ASSERT( iNumV == iCount*6 );	// two triangles for each note

		pVB->Unlock();

		pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, iNumV/3 );
	}

		

		




	//
	//	Draw color parts diffuse pass
	//
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );

	pd3dDevice->SetTexture( 0, pColorTexture->GetD3DTexture() );
	
	pVB->Lock( 0, 0, (BYTE**)&v, 0 );

	iNumV = 0;
	for( i=0; i<iCount; i++ )
	{
		NoteDisplayInstance* pCNI = &cni[i];
		const float fX = pCNI->fX;
		const float fY = pCNI->fY;
		const float fRotation = pCNI->fRotation;
		D3DXVECTOR3 vecPoints[4];	// top left, bottom left, top right, bottom right

		//
		// Set vertex positions
		//
		vecPoints[0] = D3DXVECTOR3( -fColorWidth/2, -fColorHeight/2,	0 );	// top left
		vecPoints[1] = D3DXVECTOR3( -fColorWidth/2, +fColorHeight/2,	0 );	// bottom left
		vecPoints[2] = D3DXVECTOR3( +fColorWidth/2, -fColorHeight/2,	0 );	// top right
		vecPoints[3] = D3DXVECTOR3( +fColorWidth/2, +fColorHeight/2,	0 );	// bottom right

		if( fRotation != 0 )
		{
			D3DXMATRIX matRotation;
			D3DXMatrixRotationZ( &matRotation, fRotation );

			for( int j=0; j<4; j++ )
				D3DXVec3TransformCoord( &vecPoints[j], &vecPoints[j], &matRotation ); 
		}
		for( int j=0; j<4; j++ )
		{
			vecPoints[j].x += fX;
			vecPoints[j].y += fY;
		}

		// first triangle
		v[iNumV++].p = vecPoints[0];	// top left
		v[iNumV++].p = vecPoints[1];	// bottom left
		v[iNumV++].p = vecPoints[2];	// top right
		// 2nd triangle
		v[iNumV++].p = vecPoints[2];	// top right
		v[iNumV++].p = vecPoints[1];	// bottom left
		v[iNumV++].p = vecPoints[3];	// bottom right

		//
		// set texture coordinates
		//
		iNumV -= 6;

		FRECT* pTexCoordRect = pColorTexture->GetTextureCoordRect( pCNI->iColorPartFrameNo );

		v[iNumV++].t = D3DXVECTOR2( pTexCoordRect->left,	pTexCoordRect->top );		// top left
		v[iNumV++].t = D3DXVECTOR2( pTexCoordRect->left,	pTexCoordRect->bottom );	// bottom left
		v[iNumV++].t = D3DXVECTOR2( pTexCoordRect->right,	pTexCoordRect->top );		// top right
		v[iNumV++].t = v[iNumV-1].t;													// top right
		v[iNumV++].t = v[iNumV-3].t;													// bottom left
		v[iNumV++].t = D3DXVECTOR2( pTexCoordRect->right,	pTexCoordRect->bottom );	// bottom right

		//
		// set vertex colors
		//
		iNumV -= 6;

		D3DXCOLOR colorLeadingEdge = D3DXCOLOR(pCNI->colorLeadingEdge);
		D3DXCOLOR colorTrailingEdge = D3DXCOLOR(pCNI->colorTrailingEdge);

		colorLeadingEdge.a = pCNI->fAlpha;
		colorTrailingEdge.a = pCNI->fAlpha;
	
		v[iNumV++].color = colorLeadingEdge;	// top left
		v[iNumV++].color = colorTrailingEdge;	// bottom left
		v[iNumV++].color = colorLeadingEdge;	// top right
		v[iNumV++].color = v[iNumV-1].color;	// top right
		v[iNumV++].color = v[iNumV-3].color;	// bottom left
		v[iNumV++].color = colorTrailingEdge;	// bottom right

	}
	ASSERT( iNumV == iCount*6 );	// two triangles for each note

	pVB->Unlock();

	pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, iNumV/3 );


	//
	//	Draw Color parts add pass
	//
	if( bDrawAddPass )
	{
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );

		pVB->Lock( 0, 0, (BYTE**)&v, 0 );


		iNumV = 0;
		for( int i=0; i<iCount; i++ )
		{
			NoteDisplayInstance* pCNI = &cni[i];

			const D3DXCOLOR colorGlow = D3DXCOLOR(1,1,1,pCNI->fAddAlpha);

			//
			// set vertex colors
			//
			v[iNumV++].color = colorGlow;	// top left
			v[iNumV++].color = colorGlow;	// bottom left
			v[iNumV++].color = colorGlow;	// top right
			v[iNumV++].color = colorGlow;	// top right
			v[iNumV++].color = colorGlow;	// bottom left
			v[iNumV++].color = colorGlow;	// bottom right

		}
		ASSERT( iNumV == iCount*6 );	// two triangles for each note

		pVB->Unlock();

		pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, iNumV/3 );
	}
}
*/


void NoteDisplay::DrawHold( const HoldNote& hn, const bool bActive, const float fLife, const float fPercentFadeToFail )
{
	const int	iCol			= hn.m_iTrack;
	const float fStartYOffset	= ArrowGetYOffset2(	m_PlayerNumber, hn.m_fStartBeat );
	const float fStartYPos		= ArrowGetYPos(		m_PlayerNumber, fStartYOffset );
	const float fEndYOffset		= ArrowGetYOffset2(	m_PlayerNumber, hn.m_fEndBeat );
	const float fEndYPos		= ArrowGetYPos(		m_PlayerNumber, fEndYOffset );

	// draw from bottom to top
	const float fFrameWidth = m_sprHoldParts.GetUnzoomedWidth();
	const float fFrameHeight = m_sprHoldParts.GetUnzoomedHeight();
	const float fBodyHeight = fFrameHeight*4;

	const float fYHead = min(fStartYPos, fEndYPos);		// stop drawing here
	const float fYTail = max(fStartYPos, fEndYPos);		// the center the tail

	const float fYTailTop = fYTail-fFrameHeight/2;		
	const float fYTailBottom = fYTail+fFrameHeight/2;	

	const float fYBodyTop = fYHead;			// middle of head		
	const float fYBodyBottom = fYTailTop;	// top of tail

	const int	fYStep = 8;		// draw a segment every 8 pixels	// this requires that the texture dimensions be a multiple of 8

	DISPLAY->SetBlendModeNormal();
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
		const float fYBottom		= fY+fYStep;
		const float fXTop			= ArrowGetXPos2( m_PlayerNumber, iCol, fYTop );
		const float fXBottom		= ArrowGetXPos2( m_PlayerNumber, iCol, fYBottom );
		const float fXTopLeft		= fXTop - fFrameWidth/2;
		const float fXTopRight		= fXTop + fFrameWidth/2;
		const float fXBottomLeft	= fXBottom - fFrameWidth/2;
		const float fXBottomRight	= fXBottom + fFrameWidth/2;
		const float fTopDistFromTailTop		= fYTop - fYTailTop;
		const float fBottomDistFromTailTop	= fYBottom - fYTailTop;
		const float fTexCoordTop	= SCALE( fTopDistFromTailTop,    0, fFrameHeight, 0.25f, 0.5f );
		const float fTexCoordBottom = SCALE( fBottomDistFromTailTop, 0, fFrameHeight, 0.25f, 0.5f );
		const float fTexCoordLeft	= bActive ? 0.25f : 0.00f;
		const float fTexCoordRight	= bActive ? 0.50f : 0.25f;
		const float	fAlphaTop		= ArrowGetAlpha( m_PlayerNumber, fYTop );
		const float	fAlphaBottom	= ArrowGetAlpha( m_PlayerNumber, fYBottom );
		const float	fAddAlphaTop	= GetAddAlpha( fAlphaTop, fPercentFadeToFail );
		const float	fAddAlphaBottom	= GetAddAlpha( fAlphaBottom, fPercentFadeToFail );
		const float fColorScale		= SCALE(fLife,0,1,0.5f,1);
		const D3DXCOLOR colorDiffuseTop		= D3DXCOLOR(fColorScale,fColorScale,fColorScale,fAlphaTop);
		const D3DXCOLOR colorDiffuseBottom	= D3DXCOLOR(fColorScale,fColorScale,fColorScale,fAlphaBottom);
		const D3DXCOLOR colorGlowTop			= D3DXCOLOR(1,1,1,fAddAlphaTop);
		const D3DXCOLOR colorGlowBottom		= D3DXCOLOR(1,1,1,fAddAlphaBottom);

		// the shift by -0.5 is to align texels to pixels

		DISPLAY->AddQuad( 
			D3DXVECTOR3(fXTopLeft-0.5f,    fYTop-0.5f,   0), colorDiffuseTop,    D3DXVECTOR2(fTexCoordLeft,  fTexCoordTop),   // colorGlowTop,			// top-left
			D3DXVECTOR3(fXTopRight-0.5f,   fYTop-0.5f,   0), colorDiffuseTop,    D3DXVECTOR2(fTexCoordRight, fTexCoordTop),   // colorGlowTop,			// top-right
			D3DXVECTOR3(fXBottomLeft-0.5f, fYBottom-0.5f,0), colorDiffuseBottom, D3DXVECTOR2(fTexCoordLeft,  fTexCoordBottom),// colorGlowBottom,		// bottom-left
			D3DXVECTOR3(fXBottomRight-0.5f,fYBottom-0.5f,0), colorDiffuseBottom, D3DXVECTOR2(fTexCoordRight, fTexCoordBottom) );//, colorGlowBottom );	// bottom-right
	}

	//
	// Draw the body
	//
	for( fY=fYBodyTop; fY<fYTailTop; fY+=fYStep )	// top to bottom
	{
		const float fYTop			= fY;
		const float fYBottom		= min( fY+fYStep, fYTailTop );
		const float fXTop			= ArrowGetXPos2( m_PlayerNumber, iCol, fYTop );
		const float fXBottom		= ArrowGetXPos2( m_PlayerNumber, iCol, fYBottom );
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
		const float	fAlphaTop		= ArrowGetAlpha( m_PlayerNumber, fYTop );
		const float	fAlphaBottom	= ArrowGetAlpha( m_PlayerNumber, fYBottom );
		const float	fAddAlphaTop	= GetAddAlpha( fAlphaTop, fPercentFadeToFail );
		const float	fAddAlphaBottom	= GetAddAlpha( fAlphaBottom, fPercentFadeToFail );
		const float fColorScale		= SCALE(fLife,0,1,0.5f,1);
		const D3DXCOLOR colorDiffuseTop		= D3DXCOLOR(fColorScale,fColorScale,fColorScale,fAlphaTop);
		const D3DXCOLOR colorDiffuseBottom	= D3DXCOLOR(fColorScale,fColorScale,fColorScale,fAlphaBottom);
		const D3DXCOLOR colorGlowTop			= D3DXCOLOR(1,1,1,fAddAlphaTop);
		const D3DXCOLOR colorGlowBottom		= D3DXCOLOR(1,1,1,fAddAlphaBottom);

		DISPLAY->AddQuad( 
			D3DXVECTOR3(fXTopLeft-0.5f,    fYTop-0.5f,   0), colorDiffuseTop,    D3DXVECTOR2(fTexCoordLeft,  fTexCoordTop),    //colorGlowTop,			// top-left
			D3DXVECTOR3(fXTopRight-0.5f,   fYTop-0.5f,   0), colorDiffuseTop,    D3DXVECTOR2(fTexCoordRight, fTexCoordTop),    //colorGlowTop,			// top-right
			D3DXVECTOR3(fXBottomLeft-0.5f, fYBottom-0.5f,0), colorDiffuseBottom, D3DXVECTOR2(fTexCoordLeft,  fTexCoordBottom), //colorGlowBottom,		// bottom-left
			D3DXVECTOR3(fXBottomRight-0.5f,fYBottom-0.5f,0), colorDiffuseBottom, D3DXVECTOR2(fTexCoordRight, fTexCoordBottom) );//, colorGlowBottom );	// bottom-right
	}	

	DISPLAY->FlushQueue();


	//
	// Draw head
	//
	{
		fY							= fYHead;
		const float fX				= ArrowGetXPos2( m_PlayerNumber, iCol, fY );
		const float	fAlpha			= ArrowGetAlpha( m_PlayerNumber, fY );
		const float	fAddAlpha		= GetAddAlpha( fAlpha, fPercentFadeToFail );
		const float fColorScale		= SCALE(fLife,0,1,0.5f,1);
		const D3DXCOLOR colorDiffuse= D3DXCOLOR(fColorScale,fColorScale,fColorScale,fAlpha);
		const D3DXCOLOR colorGlow	= D3DXCOLOR(1,1,1,fAddAlpha);

		m_sprHoldParts.SetState( bActive?1:0 );
		m_sprHoldParts.SetXY( fX, fY );
		m_sprHoldParts.SetDiffuseColor( colorDiffuse );
		m_sprHoldParts.SetGlowColor( colorGlow );
		m_sprHoldParts.Draw();
	}
}

void NoteDisplay::DrawTap( const int iCol, const float fBeat, const bool bUseHoldColor, const float fPercentFadeToFail )
{
	const float fYOffset		= ArrowGetYOffset2(	m_PlayerNumber, fBeat );
	const float fYPos			= ArrowGetYPos(		m_PlayerNumber, fYOffset );
	const float fRotation		= ArrowGetRotation(	m_PlayerNumber, iCol, fYOffset );
	const float fXPos			= ArrowGetXPos2(	m_PlayerNumber, iCol, fYPos );
	const float fAlpha			= ArrowGetAlpha(	m_PlayerNumber, fYPos );
	const float fAddAlpha		= GetAddAlpha(		fAlpha, fPercentFadeToFail );
	const int iGrayPartFrameNo	= GetTapGrayFrameNo( fBeat );
	const int iColorPartFrameNo	= GetTapColorFrameNo( fBeat );

	D3DXCOLOR colorGrayPart = D3DXCOLOR(1,1,1,1);
	D3DXCOLOR colorLeadingEdge;
	D3DXCOLOR colorTrailingEdge;
	if( bUseHoldColor )
		colorLeadingEdge = colorTrailingEdge = D3DXCOLOR(0,1,0,1);	// HACK: green.
	else
		GetTapEdgeColors( fBeat, colorLeadingEdge, colorTrailingEdge );
	colorGrayPart.a		*= fAlpha;
	colorLeadingEdge.a	*= fAlpha;
	colorTrailingEdge.a *= fAlpha;

	m_sprTapParts.SetXY( fXPos, fYPos );
	m_sprTapParts.SetRotation( fRotation );
	m_sprTapParts.SetGlowColor( D3DXCOLOR(1,1,1,fAddAlpha) );

	//
	// draw gray part
	//
	m_sprTapParts.SetState( iGrayPartFrameNo );
	m_sprTapParts.SetDiffuseColor( colorGrayPart );
	m_sprTapParts.Draw();

	//
	// draw color part
	//
	m_sprTapParts.SetState( iColorPartFrameNo );
	m_sprTapParts.SetDiffuseColorTopEdge( colorLeadingEdge );
	m_sprTapParts.SetDiffuseColorBottomEdge( colorTrailingEdge );
	m_sprTapParts.Draw();
}