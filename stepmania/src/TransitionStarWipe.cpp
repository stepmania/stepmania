#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: TransitionStarWipe.cpp

 Desc: Shooting start across the screen leave a black trail.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"

#include "TransitionStarWipe.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"


TransitionStarWipe::TransitionStarWipe()
{
	m_fTransitionTime = m_fTransitionTime * 4;
}

TransitionStarWipe::~TransitionStarWipe()
{

}

void TransitionStarWipe::DrawPrimitives()
{
	if( m_TransitionState == opened ) 
		return;
	else if( m_TransitionState == closed ) {
		m_rect.StretchTo( RectI(0,0,SCREEN_WIDTH,SCREEN_HEIGHT) );
		m_rect.SetDiffuse( RageColor(0,0,0,1) );
		m_rect.Draw();
		return;
	}

	float fPercentClosed = 1 - GetPercentageOpen();


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

		int x_offset = (int)(fPercentClosed*(SCREEN_WIDTH+SCREEN_HEIGHT+m_iStarWidth));

		int x = bIsAnEvenRow ?
			-SCREEN_HEIGHT/2			 + x_offset + x_tilt :
			SCREEN_WIDTH+SCREEN_HEIGHT/2 - x_offset + x_tilt;


		m_sprStar.SetRotation( bIsAnEvenRow ? D3DX_PI : 0.0f );	// flip the sprite
		m_sprStar.SetXY( bIsAnEvenRow?x-1.0f:x+0.0f, bIsAnEvenRow?y-1.0f:y+0.0f );	// fudge.  The rotation makes it off center
		m_sprStar.Draw();
		
		int x_rect_leading_edge = x + ( bIsAnEvenRow ? - m_iStarWidth/2 : m_iStarWidth/2 );
		int x_rect_trailing_edge = ( bIsAnEvenRow ? 0-1 : SCREEN_WIDTH+1 );
		int y_top = y - m_iStarHeight/2;
		int y_bot = y + m_iStarHeight/2+1;
		m_rect.StretchTo( RectI(x_rect_leading_edge, y_top, x_rect_trailing_edge,  y_bot) );
		m_rect.SetDiffuse( RageColor(0,0,0,1) );
		m_rect.Draw();
		
	}
}

void TransitionStarWipe::OpenWipingRight( ScreenMessage send_when_done )
{
	Transition::OpenWipingRight( send_when_done );
	LoadNewStarSprite( THEME->GetPathTo("Graphics","gameplay closing star") );
}

void TransitionStarWipe::OpenWipingLeft(  ScreenMessage send_when_done )
{
	Transition::OpenWipingLeft( send_when_done );
	LoadNewStarSprite( THEME->GetPathTo("Graphics","gameplay closing star") );
}

void TransitionStarWipe::CloseWipingRight(ScreenMessage send_when_done )
{
	Transition::CloseWipingRight( send_when_done );
	LoadNewStarSprite( THEME->GetPathTo("Graphics","gameplay opening star") );
}

void TransitionStarWipe::CloseWipingLeft( ScreenMessage send_when_done )
{
	Transition::CloseWipingLeft( send_when_done );
	LoadNewStarSprite( THEME->GetPathTo("Graphics","gameplay opening star") );
}

void TransitionStarWipe::LoadNewStarSprite( CString sFileName )
{
	m_sprStar.Load( sFileName );
	m_iStarWidth = (int)m_sprStar.GetZoomedWidth();
	m_iStarHeight = (int)m_sprStar.GetZoomedHeight();
}
