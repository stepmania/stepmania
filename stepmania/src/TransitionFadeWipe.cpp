#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: TransitionStarWipe.cpp

 Desc: Black bands (horizontal window blinds) gradually close.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"

#include "TransitionFadeWipe.h"
#include "ScreenDimensions.h"


//#define RECTANGLE_WIDTH	20
//#define NUM_RECTANGLES	(SCREEN_WIDTH/RECTANGLE_WIDTH)
//#define FADE_RECTS_WIDE	(NUM_RECTANGLES/4)	// number of rects from fade start to fade end
const float FADE_RECT_WIDTH		=	SCREEN_WIDTH/2;


TransitionFadeWipe::TransitionFadeWipe()
{
	m_sprLogo.LoadFromTexture( "Textures\\Logo dots.png" );
	m_sprLogo.SetXY( CENTER_X, CENTER_Y );
}

TransitionFadeWipe::~TransitionFadeWipe()
{

}

void TransitionFadeWipe::Draw()
{
	if( m_TransitionState == opened ) 
		return;

	/////////////////////////
	// draw rectangles that get progressively smaller at the edge of the wipe
	/////////////////////////
	bool bLeftEdgeIsDarker = m_TransitionState == opening_left || m_TransitionState == closing_right;
	bool bFadeIsMovingLeft = m_TransitionState == opening_left || m_TransitionState == closing_left;

	float fPercentComplete = m_fPercentThroughTransition;
	float fPercentAlongScreen = bFadeIsMovingLeft ? 1-fPercentComplete*1.3f : fPercentComplete*1.3f;

	float fFadeLeftEdgeX  = fPercentAlongScreen * SCREEN_WIDTH - FADE_RECT_WIDTH/2;
	float fFadeRightEdgeX = fPercentAlongScreen * SCREEN_WIDTH + FADE_RECT_WIDTH/2;

	float fDarkEdgeX = bLeftEdgeIsDarker ? fFadeLeftEdgeX : fFadeRightEdgeX;
	float fLightEdgeX = bLeftEdgeIsDarker ? fFadeRightEdgeX : fFadeLeftEdgeX;

	float fDarkOutsideX = bLeftEdgeIsDarker ? fDarkEdgeX - SCREEN_WIDTH*2 : fDarkEdgeX + SCREEN_WIDTH*2;

	// draw dark rect
	SCREEN->DrawRect( 
		CRect(fDarkOutsideX, 0, fDarkEdgeX, SCREEN_HEIGHT), 
		D3DXCOLOR(0,0,0,1),	// up left
		D3DXCOLOR(0,0,0,1),	// up right
		D3DXCOLOR(0,0,0,1),	// down left
		D3DXCOLOR(0,0,0,1) 	// down right
		);

	// draw gradient rect
	SCREEN->DrawRect( 
		CRect(fDarkEdgeX, 0, fLightEdgeX, SCREEN_HEIGHT), 
		D3DXCOLOR(0,0,0,1),	// up left
		D3DXCOLOR(0,0,0,0),	// up right
		D3DXCOLOR(0,0,0,1),	// down left
		D3DXCOLOR(0,0,0,0) 	// down right
		);

	bool bIsOpening = m_TransitionState == opening_left || m_TransitionState == opening_right;
	float fLogoAlpha = bIsOpening ? (1-fPercentComplete)*3-2 : fPercentComplete*3-2;
	m_sprLogo.SetDiffuseColor( D3DXCOLOR(1,1,1,fLogoAlpha) );
	m_sprLogo.Draw();

}


