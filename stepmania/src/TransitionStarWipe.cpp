#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: TransitionStarWipe.cpp

 Desc: Shooting start across the screen leave a black trail.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"

#include "TransitionStarWipe.h"


#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

#define TEXTURE_STAR_BLUE	"Textures\\Star Blue.png"
#define TEXTURE_STAR_YELLOW	"Textures\\Star Yellow.png"

TransitionStarWipe::TransitionStarWipe()
{
	m_fTransitionTime = m_fTransitionTime * 1.5f;
}

TransitionStarWipe::~TransitionStarWipe()
{

}

void TransitionStarWipe::Draw()
{
	if( m_TransitionState == opened ) 
		return;
	else if( m_TransitionState == closed ) {
		SCREEN->DrawRect( CRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT), D3DCOLOR_RGBA(0,0,0,255) );
		return;
	}

	FLOAT fPercentOpen;
	switch( m_TransitionState )
	{
	case opening_right:
	case opening_left:
		fPercentOpen = (1.0f - m_fPercentThroughTransition);
		break;
	case closing_right:
	case closing_left:
		fPercentOpen = m_fPercentThroughTransition;
		break;
	}


	int iNumStars = SCREEN_HEIGHT/m_iStarHeight + 1;

	for( int row=0; row<iNumStars; row++ )	// foreach row of stars
	{
		BOOL bIsAnEvenRow = row % 2;
		int y = m_iStarHeight*row + m_iStarHeight/2;
		int x_tilt;
		switch( m_TransitionState )
		{
		case opening_right:
		case opening_left:
			x_tilt = y - SCREEN_HEIGHT/2;
			break;
		case closing_right:
		case closing_left:
			x_tilt = abs( y - SCREEN_HEIGHT/2 );
			if( bIsAnEvenRow )	x_tilt *= -1; 
			break;
		}

		int x_offset = (int)(fPercentOpen*(SCREEN_WIDTH+SCREEN_HEIGHT+m_iStarWidth));

		int x = bIsAnEvenRow ?
			-SCREEN_HEIGHT/2			 + x_offset + x_tilt :
			SCREEN_WIDTH+SCREEN_HEIGHT/2 - x_offset + x_tilt;


		m_sprStar.SetRotation( bIsAnEvenRow ? D3DX_PI : 0.0f );	// flip the sprite
		m_sprStar.SetXY( bIsAnEvenRow?x-1:x, bIsAnEvenRow?y-1:y );	// fudge.  The rotation makes it off center
		m_sprStar.Draw();
		
		int x_rect_leading_edge = x + ( bIsAnEvenRow ? - m_iStarWidth/2 : m_iStarWidth/2 );
		int x_rect_trailing_edge = ( bIsAnEvenRow ? 0 : SCREEN_WIDTH );
		int y_top = y - m_iStarHeight/2;
		int y_bot = y + m_iStarHeight/2;
		SCREEN->DrawRect( CRect(x_rect_leading_edge, y_top,
								x_rect_trailing_edge,  y_bot ),
						  D3DCOLOR_ARGB(255,0,0,0) 
						);
		
	}
}

void TransitionStarWipe::OpenWipingRight( WindowMessage send_when_done )
{
	Transition::OpenWipingRight( send_when_done );
	LoadNewStarSprite( TEXTURE_STAR_BLUE );
}

void TransitionStarWipe::OpenWipingLeft(  WindowMessage send_when_done )
{
	Transition::OpenWipingLeft( send_when_done );
	LoadNewStarSprite( TEXTURE_STAR_BLUE );
}

void TransitionStarWipe::CloseWipingRight(WindowMessage send_when_done )
{
	Transition::CloseWipingRight( send_when_done );
	LoadNewStarSprite( TEXTURE_STAR_YELLOW );
}

void TransitionStarWipe::CloseWipingLeft( WindowMessage send_when_done )
{
	Transition::CloseWipingLeft( send_when_done );
	LoadNewStarSprite( TEXTURE_STAR_YELLOW );
}

void TransitionStarWipe::LoadNewStarSprite( CString sFileName )
{
	m_sprStar.LoadFromTexture( sFileName );
	m_iStarWidth = m_sprStar.GetZoomedWidth();
	m_iStarHeight = m_sprStar.GetZoomedHeight();
}
