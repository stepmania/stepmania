#include "stdafx.h"
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
	m_rect.StretchTo( CRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT) );
	SetDiffuseColor( D3DXCOLOR(0,0,0,1) );	// black
}

TransitionFade::~TransitionFade()
{

}

void TransitionFade::DrawPrimitives()
{
	const float fPercentageOpaque = 1 - GetPercentageOpen();
	if( fPercentageOpaque == 0 )
		return;	// draw nothing

	D3DXCOLOR colorTemp = m_colorDiffuse[0] * fPercentageOpaque;
	m_rect.SetDiffuseColor( colorTemp );
	m_rect.Draw();
}


