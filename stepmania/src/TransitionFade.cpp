#include "stdafx.h"
//-----------------------------------------------------------------------------
// File: TransitionFade.cpp
//
// Desc: "Window blinds"-type transition.
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------


#include "RageUtil.h"

#include "TransitionFade.h"


#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

#define RECTANGLE_WIDTH	20
#define NUM_RECTANGLES	(SCREEN_WIDTH/RECTANGLE_WIDTH)
#define FADE_RECTS_WIDE	(NUM_RECTANGLES/3)	// number of rects from fade start to fade end



TransitionFade::TransitionFade()
{
}

TransitionFade::~TransitionFade()
{

}

void TransitionFade::Draw()
{
	FLOAT fPercentageOpaque;
	switch( m_TransitionState )
	{
	case opened:
		fPercentageOpaque = 0.0;
		break;
	case closed:
		fPercentageOpaque = 1.0;
		break;
	case opening_right:
	case opening_left:
		fPercentageOpaque = 1.0f - m_fPercentThroughTransition;
		break;
	case closing_right:
	case closing_left:
		fPercentageOpaque = m_fPercentThroughTransition;
		break;
	}

	m_Color.a = fPercentageOpaque;
	int alpha = (int)( fPercentageOpaque * 255 );
	SCREEN->DrawRect( CRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT), m_Color );
}


