#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: TransitionFade

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "RageUtil.h"

#include "TransitionFade.h"
#include "GameConstantsAndTypes.h"



TransitionFade::TransitionFade()
{
	m_rect.StretchTo( RectI(0,0,SCREEN_WIDTH,SCREEN_HEIGHT) );
	SetDiffuse( RageColor(0,0,0,1) );	// black
}

TransitionFade::~TransitionFade()
{

}

void TransitionFade::DrawPrimitives()
{
	float fPercentageOpaque = 1 - GetPercentageOpen();
	if( fPercentageOpaque == 0 )
		return;	// draw nothing

	CLAMP( fPercentageOpaque, 0, 1 );

	RageColor colorTemp = GetDiffuse();
	colorTemp.a *= fPercentageOpaque;
	m_rect.SetDiffuse( colorTemp );
	m_rect.Draw();
}


