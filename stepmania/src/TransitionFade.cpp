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
	SetDiffuse( D3DXCOLOR(0,0,0,1) );	// black
}

TransitionFade::~TransitionFade()
{

}

void TransitionFade::DrawPrimitives()
{
	const float fPercentageOpaque = 1 - GetPercentageOpen();
	if( fPercentageOpaque == 0 )
		return;	// draw nothing

	D3DXCOLOR colorTemp = GetDiffuse() * fPercentageOpaque;
	m_rect.SetDiffuse( colorTemp );
	m_rect.Draw();

// SUPER HACK!  For some reason, this does not draw in release mode.  I've looked for
// hours and can't figure out why.  It appears though if you draw it twice, so that's
// what we'll do for now.  Aye...
#ifndef _DEBUG
	m_rect.SetDiffuse( colorTemp );
	m_rect.Draw();
#endif
}


