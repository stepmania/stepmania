#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: TransitionFade.cpp

 Desc: Fades out or in.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


#include "RageUtil.h"

#include "TransitionFade.h"
#include "ScreenDimensions.h"



TransitionFade::TransitionFade()
{
	m_rect.StretchTo( CRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT) );
}

TransitionFade::~TransitionFade()
{

}

void TransitionFade::RenderPrimitives()
{
	const float fPercentageOpaque = 1 - GetPercentageOpen();
	if( fPercentageOpaque == 0 )
		return;	// draw nothing

	D3DXCOLOR colorTemp = m_Color * fPercentageOpaque;
	m_rect.SetDiffuseColor( colorTemp );
	m_rect.Draw();
}


