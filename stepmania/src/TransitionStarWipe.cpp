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
#include "ScreenDimensions.h"
#include "ThemeManager.h"


TransitionStarWipe::TransitionStarWipe()
{
	m_fTransitionTime = m_fTransitionTime * 1.5f;
}

TransitionStarWipe::~TransitionStarWipe()
{

}

void TransitionStarWipe::RenderPrimitives()
{
	if( m_TransitionState == opened ) 
		return;
	else if( m_TransitionState == closed ) {
		m_rect.StretchTo( CRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT) );
		m_rect.SetDiffuseColor( D3DCOLOR_RGBA(0,0,0,255) );
		m_rect.Draw();
		return;
	}

	float fPercentOpen = 1-GetPercentageOpen();
	/*
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
	*/


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
		default:
			ASSERT( false );
			x_tilt = 0;
		}

		int x_offset = (int)(fPercentOpen*(SCREEN_WIDTH+SCREEN_HEIGHT+m_iStarWidth));

		int x = bIsAnEvenRow ?
			-SCREEN_HEIGHT/2			 + x_offset + x_tilt :
			SCREEN_WIDTH+SCREEN_HEIGHT/2 - x_offset + x_tilt;


		m_sprStar.SetRotation( bIsAnEvenRow ? D3DX_PI : 0.0f );	// flip the sprite
		m_sprStar.SetXY( bIsAnEvenRow?x-1.0f:x+0.0f, bIsAnEvenRow?y-1.0f:y+0.0f );	// fudge.  The rotation makes it off center
		m_sprStar.Draw();
		
		int x_rect_leading_edge = x + ( bIsAnEvenRow ? - m_iStarWidth/2 : m_iStarWidth/2 );
		int x_rect_trailing_edge = ( bIsAnEvenRow ? 0 : SCREEN_WIDTH );
		int y_top = y - m_iStarHeight/2;
		int y_bot = y + m_iStarHeight/2+1;
		m_rect.StretchTo( CRect(x_rect_leading_edge, y_top, x_rect_trailing_edge,  y_bot) );
		m_rect.SetDiffuseColor( D3DCOLOR_ARGB(255,0,0,0) );
		m_rect.Draw();
		
	}
}

void TransitionStarWipe::OpenWipingRight( WindowMessage send_when_done )
{
	Transition::OpenWipingRight( send_when_done );
	LoadNewStarSprite( THEME->GetPathTo(GRAPHIC_CLOSING_STAR) );
}

void TransitionStarWipe::OpenWipingLeft(  WindowMessage send_when_done )
{
	Transition::OpenWipingLeft( send_when_done );
	LoadNewStarSprite( THEME->GetPathTo(GRAPHIC_CLOSING_STAR) );
}

void TransitionStarWipe::CloseWipingRight(WindowMessage send_when_done )
{
	Transition::CloseWipingRight( send_when_done );
	LoadNewStarSprite( THEME->GetPathTo(GRAPHIC_OPENING_STAR) );
}

void TransitionStarWipe::CloseWipingLeft( WindowMessage send_when_done )
{
	Transition::CloseWipingLeft( send_when_done );
	LoadNewStarSprite( THEME->GetPathTo(GRAPHIC_OPENING_STAR) );
}

void TransitionStarWipe::LoadNewStarSprite( CString sFileName )
{
	m_sprStar.Load( sFileName );
	m_iStarWidth = (int)m_sprStar.GetZoomedWidth();
	m_iStarHeight = (int)m_sprStar.GetZoomedHeight();
}
