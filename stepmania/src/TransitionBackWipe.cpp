#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: TransitionBackWipe

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "TransitionBackWipe.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "GameConstantsAndTypes.h"


#define RECTANGLE_WIDTH	20
#define NUM_RECTANGLES	(SCREEN_WIDTH/RECTANGLE_WIDTH)
#define FADE_RECTS_WIDE	(NUM_RECTANGLES/4)	// number of rects from fade start to fade end


TransitionBackWipe::TransitionBackWipe()
{
	m_soundBack.Load( THEME->GetPathTo("Sounds","menu back") );
}

void TransitionBackWipe::DrawPrimitives()
{
	if( m_TransitionState == opened ) 
		return;

	/////////////////////////
	// draw rectangles that get progressively smaller at the edge of the wipe
	/////////////////////////
	bool bLeftEdgeIsDarker = m_TransitionState == opening_left || m_TransitionState == closing_right;
	bool bFadeIsMovingLeft = m_TransitionState == opening_left || m_TransitionState == closing_left;

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

		if( iRectWidth > 0 )
		if( i == iFadeLeftEdge  ||  i == iFadeRightEdge )
		{
			m_quad.SetXY( (float)iRectX, CENTER_Y );
			m_quad.SetZoomX( (float)iRectWidth );
			m_quad.SetZoomY( SCREEN_HEIGHT );
			m_quad.SetDiffuse( RageColor(0,0,0,1) );
			m_quad.Draw();
		}
	}	// end foreach rect
}


