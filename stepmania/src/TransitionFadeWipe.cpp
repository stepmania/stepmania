#include "stdafx.h"
//-----------------------------------------------------------------------------
// File: TransitionFadeWipe.cpp
//
// Desc: "Window blinds"-type transition.
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------


#include "RageUtil.h"

#include "TransitionFadeWipe.h"


#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

#define RECTANGLE_WIDTH	20
#define NUM_RECTANGLES	(SCREEN_WIDTH/RECTANGLE_WIDTH)
#define FADE_RECTS_WIDE	(NUM_RECTANGLES/3)	// number of rects from fade start to fade end



TransitionFadeWipe::TransitionFadeWipe()
{
	
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
	BOOL bLeftEdgeIsDarker = m_TransitionState == opening_left || m_TransitionState == closing_right;
	BOOL bFadeIsMovingLeft = m_TransitionState == opening_left || m_TransitionState == closing_left;

	double	fPercentComplete = m_fPercentThroughTransition;
	double  fPercentAlongScreen = bFadeIsMovingLeft ? 1-fPercentComplete : fPercentComplete;

	int	iFadeLeftEdge  = (int)
			(-FADE_RECTS_WIDE + (NUM_RECTANGLES + FADE_RECTS_WIDE) * fPercentAlongScreen);
	int	iFadeRightEdge = (int)
			(0				  + (NUM_RECTANGLES + FADE_RECTS_WIDE) * fPercentAlongScreen);
	
	// draw all rectangles
	for( int i=0; i<NUM_RECTANGLES; i++ )
	{
		int iRectX		= (int)(RECTANGLE_WIDTH * (i+0.5));
		int iRectWidth;
		
		if( i < iFadeLeftEdge )
			iRectWidth = bLeftEdgeIsDarker ? RECTANGLE_WIDTH : 0;
			//iRectWidth = 0;
		else if( i > iFadeRightEdge )
			iRectWidth = bLeftEdgeIsDarker ? 0 : RECTANGLE_WIDTH;
			//iRectWidth = 0;
		else 		// this is a fading rectangle
		{
			int iRectsFromLeftFadeEdge = i - iFadeLeftEdge;
			double fPercentAlongFade   = iRectsFromLeftFadeEdge / (double)FADE_RECTS_WIDE;
			double fPercentDarkness    = bLeftEdgeIsDarker ? 1-fPercentAlongFade : fPercentAlongFade;
			iRectWidth = (int)(RECTANGLE_WIDTH * fPercentDarkness);
			
		}
		//iRectWidth = RECTANGLE_WIDTH;

		if( iRectWidth > 0 )
		//if( i == iFadeLeftEdge  ||  i == iFadeRightEdge )
		{	// draw the rectangle
			int iRectAlpha = (int)( iRectWidth * 255.0f / RECTANGLE_WIDTH );
			RECT rect;
			SetRect( &rect, iRectX-RECTANGLE_WIDTH/2, 0, 
							iRectX+RECTANGLE_WIDTH/2, SCREEN_HEIGHT );
			SCREEN->DrawRect( &rect, D3DCOLOR_ARGB(iRectAlpha,0,0,0) );
		}
	}	// end foreach rect
}


